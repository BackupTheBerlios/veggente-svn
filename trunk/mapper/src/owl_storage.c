/*	
 *	Veggente - owl_storage.c
 *	OWL storage and API for operations on OWL documents
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
#include <string.h>
#include "config.h"
#include "owl_storage.h"
#include "generic_list.h"
#include <redland.h>
/* If under WIN32 and VCC compiler, snprintf becomes _snprintf*/
#ifdef HAVE_SNPRINTF
	#define SNPRINTF snprintf
#else
	#define SNPRINTF _snprintf
#endif
struct owl_storage {
		list_data_t uri_list;
		/* Redland stuff*/
		librdf_world* rdf_world;
		librdf_storage* storage;
		librdf_model* model;
};

int uri_compare(void* uri1, void* uri2);

int owl_storage_create(owl_storage_t *s, char* datadir){
		owl_storage_t t=NULL;
		char* storage_options=NULL;
		int options_size=0;
		

		if (s==(owl_storage_t*)NULL) return(-1);
		t=(owl_storage_t)calloc(1,sizeof(struct owl_storage));
		if (!t) return (-1);
		*s=t;
		t->rdf_world=NULL;
		t->storage=NULL;
		t->model=NULL;
		/* Build options string */
		options_size=strlen(datadir)+50;
		storage_options=(char*)calloc(options_size,sizeof(char));
		options_size=(strlen(datadir)+50)*sizeof(char);
		if (storage_options==NULL) {
				free(t);
				return (-1);
		}
		if((SNPRINTF(storage_options,options_size,"hash-type='bdb',dir='%s',contexts='yes',write='yes'",datadir))>options_size) {
				free(storage_options);
				free(t);
				return (-1);
		}
		
		if(list_init(&(t->uri_list))!=0) {
				fprintf(stderr, "Failed to initialize URI list\n");
				free(storage_options);
				free(t);
				return (-1);
		}
		/* libRDF init */
		t->rdf_world=librdf_new_world();
		if (t->rdf_world==NULL) {
				fprintf(stderr, "Failed to initialize RDF library\n");
				list_destroy(&(t->uri_list));
				free(storage_options);
				free(t);
				return (-1);
		}
		librdf_world_open(t->rdf_world);

		t->storage=librdf_new_storage(t->rdf_world,"hashes","owldb",storage_options);
		if (t->storage==NULL) {
				fprintf(stderr, "Failed to initialize RDF storage\n");
				librdf_free_world(t->rdf_world);
				list_destroy(&(t->uri_list));
				free(storage_options);
				free(t);
				return (-1);
		}
		t->model=librdf_new_model(t->rdf_world, t->storage, NULL);
	  	if(t->model==NULL) {
				fprintf(stderr, "Failed to create RDF model\n");
				librdf_free_storage(t->storage);
				librdf_free_world(t->rdf_world);
				list_destroy(&(t->uri_list));
				free(storage_options);
				free(t);
				return (-1);
	  	}	
		free(storage_options);
		return (0);
}
int owl_storage_destroy(owl_storage_t *s){
		owl_storage_t t=*s;
		
		if (s==(owl_storage_t*)NULL) return (-1);
		if (t->uri_list!=(list_data_t)NULL) {
				if (list_destroy(&(t->uri_list))!=0) {
						fprintf(stderr,"Failed to destroy uri list\n");
				}
		}
		if ((t->model)!=(librdf_model*)NULL) {
				librdf_free_model(t->model);
		}
		if ((t->storage)!=(librdf_storage*)NULL) {
				librdf_free_storage(t->storage);
		}
		if ((t->rdf_world)!=(librdf_world*)NULL) {
				librdf_free_world(t->rdf_world);
		}
		free(t);
		return (0);
}

/* If overwrite==1 then first delete URI context, then insert data*/
int owl_storage_add(owl_storage_t *s, char* uri,  int overwrite) {
		owl_storage_t t=*s;
		librdf_stream* rdf_stream=NULL;
		librdf_parser* parser=NULL;
		librdf_uri* rdf_uri=NULL;
		librdf_statement* current=NULL;
		librdf_node* context=NULL;
		
		
		/* Initialize librdf and parse as a stream */
		if ((s==(owl_storage_t*)NULL)||(uri==(char*)NULL)) return (-1);
		fprintf(stdout,"RDF: %s\n",uri);
		
		parser=librdf_new_parser(t->rdf_world,"rdfxml",NULL,NULL);
		if (parser==NULL) {
				fprintf(stderr,"Failed to create parser\n");
				return (-1);
		}
		rdf_uri=librdf_new_uri(t->rdf_world,(unsigned char*)uri);
		if (rdf_uri==NULL) {
				fprintf(stderr,"Malformed or nonexistent URI\n");
				librdf_free_parser(parser);
				return (-1);
		}
		fprintf(stdout,"Parsing URI %s\n",uri);
		rdf_stream=librdf_parser_parse_as_stream(parser,rdf_uri,NULL);
		if (rdf_stream==NULL) {
				fprintf(stderr,"Failed to parse %s into model\n", uri);
				librdf_free_uri(rdf_uri);
				librdf_free_parser(parser);
				return (-1);
		}
		/* Add RDF stream to model with context */
		context=librdf_new_node_from_uri(t->rdf_world,rdf_uri);
		if (context==NULL) {
				fprintf(stderr,"Failed to parse %s into model\n", uri);
				librdf_free_stream(rdf_stream);
				librdf_free_uri(rdf_uri);
				librdf_free_parser(parser);
				return (-1);
		}
		if ((overwrite==1)&&(librdf_model_contains_context(t->model,context))) {
				if (owl_storage_remove(&t,uri)!=0) {
						fprintf(stderr,"Failed to overwrite %s triples in model\n", uri);
						librdf_free_node(context);
						librdf_free_stream(rdf_stream);
						librdf_free_uri(rdf_uri);
						librdf_free_parser(parser);
						return (-1);
				}
		}
		while (!librdf_stream_end(rdf_stream)) {
				current=librdf_new_statement_from_statement(librdf_stream_get_object(rdf_stream));
				if (current!=NULL) {
						if (librdf_model_context_add_statement(t->model,context,current)) {
								fprintf(stderr,"Failed to add %s triples into model\n", uri);
								librdf_free_statement(current);
								librdf_free_node(context);
								librdf_free_stream(rdf_stream);
								librdf_free_uri(rdf_uri);
								librdf_free_parser(parser);
								return (-1);
						}
						librdf_free_statement(current);
						librdf_stream_next(rdf_stream);
				}
		}
		if (list_add(&(t->uri_list),(void*)uri)) {
				fprintf(stderr,"Failed to add URI to list\n");
				librdf_free_node(context);
				librdf_free_stream(rdf_stream);
				librdf_free_uri(rdf_uri);
				librdf_free_parser(parser);
				return (-1);
		}
		librdf_free_node(context);
		librdf_free_stream(rdf_stream);
		librdf_free_uri(rdf_uri);
		librdf_free_parser(parser);
		return (0);
}

/* Remove all statements associated with the URI context from storage 
 * TODO: remove URI from list */
int owl_storage_remove(owl_storage_t *s, char* uri) {
		owl_storage_t t=NULL;
		librdf_node* context=NULL;
		librdf_uri* document_uri=NULL;
		
		if ((s==(owl_storage_t*)NULL)||(uri==NULL)) return (-1);
		t=*s;
		document_uri=librdf_new_uri(t->rdf_world,(unsigned char*)uri);
		if (document_uri==NULL) {
				fprintf(stderr,"Malformed or nonexistent URI\n");
				return (-1);
		}
		context=librdf_new_node_from_uri(t->rdf_world,document_uri);
		if (context==NULL) {
				fprintf(stderr,"Failed to parse %s into model\n", uri);
				librdf_free_uri(document_uri);
				return (-1);
		}
  		if (librdf_model_context_remove_statements(t->model,context)!=0) {
				fprintf(stderr,"Failed to remove %s from storage\n", uri);
				librdf_free_node(context);
				librdf_free_uri(document_uri);
				return (-1);
		}
		librdf_free_node(context);
		librdf_free_uri(document_uri);
		return (0);
}
		
/* Merge two owl object with the aid of a map object*/
int owl_storage_merge(owl_storage_t *s,  char* onto1_uri, char* onto2_uri, char* map_file_uri){
		owl_storage_t t=NULL;
		list_data_t result=NULL;
		
		if (s==(owl_storage_t*)NULL) return(-1);
		t=*s;
		if (list_find(&(t->uri_list),&result,(void*)&onto1_uri,&uri_compare)) {
				fprintf (stderr,"Error searching for %s in RDF model\n",onto1_uri);
				return (-1);
		}
		if (result!=NULL) {
				if(owl_storage_add(&t,onto1_uri,1)) {
						fprintf (stderr,"Error inserting %s in RDF model\n",onto1_uri);
						return (-1);
				}
		}
		if (list_find(&(t->uri_list),&result,(void*)&onto2_uri,&uri_compare)) {
				fprintf (stderr,"Error searching for %s in RDF model\n",onto2_uri);
				return (-1);
		}
		if (result!=NULL) {
				if(owl_storage_add(&t,onto2_uri,1)) {
						fprintf (stderr,"Error inserting %s in RDF model\n",onto2_uri);
						return (-1);
				}
		}
	return (0);
}

int owl_storage_print_model(owl_storage_t *s) {
		if (s==(owl_storage_t*)NULL) return(-1);
		fprintf(stdout,"RDF model output:\n");
		librdf_model_print((*s)->model, stdout);
		return (0);
}

int owl_storage_clone(owl_storage_t *s,owl_storage_t *dest) {
		owl_storage_t t=NULL;
		owl_storage_t target=NULL;
		if (s=(owl_storage_t*)NULL) return (-1);
		target=(owl_storage_t)calloc(1,sizeof(struct owl_storage));
		if (target==NULL) return (-1);
		target=(owl_storage_t)memcpy((void*)target,(void*)t,sizeof(struct owl_storage));
		dest=&target;
		return (0);
}

int uri_compare(void* uri1, void* uri2) {
		return strcmp((char*)uri1,(char*)uri2);
}
