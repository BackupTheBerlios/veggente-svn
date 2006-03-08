/*	
 *	Veggente - request.c
 *	Requests for operations
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
#include "request.h"
#include "owl_storage.h"
#include "engine.h"

#include "soapH.h"
#ifndef __NSMAP
#define __NSMAP
#include "map.nsmap"
#endif

/* Operation data structure*/
struct operation{
		int status;
		pthread_mutex_t op_mutex;
		pthread_cond_t op_signal;
		request_t request;
		response_t response;
};

/** Request data superclass */
struct request {
		int type;
		owl_storage_t storage;
};

/** Document-class request */
struct doc_request {
		int type;
		owl_storage_t storage;
		char* uri;
		int action;
};

/** Map-class request*/
struct map_request {
		int type;
		owl_storage_t storage;
		char* source_rdf;
		char* dest_rdf;
		char* map_uri;
};

/** Response data superclass */
struct response {
		int type;
		int retval;
};

int exec_doc_request(doc_request_t* s);
int exec_map_request(map_request_t* s);

int doc_request_create(doc_request_t *s, int action, char* uri) {
		doc_request_t t=NULL;
		if (s==(doc_request_t*)NULL) return(-1);
		t=(doc_request_t)calloc(1,sizeof(struct doc_request));
		*s=t;
		if (t==NULL) return (-1);
		t->type=REQUEST_DOC;
		t->action=action;
		t->uri=uri;
		return(0);
}

int doc_request_destroy(doc_request_t *s) {
		doc_request_t t=NULL;
		if (s==(doc_request_t*)NULL) return(-1);
		*s=t;
		free(t->uri);
		free(t);
		return(0);
}

int map_request_create(map_request_t *s, char* base_rdf_uri, char* dest_rdf_uri, char* map_uri) {
		map_request_t t=NULL;
		if (s==(map_request_t*)NULL) return(-1);
		t=(map_request_t)calloc(1,sizeof(struct map_request));
		*s=t;
		if (t==NULL) return (-1);
		t->type=REQUEST_MAP;
		t->source_rdf=base_rdf_uri;
		t->dest_rdf=dest_rdf_uri;
		t->map_uri=map_uri;
		return(0);
}

int map_request_destroy(map_request_t *s) {
		map_request_t t=NULL;
		if (s==(map_request_t*)NULL) return(-1);
		*s=t;
		free(t->source_rdf);
		free(t->dest_rdf);
		free(t->map_uri);
		free(t);
		return(0);
}

int request_get_type(request_t* s){
		request_t t=NULL;
		if (s==(request_t*)NULL) return(-1);
		*s=t;
		return t->type; 
}

int exec_request(request_t* s,response_t *res) {
		int op_code=request_get_type(s);
		if (op_code==REQUEST_MAP) {
				return exec_map_request((map_request_t*)s);
		}
		if (op_code==REQUEST_DOC) {
				return exec_doc_request((doc_request_t*)s);
		}
		return (-1);
}

int exec_doc_request(doc_request_t* s) {
		doc_request_t t=NULL;
		if (s==(doc_request_t*)NULL) return(-1);
		if (t->action==REQUEST_DOC_ADD) {
				return (owl_storage_add( &(t->storage),
										t->uri,
										OVERWRITE ));
		}
		if (t->action==REQUEST_DOC_DEL) {
				return (owl_storage_remove( &(t->storage),
										t->uri ));
		}
		return (-1);
}

/* Execute a map request 
 * TODO: define function to use */
int exec_map_request(map_request_t* s) {
		if (s==(map_request_t*)NULL) return(-1);
		return (0);
}

/* Operation functions
 * An operation contains both the request and the response
 */
int operation_create(operation_t *op, request_t *req) {
		operation_t t=NULL;
		response_t new_response=NULL;
		if (op==(operation_t*)NULL) return (-1);
		t=(operation_t)calloc(1,sizeof(struct operation));
		if (t==NULL) return (-1);
		*op=t;
		if (response_create(&new_response)!=0) {
				fprintf(stderr,"Error creating response object\n");
				return (-1);
		}
		pthread_mutex_init(&(t->op_mutex),NULL);
		pthread_cond_init(&(t->op_signal),NULL);
		t->response=new_response;
		t->request=*req;
		t->status=OPERATION_PENDING;
		return (0);
}

int operation_destroy(operation_t *op) {
		operation_t t=NULL;
		if (op==(operation_t*)NULL) return (-1);
		t=*op;
		if (t->status==OPERATION_PENDING) {
				fprintf(stderr,"Attempted to destroy a pending operation\n");
				return (-1);
		}
		free(t->response);
		free(t->request);
		pthread_mutex_destroy(&(t->op_mutex));
		pthread_cond_destroy(&(t->op_signal));
		free (t);
		op=NULL;
		return (0);
}

int operation_get_request(operation_t *op, request_t *req) {
		operation_t t=NULL;
		if (op==(operation_t*)NULL) return (-1);
		t=*op;
		*req=t->request;
		return (0);
}

int operation_get_response(operation_t *op, response_t *res) {
		operation_t t=NULL;
		if (op==(operation_t*)NULL) return (-1);
		t=*op;
		*res=t->response;
		return (0);
}

int operation_lock(operation_t *op){
		operation_t t=NULL;
		if (op==(operation_t*)NULL) return (-1);
		t=*op;
		pthread_mutex_lock(&(t->op_mutex));
		return (0);
}

int operation_unlock(operation_t *op){
		operation_t t=NULL;
		if (op==(operation_t*)NULL) return (-1);
		t=*op;
		pthread_mutex_unlock(&(t->op_mutex));
		return (0);
}

/* Response functions */
int response_create(response_t *res) {
		return(0);
}

int response_destroy(response_t *res) {
		return(0);
}

/* SOAP functions */
int ns__exec_doc_add_request(struct soap *soap_env, char* uri, int *result){
		slave_data_t t=NULL;
		operation_t new_op=NULL;
		doc_request_t new_request=NULL;
		list_data_t pending_list=NULL;
		
		if (uri==(char*)NULL) return (-1);
		if (soap_env->user==NULL) return (-1);
		t=(slave_data_t)(soap_env->user);
		if (doc_request_create(&new_request,REQUEST_DOC_ADD,uri)!=0) {
				fprintf(stderr,"[SOAP] Error creating new operationn\n");
				return (-1);
		}
		/*TODO: check casting */
		if (operation_create(&new_op, (request_t*)&new_request)!=0) {
				fprintf(stderr,"[SOAP] Error creating new operation\n");
				return (-1);
		}
		if (engine_slave_get_op_list(&t,&pending_list)!=0) {
				fprintf(stderr,"[SOAP] Error getting pending operations list\n");
				return (-1);
		}
		if (list_add(&pending_list,new_op)!=0) {
				fprintf(stderr,"[SOAP] Error adding a new operation to pending ops list\n");
				return (-1);
		}
		pthread_mutex_lock(&(new_op->op_mutex));
		pthread_cond_wait(&(new_op->op_signal),&(new_op->op_mutex));
		/* Now the operation SHOULD be completed and the mutex re-acquired */
		if (new_op->response!=NULL) {
				*result=new_op->response->retval;
		}

		pthread_mutex_unlock(&(new_op->op_mutex));
		return (0);
}

int ns__exec_doc_del_request(struct soap *soap_env, char* uri, int *result){
		slave_data_t t=NULL;
		if (soap_env->user==NULL) return (-1);
		t=(slave_data_t)(soap_env->user);
		if (uri==(char*)NULL) return (-1);
		return (0);
}
int ns__exec_map_request(struct soap *soap_env, char* source_rdf, char* map_file, char* dest_rdf, int *result){
		slave_data_t t=NULL;
		if (soap_env->user==NULL) return (-1);
		t=(slave_data_t)(soap_env->user);
		return (0);
}
