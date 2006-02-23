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

#include "engine.h"
#include "request.h"
#include "context.h"
#include "generic_list.h"

#ifndef THREAD_LIMIT
#define THREAD_LIMIT 10
#endif

struct engine_data{
		pthread_mutex_t queue_mutex;
		pthread_cond_t queue_signal;
		pthread_t queued_id;
		pthread_t requestd_id;
		int op_limit;  /* Limit of operation after which starts queue processing */
		int op_count;
		int timeout_sec; /* Every how many seconds queue processing is forced */
		context_t context_data;
		list_data_t* op_list;
		list_data_t* processed_list;
		int (*group_ops)(list_data_t*,list_data_t*);
		int (*load_function)();
};

struct op_data {
		pthread_mutex_t op_mutex;
		void* request;
		void* response;
};

/* Request threads' data*/
struct serve_data {
		list_data_t dead_list;
		pthread_mutex_t dead_list_lock;
		soap_t soap_env;
		context_t context_data;
};
typedef struct serve_data *serve_data_t;

/* Thread specific functions */
void* engine_queue_daemon(void* manager_data);
void* engine_request_daemon(void* manager_data);
void* engine_serve_request(void* serve_thread_data);

int engine_spawn_executors (engine_data_t *s, list_data_t* grouped_list);
int engine_cleanup_threads(list_data_t *s);

/* Engine constructor 
 * op_limit: limit in queue length after which start immediate processing 
 * group_ops: function to group operations which share the same resources
 * load_function: return the optimal number of thread to spawn based on system
 * 		load and queue length*/
int engine_init(engine_data_t *s,
				int op_limit,
				int (*group_ops)(list_data_t*,list_data_t*),
				int (*load_function)()) {
		engine_data_t t=NULL;
		t=(engine_data_t)calloc(1,sizeof(struct engine_data));
		if (t==NULL) return (-1);
		/* Engine data init */
		if (list_init(t->op_list)!=0) {
				fprintf(stderr,"[Engine] Error initializing operations list\n");
				return (-1);
		}
		if (list_init(t->processed_list)) {
				fprintf(stderr,"[Engine] Error initializing processed operations list\n");
				return (-1);
		}
		if (context_init(&(t->context_data))!=0) {
				fprintf(stderr,"[Engine] Error initializing context application data\n");
				return (-1);
		}
		t->op_count=0;
		t->op_limit=op_limit;
		pthread_mutex_init(&(t->queue_mutex),NULL);
		pthread_cond_init(&(t->queue_signal),NULL);
		*s=t;
		return (0);
}

/* Starts execution of a previously created engine */
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
		fprintf(stdout,"[Engine] Creation of the queue handler thread\n");
		if (pthread_create(&(t->queued_id),&th_attr,engine_queue_daemon,(void*)s)) {
				fprintf(stderr,"[Engine] Error creating queue handler\n");
				return (-1);
		}
		/* Create request handler thread */
		fprintf(stdout,"[Engine] Creation of the request handler thread\n");
		if (pthread_create(&(t->requestd_id),&th_attr,engine_request_daemon,(void*)s)) {
				fprintf(stderr,"[Engine] Error creating request handler\n");
				return (-1);
		}
		pthread_attr_destroy(&th_attr);
		if (pthread_join(t->queued_id,(void**)&ret_code)) {
				fprintf(stderr,"[Engine] Queue handler exited with status %d\n",ret_code);
				return (ret_code);
		}
		if (pthread_join(t->requestd_id,(void**)&ret_code)) {
				fprintf(stderr,"[Engine] Request handler exited with status %d\n",ret_code);
				return (ret_code);
		}
		return (0);
}

/* Engine data destructor/deallocator */
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
		t=*((engine_data_t*)manager_data);
		mutex_code=pthread_mutex_lock(&(t->queue_mutex));

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
										fprintf(stdout,"[Queued] Timeout: Queue processing...\n");
										/* Group operations */
										if ( ((t->group_ops)(t->op_list,&group_list))!=0 ) {
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
								/* Group operations */
								fprintf(stdout,"[Queued] Timeout: Queue processing...\n");
								if ( ((t->group_ops)(t->op_list,&group_list))!=0 ) {
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
				if (t->op_count<0) {
						fprintf(stderr,"[Queued] Error in queue length\n");
						pthread_mutex_unlock(&(t->queue_mutex));
						pthread_exit((void*)-1);
				}
		}

		pthread_mutex_unlock(&(t->queue_mutex));
		pthread_exit((void*)0);
}


void* engine_request_daemon(void* manager_data) {
		int sock=0;
		int retcode=0;
		pthread_t th_id;
		list_data_t req_threads_list=NULL;
		pthread_mutex_t req_list_lock;
		soap_t soap_env=NULL;
		soap_t soap_clone=NULL;
		serve_data_t thread_data=NULL;
		engine_data_t t=*((engine_data_t*)manager_data);
		
		if (t==(engine_data_t)NULL) pthread_exit((void*)-1);
		pthread_mutex_init(&req_list_lock,NULL);
		
		if (context_get_soap(&(t->context_data),&soap_env)!=0) {
				fprintf(stderr,"[Requestd] Error getting SOAP environment\n");
				pthread_exit((void*)-1);
		}
		if (list_init(&req_threads_list)) {
				fprintf(stderr,"[Requestd] Error initializing dead threads list\n");
				pthread_exit((void*)-1);
		}

		fprintf(stdout,"[Requestd] Thread started\n");
		while (1) {
				sock=soap_accept(soap_env);
				if (!soap_valid_socket(sock)) {
						fprintf(stderr,"[Requestd] Error accepting SOAP connections\n");
						retcode=-1;
						break;
				}

				/* Allocates thread data */
				thread_data=(serve_data_t)calloc(1,sizeof(struct serve_data));
				if (thread_data==NULL) {
						fprintf(stderr,"[Requestd] Error allocating thread data\n");
						retcode=-1;
						break;
				}

				/* Fill thread data*/
				thread_data->dead_list=req_threads_list;
				thread_data->dead_list_lock=req_list_lock;
				thread_data->context_data=t->context_data;
				thread_data->soap_env=soap_clone;

				/* Add thread data to list */
				pthread_mutex_lock(&req_list_lock);
				if (list_add(&req_threads_list,(void*)thread_data)) {
						fprintf(stderr,"[Requestd] Error adding thread data to control list\n");
						pthread_mutex_unlock(&req_list_lock);
						retcode=-1;
						break;
				}
				pthread_mutex_unlock(&req_list_lock);

				/* Start the thread */
				if (pthread_create(&th_id,NULL,engine_serve_request,(void*)thread_data)) {
						fprintf(stderr,"[Requestd] Error starting request worker thread\n");
						retcode=-1;
						break;
				}
				/* Cleanup dead threads */
				if (engine_cleanup_threads(&req_threads_list)!=0) {
						fprintf(stderr,"[Requestd] Error cleaning up dead threads data\n");
						retcode=-1;
						break;
				}
		}
		list_destroy(&req_threads_list);
		soap_done(soap_env);
		pthread_exit((void*)retcode);
}

void* engine_serve_request(void* serve_thread_data) {
		serve_data_t t=NULL;
		t=*((serve_data_t*)serve_thread_data);
		soap_serve(t->soap_env);
		soap_destroy(t->soap_env);
		soap_end(t->soap_env);
		soap_done(t->soap_env);
		free(t->soap_env);
		free(t->context_data);
		free(t);
		pthread_exit((void*)0);
}

/* Process the operation group list */
int engine_spawn_executors (engine_data_t *s, list_data_t* grouped_list){
		engine_data_t t=NULL;
		int i=0;
		int thread_limit=0;
		if ((s==(engine_data_t*)NULL)||(grouped_list==(list_data_t*)NULL)) return (-1);
		t=*s;
		thread_limit=(t->load_function)();
		while (i<thread_limit) {
				i++;
		}
		return (0);
}

int engine_cleanup_threads(list_data_t *s){
		return(0);
}
