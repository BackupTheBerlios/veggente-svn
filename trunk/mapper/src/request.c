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
#include "request.h"
#include "owl_storage.h"

struct request {
		int type;
		owl_storage_t storage;
};

struct doc_request {
		int type;
		owl_storage_t storage;
		char* uri;
		int action;
};

struct map_request {
		int type;
		owl_storage_t storage;
		char* source_rdf;
		char* dest_rdf;
		char* map_uri;
};

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

int exec_request(request_t* s) {
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
		map_request_t t=NULL;
		if (s==(map_request_t*)NULL) return(-1);
		return (0);
}

/* SOAP functions */
int ns__exec_doc_add_request(struct soap *soap_env, char** uri);
		return SOAP_OK;
}
int ns__exec_doc_del_request(struct soap *soap_env, char** uri);
		return SOAP_OK;
}
int ns__exec_map_request(struct soap *soap_env, char* source_rdf, char** map_file, char** dest_rdf);
		return SOAP_OK;
}
