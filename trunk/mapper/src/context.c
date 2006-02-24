/*	
 *	Veggente - context.c
 *	Context dependent data for application engine
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
#include "context.h"
#include "owl_storage.h"

struct context{
		owl_storage_t storage;
		soap_t soap_lib;
		int port;
};

/*
 * TODO: fix directory name 
 * */
int context_init(context_t* s) {
		context_t t=NULL;
		/* Port number and storage dir are to be extracted from conf file */
		char* dir="/tmp/owldb";
		int bind_port=7777;
		/* End of variables to be extracted */
		t=(context_t)calloc(1,sizeof(struct context));
		if (t==NULL) return (-1);
		t->soap_lib=(soap_t)calloc(1,sizeof(struct soap));
		if ((t->soap_lib)==NULL) return (-1);
		*s=t;
		if (owl_storage_create(&(t->storage),dir)!=0) {
				fprintf(stderr,"[Context] Error initializing owl storage\n");
				return (-1);
		}
		soap_init(t->soap_lib);
		if ((t->soap_lib)==NULL){
				fprintf(stderr,"[Context] Error initializing \n");
			   	return (-1);
		}
		t->port=bind_port;
		return (0);
}
int context_destroy(context_t* s) {
		context_t t=NULL;
		if (s==(context_t*)NULL) return (-1);
		t=*s;
		if (owl_storage_destroy(&(t->storage))!=0) {
				fprintf(stderr,"[Context] Error deallocating owl storage\n");
				return (-1);
		}
		soap_destroy(t->soap_lib);
		soap_end(t->soap_lib);
		soap_done(t->soap_lib);
		free(t->soap_lib);
	   	free(t);
		return (0);
}

int context_get_soap(context_t *s, soap_t *soap_env){
		context_t t=NULL;
		if (s==(context_t*)NULL) return (-1);
		t=*s;
		if ((t->soap_lib)==NULL) return (-1);
		*soap_env=t->soap_lib;
		return (0);
}

int context_get_port(context_t *s, int *port){
		context_t t=NULL;
		if (s==(context_t*)NULL) return (-1);
		t=*s;
		*port=t->port;
		return (0);
}
