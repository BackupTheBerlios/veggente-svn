/*	
 *	Veggente - engine.c
 *	Generic multithreaded request processor
 *	
 *	Copyright(c) 2006 Alessio Carenini <carenini@gmail.com>
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *	*/

 #include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include "generic_list.h"
#include "engine.h"

#define THREAD_LIMIT 10
struct engine_data{
		pthread_mutex_t queue_mutex;
		pthread_cond_t queue_signal;
		pthread_t queued_id;
		pthread_t requestd_id;
		int op_limit;  /* Limit of operation after which starts queue processing */
		int op_count;
		int timeout_sec; /* Every how many seconds queue processing is forced */
		list_data_t* op_list;
};

struct op_data {
		void* request;
		void* response;
};

void* engine_queue_daemon(void* manager_data);
void* engine_request_daemon(void* manager_data);
int engine_spawn_executors (engine_data_t *s, list_data_t* grouped_list);
int engine_group_work(engine_data_t *s,list_data_t* grouped_list);

int engine_init(engine_data_t *s,int op_limit) {
		engine_data_t t=NULL;
		t=(engine_data_t)calloc(1,sizeof(struct  engine_data));
		if (s==NULL) return (-1);
		/* Engine data init */
		list_init(t->op_list);
		t->op_count=0;
		t->op_limit=op_limit;
		pthread_mutex_init(&(t->queue_mutex),NULL);
		pthread_cond_init(&(t->queue_signal),NULL);
		*s=t;
		return (0);
}
int engine_start(engine_data_t *s) {
		int ret_code=0;
		engine_data_t t=NULL;
		pthread_attr_t th_attr;
		
		if (s==(engine_data_t*)NULL) return (-1);
		t=*s;
		/* Set joinable attribute */
		pthread_attr_init(&th_attr);
		pthread_attr_setdetachstate(&th_attr, PTHREAD_CREATE_JOINABLE);
		/* Create queue handler thread */
		fprintf(stdout,"Creation of the queue handler thread\n");
		if (pthread_create(&(t->queued_id),&th_attr,engine_queue_daemon,(void*)s)) {
				fprintf(stderr,"Error creating queue handler\n");
				return (-1);
		}
		/* Create request handler thread */
		fprintf(stdout,"Creation of the request handler thread\n");
		if (pthread_create(&(t->requestd_id),&th_attr,engine_request_daemon,(void*)s)) {
				fprintf(stderr,"Error creating request handler\n");
				return (-1);
		}
		pthread_attr_destroy(&th_attr);
		if (pthread_join(t->queued_id,(void**)&ret_code)) {
				fprintf(stderr,"Queue handler exited with status %d\n",ret_code);
				return (ret_code);
		}
		if (pthread_join(t->requestd_id,(void**)&ret_code)) {
				fprintf(stderr,"Request handler exited with status %d\n",ret_code);
				return (ret_code);
		}
		return (0);
}

int engine_destroy(engine_data_t *s) {
		engine_data_t t=NULL;
		if (s==(engine_data_t*)NULL) return (-1);
		t=*s;
		list_destroy(t->op_list);
		pthread_mutex_destroy(&(t->queue_mutex));
		pthread_cond_destroy(&(t->queue_signal));
		pthread_cancel(t->requestd_id);
		pthread_cancel(t->queued_id);
		free(t);
		return (0);
}

/* Function executed by "queued" thread */
void* engine_queue_daemon(void* manager_data) {
		int cond_code=0;
		engine_data_t t=NULL;
		list_data_t group_list=NULL;
		struct timeval now;
		struct timespec timeout;
		int mutex_code=0;

		if (( ((engine_data_t*)manager_data)==(engine_data_t*)NULL)||(list_init(&group_list)!=0)){
			   	pthread_exit((void*)-1);
		}
		t=((engine_data_t)manager_data);
		fprintf(stdout,"[Queued] locking\n");
		mutex_code=pthread_mutex_lock(&(t->queue_mutex));
		fprintf(stdout,"[Queued] mutex code %d\n",mutex_code);

		while(1) {
				if (t->op_count==0) {
						/* Wait if there are no requests */
						fprintf(stdout,"[Queued] Empty queue, sleeping\n");
						pthread_cond_wait(&(t->queue_signal),&(t->queue_mutex));
				} 
				if (t->op_count>0) {
						if (t->op_count < t->op_limit) {
								/* Timed wait*/
								gettimeofday(&now,NULL);
								timeout.tv_sec=now.tv_sec+t->timeout_sec;
								timeout.tv_nsec=now.tv_usec*1000;
								cond_code=pthread_cond_timedwait(&(t->queue_signal),&(t->queue_mutex),&timeout);
								if ((cond_code==0)||(cond_code==ETIMEDOUT)) {
										fprintf(stdout,"[Queued] Queue processing...\n");
										/* Group operations */
										if (engine_group_work(&t,&group_list)!=0) {
												fprintf(stderr,"[Queued] Error grouping operations\n");
												pthread_mutex_unlock(&(t->queue_mutex));
												pthread_exit((void*)-1);
										}
										/* Process operation groups */
										if (engine_spawn_executors(&t,&group_list)!=0) {
												fprintf(stderr,"[Queued] Error processing operations\n");
												pthread_mutex_unlock(&(t->queue_mutex));
												pthread_exit((void*)-1);
										}
										pthread_mutex_unlock(&(t->queue_mutex));
								}
						}
						else {
								/* Group and process*/
								pthread_mutex_unlock(&(t->queue_mutex));
						}
				}
				if (t->op_count<0) {
						fprintf(stderr,"[Queued] Error in queue length\n");
						pthread_mutex_unlock(&(t->queue_mutex));
						pthread_exit((void*)-1);
				}
		}

		pthread_mutex_unlock(&(t->queue_mutex));
		pthread_exit((void*)0);
}


/* While testing requests are created in this thread */
void* engine_request_daemon(void* manager_data) {
		engine_data_t t=*((engine_data_t*)manager_data);
		fprintf(stdout,"[Requestd] Thread started\n");
		pthread_exit((void*)0);
}

/* Build a list of lists of operations (operation group) */
int engine_group_work(engine_data_t *s,list_data_t* grouped_list) {
		engine_data_t t=NULL;
		if (s==(engine_data_t*)NULL) return (-1);
		t=*s;
		return (0);
}

/* Process the operation group list */
int engine_spawn_executors (engine_data_t *s, list_data_t* grouped_list){
		engine_data_t t=NULL;
		if ((s==(engine_data_t*)NULL)||(grouped_list==(list_data_t*)NULL)) return (-1);
		t=*s;
		return (0);
}
