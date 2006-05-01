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

#ifndef THREAD_LIMIT
#define THREAD_LIMIT 10
#endif

/** 
 * Engine data structure 
 */
struct engine_data{
		pthread_t queued_id;
		pthread_t requestd_id;
		/** Limit of operation after which starts queue processing */
		int op_limit;  
		int op_count;
		/** Every how many seconds queue processing is forced */
		int timeout_sec; 
		context_t context_data;
		/* List of operations and its mutex and condition variable*/
		list_data_t op_list;
		pthread_mutex_t *op_queue_mutex;
		pthread_cond_t *op_queue_signal;
		/* List of completed ops and its mutex and condition variable*/
		list_data_t processed_list;
		pthread_mutex_t *proc_queue_mutex;
		pthread_cond_t *proc_queue_signal;
		/* Slaves lists */
		/** Threads serving Requestd */
		list_data_t req_slave_list; 
		/** Threads serving Queued */
		list_data_t queue_slave_list; 	
		int (*group_ops)(list_data_t*,list_data_t*);
		int (*load_function)();
};

/**
 * Generic slave thread data (superclass-like)
 */
struct slave_data {
		int type;
		int status;
		pthread_t self_id;
		context_t *context_data;
		list_data_t list;
		pthread_mutex_t *list_mutex;
		pthread_cond_t *list_cond;
};

/**
 * Queue slave thread data
 */
struct queue_slave_data {
		/* Slave common variables */
		int type;
		int status;
		pthread_t self_id;
		context_t context_data;
		/* Shared list of processed items*/
		list_data_t processed_list;
		pthread_mutex_t *proc_queue_mutex;
		pthread_cond_t *proc_queue_signal;
		/* End of common variables */
		/* Thread private list of operations*/
		list_data_t private_op_list;
};

/** 
 * Request slave thread data
 */
struct req_slave_data {
		/* Slave common variables */
		int type;
		int status;
		pthread_t self_id;
		context_t context_data;
		list_data_t op_list;
		pthread_mutex_t *op_queue_mutex;
		pthread_cond_t *op_queue_signal;
		/* End of common variables */
		pthread_mutex_t dead_list_lock;
		list_data_t proc_list;
		pthread_mutex_t *proc_queue_mutex;
		pthread_cond_t *proc_queue_signal;
		/* SOAP environment */
		soap_t soap_env;
};

/* Thread specific functions */
void* engine_queue_daemon(void* manager_data);
void* engine_request_daemon(void* manager_data);
void* engine_serve_request(void* serve_thread_data);

int engine_spawn_executors (engine_data_t *s, list_data_t* grouped_list);
int engine_cleanup_threads(list_data_t *s);

/** 
 * Engine constructor
 * 
 * op_limit: limit in queue length after which start immediate processing 
 * group_ops: function to group operations which share the same resources
 * load_function: return the optimal number of thread to spawn based on system
 * load and queue length
 */
int engine_init(engine_data_t *s,
				int op_limit,
				context_t *context,
				int (*group_ops)(list_data_t*,list_data_t*),
				int (*load_function)()) {
		engine_data_t t=NULL;
		t=(engine_data_t)calloc(1,sizeof(struct engine_data));
		if (t==NULL) return (-1);
		t->op_queue_mutex=(pthread_mutex_t*)calloc(1,sizeof(pthread_mutex_t));
		t->op_queue_signal=(pthread_cond_t*)calloc(1,sizeof(pthread_cond_t));
		t->proc_queue_mutex=(pthread_mutex_t*)calloc(1,sizeof(pthread_mutex_t));
		t->proc_queue_signal=(pthread_cond_t*)calloc(1,sizeof(pthread_cond_t));
		
		if (context==(context_t*)NULL) return (-1);
		/* Engine data init */
		if (list_init(&(t->op_list))!=0) {
				fprintf(stderr,"[Engine] Error initializing operations list\n");
				return (-1);
		}
		if (list_init(&(t->processed_list))!=0) {
				fprintf(stderr,"[Engine] Error initializing processed operations list\n");
				return (-1);
		}
		if (list_init(&(t->req_slave_list))!=0) {
				fprintf(stderr,"[Engine] Error initializing Requested slaves list\n");
				return (-1);
		}
		if (list_init(&(t->queue_slave_list))!=0) {
				fprintf(stderr,"[Engine] Error initializing Queued slave list\n");
				return (-1);
		}
		t->context_data=*context;
		t->op_count=0;
		t->op_limit=op_limit;
		t->group_ops=group_ops;
		pthread_mutex_init(t->op_queue_mutex,NULL);
		pthread_cond_init(t->op_queue_signal,NULL);
		pthread_mutex_init(t->proc_queue_mutex,NULL);
		pthread_cond_init(t->proc_queue_signal,NULL);
		*s=t;
		return (0);
}

/** [Engine] Starts execution of a previously created engine */
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

/** [Engine] Data destructor/deallocator */
int engine_destroy(engine_data_t *s) {
		engine_data_t t=NULL;
		if (s==(engine_data_t*)NULL) return (-1);
		t=*s;
		list_destroy(&(t->op_list));
		list_destroy(&(t->processed_list));
		list_destroy(&(t->req_slave_list));
		list_destroy(&(t->queue_slave_list));
		pthread_mutex_destroy(t->op_queue_mutex);
		pthread_cond_destroy(t->op_queue_signal);
		pthread_mutex_destroy(t->proc_queue_mutex);
		pthread_cond_destroy(t->proc_queue_signal);
		free (t->op_queue_mutex);
		free (t->op_queue_signal);
		free (t->proc_queue_mutex);
		free (t->proc_queue_signal);
		pthread_cancel(t->requestd_id);
		pthread_cancel(t->queued_id);
		free(t);
		return (0);
}

/**
 * Queue daemon thread 
 * 
 * Executes the operations inside op_list and posts the results in 
 * processed_list.
 *  
 */

/* Function executed by "queued" thread  */
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
		mutex_code=pthread_mutex_lock(t->op_queue_mutex);

		while(1) {
				if (list_count(t->op_list)==0) {
						/* Wait if there are no requests */
						fprintf(stdout,"[Queued] Empty queue, sleeping\n");
						if (pthread_cond_wait(t->op_queue_signal,t->op_queue_mutex)!=0) {
										fprintf(stderr,"[Queued] Error waiting for lock\n");
										pthread_exit((void*)-1);
						}
				} 
				fprintf(stdout,"[Queued] Running...\n");
/*				if (t->op_count>0) {*/
				fprintf(stdout,"[Queued] Processing queue...\n");
						if (t->op_count < t->op_limit) {
								/* Timed wait*/
								gettimeofday(&now,NULL);
								timeout.tv_sec=now.tv_sec+t->timeout_sec;
								timeout.tv_nsec=now.tv_usec*1000;
								cond_code=pthread_cond_timedwait(t->op_queue_signal,t->op_queue_mutex,&timeout);
								if ((cond_code!=0)&&(cond_code!=ETIMEDOUT)) {
										fprintf(stderr,"[Queued] Error waiting for data\n");
										pthread_exit((void*)-1);
								}
						}
						/* Group operations */
						if ( ((t->group_ops)(&(t->op_list),&group_list))!=0 ) {
								fprintf(stderr,"[Queued] Error grouping operations\n");
								pthread_mutex_unlock(t->op_queue_mutex);
								pthread_exit((void*)-1);
						}
						/* Process operation groups */
						if (engine_spawn_executors(&t,&group_list)!=0) {
								fprintf(stderr,"[Queued] Error processing operations\n");
								pthread_mutex_unlock(t->op_queue_mutex);
								pthread_exit((void*)-1);
						}
						pthread_mutex_unlock(t->op_queue_mutex);
						/* Cleanup dead threads data */
						if (engine_cleanup_threads(&(t->req_slave_list))!=0) {
								fprintf(stderr,"[Queued] Error cleaning slave list\n");
								pthread_exit((void*)-1);
						}
/*				}*/
				if (t->op_count<0) {
						fprintf(stderr,"[Queued] Error in queue length\n");
						pthread_mutex_unlock(t->op_queue_mutex);
						pthread_exit((void*)-1);
				}
		}

		pthread_mutex_unlock(t->op_queue_mutex);
		pthread_exit((void*)0);
}


/**
 * Request daemon
 *
 * Handles incoming SOAP connections, creates requests in op_list and listens 
 * for results in processed_list.
 *
 */ 

/* Function executed by "requestd" thread */
void* engine_request_daemon(void* manager_data) {
		SOAP_SOCKET sock=0;
		SOAP_SOCKET bind_sock=0;
		int bind_port=0;
		int retcode=0;
		pthread_mutex_t req_list_lock;
		soap_t soap_env=NULL;
		soap_t soap_clone=NULL;
		req_slave_data_t thread_data=NULL;
		engine_data_t t=*((engine_data_t*)manager_data);

		if (t==(engine_data_t)NULL) pthread_exit((void*)-1);
		
		pthread_mutex_init(&req_list_lock,NULL);
		
		if (context_get_soap(&(t->context_data),&soap_env)!=0) {
				fprintf(stderr,"[Requestd] Error getting SOAP environment\n");
				pthread_exit((void*)-1);
		}
		if (context_get_port(&(t->context_data),&bind_port)!=0) {
				fprintf(stderr,"[Requestd] Error getting bind port from context\n");
				pthread_exit((void*)-1);
		}
		/* TODO: substitute "100" with config backlog */
		bind_sock=soap_bind(soap_env,NULL,bind_port,100);
		if (!soap_valid_socket(bind_sock)) {
				fprintf(stderr,"[Requestd] Error binding port %d\n",bind_port);
				pthread_exit((void*)-1);
		}
		soap_clone=(soap_t)calloc(1,sizeof(struct soap));
		if (soap_clone==NULL) {
				fprintf(stderr,"[Requestd] Error allocating SOAP copy env\n");
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
				thread_data=(req_slave_data_t)calloc(1,sizeof(struct req_slave_data));
				if (thread_data==NULL) {
						fprintf(stderr,"[Requestd] Error allocating thread data\n");
						retcode=-1;
						break;
				}
				soap_clone=soap_copy(soap_env);
				if (soap_clone==NULL) {
						fprintf(stderr,"[Requestd] Error copying SOAP env\n");
						retcode=-1;
						break;
				}

				/* Fill thread data*/
				thread_data->dead_list_lock=req_list_lock;
				thread_data->op_list=t->op_list;
				thread_data->op_queue_mutex=t->op_queue_mutex;
				thread_data->op_queue_signal=t->op_queue_signal;
				thread_data->context_data=t->context_data;
				thread_data->soap_env=soap_clone;
				thread_data->type=REQUEST_THREAD;
				thread_data->status=THREAD_RUNNING;

				list_add(t->op_list,"lista");
				list_add(thread_data->op_list,"thread_data");

				/* Add thread data to list */
				pthread_mutex_lock(&req_list_lock);
				if (list_add(t->req_slave_list,thread_data)) {
						fprintf(stderr,"[Requestd] Error adding thread data to control list\n");
						pthread_mutex_unlock(&req_list_lock);
						retcode=-1;
						break;
				}
				pthread_mutex_unlock(&req_list_lock);

				/* Start the thread */
				if (pthread_create(&(thread_data->self_id),NULL,engine_serve_request,thread_data)) {
						fprintf(stderr,"[Requestd] Error starting request worker thread\n");
						retcode=-1;
						break;
				}
				/* Cleanup dead threads */
				pthread_mutex_lock(&req_list_lock);
				if (engine_cleanup_threads(&(t->req_slave_list))!=0) {
						fprintf(stderr,"[Requestd] Error cleaning up dead threads data\n");
						retcode=-1;
						pthread_mutex_unlock(&req_list_lock);
						break;
				}

				pthread_mutex_unlock(&req_list_lock);
		}
		soap_done(soap_env);
		pthread_exit((void*)retcode);
}


/** [Requestd]: function executed by slave threads  */
void* engine_serve_request(void* serve_thread_data) {
		req_slave_data_t t=NULL;
		t=(req_slave_data_t)serve_thread_data;
		t->soap_env->user=(void*)t;
		soap_serve(t->soap_env);
		t->status=THREAD_DEAD;
		pthread_exit((void*)0);
}


/** [Queued]: function executed by slave threads  */
void* engine_process_request(void* proc_thread_data) {
		queue_slave_data_t t=NULL;
		list_data_t current_elem=NULL;
		void* payload=NULL;
		operation_t op=NULL;
		request_t req=NULL;
		response_t res=NULL;
		t=(queue_slave_data_t)proc_thread_data;
		fprintf(stdout,"[Queued] Processing requests...\n");
		/*Extract data and call exec request*/
		while (list_next_from_node(t->private_op_list,&current_elem,&current_elem)!=-1) {
			if (list_get_payload(current_elem,&payload)!=0){
						fprintf(stderr,"[Queued] Error getting request thread data payload\n");
						pthread_exit((void*)-1);
			}
			if (payload==NULL) pthread_exit((void*)-1);
			op=(operation_t)payload;
			/* Get request, response pointers and lock operation */
			if ( (operation_get_request(&op,&req)!=0)||
			 	 (operation_get_response(&op,&res)!=0) ||
				 (operation_lock(&op)!=0) ) {
						fprintf(stderr,"[Queued] Error extracting request\n");
						pthread_exit((void*)-1);
			}
			/* Executes request */
			if (exec_request(&req,&res)!=0) {
						fprintf(stderr,"[Queued] Error executing request\n");
						pthread_exit((void*)-1);
			}
			/*TODO: lock shared list of processed ops */
			/* Move operation from list of pending ops to list of processed ops */
			if (list_move_node(t->private_op_list,t->processed_list,&current_elem)) {
					fprintf(stderr,"[Queued] Error moving operation to processed list\n");
						pthread_exit((void*)-1);
			}
			if (operation_unlock(&op)!=0) {
						fprintf(stderr,"[Queued] Error unlocking operation\n");
						pthread_exit((void*)-1);
			}
			if (operation_signal(&op)!=0) {
					fprintf(stderr,"[Queued] Error signaling completed operation\n");
					pthread_exit((void*)-1);
			}
		}
		pthread_exit((void*)0);
}

/** [Queued] Process the operation group list */
int engine_spawn_executors (engine_data_t *s, list_data_t* grouped_list){
		engine_data_t t=NULL;
		int i=0;
		int thread_limit=0;
		list_data_t current=NULL;
		queue_slave_data_t thread_data=NULL;

		fprintf(stdout,"[Queued] Spawning executors...\n");
		if ((s==(engine_data_t*)NULL)||(grouped_list==(list_data_t*)NULL)) return (-1);
		t=*s;
		if (t->load_function!=NULL) {
				thread_limit=(t->load_function)();
		}
		else thread_limit=THREAD_LIMIT;
		while ((i<thread_limit)&&(list_next_from_node(*grouped_list,&current,&current)!=-1)) {
				fprintf(stdout,"Nuova operazione\n");
				if (current==(list_data_t)NULL) break;
				thread_data=(queue_slave_data_t)calloc(1,sizeof(struct queue_slave_data));
				if (thread_data==NULL) {
						fprintf(stderr,"[Queued] Failed allocating slave data\n");
						return (-1);
				}
				if (list_init(&(thread_data->private_op_list))!=0) {
						fprintf(stderr,"[Queued] Failed creating new list of operations\n");
						return (-1);
				}
				if (list_move_node(*grouped_list,thread_data->private_op_list,&current)!=0) {
						fprintf(stderr,"[Queued] Failed assigning operations to thread\n");
						return (-1);
				}
				thread_data->type=QUEUE_THREAD;
				thread_data->status=THREAD_RUNNING;
				thread_data->processed_list=t->processed_list;
				thread_data->context_data=t->context_data;
				thread_data->proc_queue_signal=t->proc_queue_signal;
				thread_data->proc_queue_mutex=t->proc_queue_mutex;

				if (list_add(t->queue_slave_list,&thread_data)!=0) {
						fprintf(stderr,"[Queued] Failed adding slave data to list\n");
						free(thread_data);
						return (-1);
				}
				if (pthread_create(&(thread_data->self_id),NULL,engine_process_request,(void*)thread_data)) {
						fprintf(stderr,"[Requestd] Error starting request worker thread\n");
						return (-1);
						
				}
				current=NULL;
				i++;
		}
		return (0);
}


/** [Engine]: calls join on dead threads and cleans up their data */
int engine_cleanup_threads(list_data_t *s) {
		list_data_t res=NULL;
		void* current_payload=NULL;
		slave_data_t data=NULL;
		req_slave_data_t data_req=NULL;
		queue_slave_data_t data_queue=NULL;
		int ret_code=0;

		if (s==(list_data_t*)NULL) return (-1);
		
		fprintf(stdout,"[Engine] Cleanup thread routine\n");
		while (list_next_from_node(*s,&res,&res)!=-1) {
				if (list_get_payload(res,(void*)&current_payload)!=0) {
						fprintf(stderr,"[Requestd] Error getting request thread data payload\n");
						return (-1);
				}
				if (current_payload==NULL) return (0);
				data=(slave_data_t)current_payload;
				/* Request thread cleanup */
				if (data->type==REQUEST_THREAD) {
						data_req=(req_slave_data_t)current_payload;
						if ((data_req->status==THREAD_DEAD)) {
								pthread_join(data_req->self_id,(void**)&ret_code);
								fprintf(stdout,"[Requestd] Joined thread %ld with return code: %d\n",data_req->self_id,ret_code);
								if (list_remove_node(*s,&res)!=0) {
										fprintf(stderr,"[Requestd] Error cleaning up thread data\n");
								}
								soap_destroy(data_req->soap_env);
								soap_end(data_req->soap_env);
								soap_done(data_req->soap_env);
								free(data_req->soap_env);
								free(data_req);
								data_req=NULL;
								res=NULL;
								data=NULL;
						}
				}
				/* Queue thread cleanup */
				else if (data->type==QUEUE_THREAD) {
						data_queue=(queue_slave_data_t)data;
						if ((data_queue!=NULL)&&(data_req->status==THREAD_DEAD)) {
								pthread_join(data_queue->self_id,(void**)&ret_code);
								fprintf(stdout,"[Queued] Joined thread %ld with return code: %d\n",data_queue->self_id,ret_code);
								if (list_remove_node(*s,&res)!=0) {
										fprintf(stderr,"[Requestd] Error cleaning up thread data\n");
								}
								/*TODO: clean list specific data*/
								free(data_queue);
								data_queue=NULL;
								res=NULL;
								data=NULL;
						}
				}
		}
		return(0);
}


int engine_slave_lock_ops_queue(slave_data_t *s){
		req_slave_data_t t=NULL;
		if (s==(slave_data_t*)NULL) return (-1);
		t=(req_slave_data_t)(*s);
		return(pthread_mutex_lock(t->op_queue_mutex));
}

int engine_slave_unlock_ops_queue(slave_data_t *s){
		req_slave_data_t t=NULL;
		if (s==(slave_data_t*)NULL) return (-1);
		t=(req_slave_data_t)(*s);
		return(pthread_mutex_unlock(t->op_queue_mutex));
}

int engine_slave_lock_proc_queue(slave_data_t *s){
		queue_slave_data_t t=NULL;
		if (s==(slave_data_t*)NULL) return (-1);
		t=(queue_slave_data_t)(*s);
		return(pthread_mutex_lock(t->proc_queue_mutex));
}

int engine_slave_unlock_proc_queue(slave_data_t *s){
		queue_slave_data_t t=NULL;
		if (s==(slave_data_t*)NULL) return (-1);
		t=(queue_slave_data_t)(*s);
		return(pthread_mutex_unlock(t->proc_queue_mutex));
}

int engine_enqueue_operation(slave_data_t *s, operation_t *op) {
		slave_data_t t=NULL;
		if ((s==(slave_data_t*)NULL)||(op==(operation_t*)NULL)) return (-1);
		t=*s;
		if (pthread_mutex_lock(t->list_mutex)!=0) {
				fprintf(stderr,"[Queued] Failed locking op_queue\n");
				return (-1);
		}
		if (list_add(t->list,op)!=0) {
				fprintf(stderr,"[Queued] Failed adding operation to op_queue\n");
				return (-1);
		}
		if (pthread_mutex_unlock(t->list_mutex)!=0) {
				fprintf(stderr,"[Queued] Failed unlocking op_queue\n");
				return (-1);
		}
		if (pthread_cond_signal(t->list_cond)!=0) {;
				fprintf(stderr,"[Queued] Failed signaling op_queue\n");
				return (-1);
		}
		return (0);
}

int dummy_group(list_data_t *origin, list_data_t *groups) {
		if (origin==(list_data_t*)NULL) return (-1);
		*groups=*origin;
		return (0);
}
