#include <stdio.h>
#include <stdlib.h>

#include "owl.h"
#include "generic_list.h"
#ifndef LIBRDF
#include <redland.h>
#define LIBRDF
#endif

struct owl_model {
		list_data_t uri_list;
		/* Redland stuff*/
		librdf_world* rdf_world;
		librdf_storage* storage;
		librdf_model* model;
};

int owl_model_create(owl_model_t *s){
		owl_model_t t=NULL;

		if (s==(owl_model_t*)NULL) return (-1);
		t=(owl_model_t)calloc(1,sizeof(struct owl_model));
		if (!t) return (-1);
		*s=t;
		t->rdf_world=NULL;
		t->storage=NULL;
		t->model=NULL;
		if(list_init(&(t->uri_list))!=0) {
				fprintf(stderr, "Failed to initialize URI list\n");
				return (-1);
		}
				
		t->rdf_world=librdf_new_world();
		if (t->rdf_world==NULL) {
				fprintf(stderr, "Failed to initialize RDF library\n");
				return (-1);
		}
		librdf_world_open(t->rdf_world);

		t->storage=librdf_new_storage(t->rdf_world,"memory",NULL,NULL);
		if (t->storage==NULL) {
				fprintf(stderr, "Failed to initialize RDF storage\n");
				return (-1);
		}
		t->model=librdf_new_model(t->rdf_world, t->storage, NULL);
	  	if(t->model==NULL) {
				fprintf(stderr, "Failed to create RDF model\n");
				return(-1);
	  	}	
		
		return (0);
}
int owl_model_destroy(owl_model_t *s){
		owl_model_t t=*s;
		if (s==(owl_model_t*)NULL) return (-1);
		if (t->uri_list!=(list_data_t)NULL) {
				if (list_destroy(&(t->uri_list))!=0) {
						fprintf(stderr,"Failed to destroy uri list\n");
						return (-1);
				}
		}
		librdf_free_model(t->model);
		librdf_free_storage(t->storage);
		librdf_free_world(t->rdf_world);
		free(t);
		return 0;
}
int owl_model_add(owl_model_t *s, char* uri) {
		owl_model_t t=*s;
		librdf_parser* parser=NULL;
		librdf_uri* rdf_uri=NULL;
		
		if (s==(owl_model_t*)NULL) return (-1);
		fprintf(stdout,"RDF: %s\n",uri);
		if (list_add(&(t->uri_list),(void*)uri)) {
				fprintf(stderr,"Failed to add URI to list\n");
				return (-1);
		}
		
		parser=librdf_new_parser(t->rdf_world,"rdfxml",NULL,NULL);
		if (parser==NULL) {
				fprintf(stderr,"Failed to create parser\n");
				return (-1);
		}
		rdf_uri=librdf_new_uri(t->rdf_world,uri);
		if (rdf_uri==NULL) {
  				librdf_free_parser(parser);
				fprintf(stderr,"Malformed or nonexistent URI\n");
				return (-1);
		}
		fprintf(stdout,"Parsing URI %s\n",uri);
		if (librdf_parser_parse_into_model(parser,rdf_uri,NULL,t->model)) {
				fprintf(stderr,"Failed to parse %s into model\n", uri);
  				librdf_free_uri(rdf_uri);
				librdf_free_parser(parser);
		}
  		librdf_free_uri(rdf_uri);
  		librdf_free_parser(parser);
		return (0);
}

int owl_model_remove(owl_model_t *s, char* uri) {
		owl_model_t t=NULL;
		librdf_parser* parser=NULL;
		librdf_uri* document_uri=NULL;
		librdf_stream* document_stream=NULL;
		librdf_statement* current=NULL;
		
		if ((s==(owl_model_t*)NULL)||(uri==NULL)) return (-1);
		t=*s;
		/* 1. Parse the URI into a stream */
		parser=librdf_new_parser(t->rdf_world,"rdfxml",NULL,NULL);
		if (parser==NULL) {
				fprintf(stderr,"Failed to create parser\n");
				return (-1);
		}
		document_uri=librdf_new_uri(t->rdf_world,uri);
		if (document_uri==NULL) {
  				librdf_free_parser(parser);
				fprintf(stderr,"Malformed or nonexistent URI\n");
				return (-1);
		}
		document_stream=librdf_parser_parse_as_stream(parser,document_uri,NULL);
		if (document_stream==NULL) {
  				librdf_free_parser(parser);
  				librdf_free_uri(document_uri);
				fprintf(stderr,"Error parsing %s\n",uri);
				return (-1);
		}
		/* 2. Remove statements from model */
		if (!librdf_stream_end(document_stream)) {
				current=librdf_new_statement_from_statement(librdf_stream_get_object(document_stream));
				do {
						if (librdf_storage_remove_statement(t->storage,current)) {
								fprintf(stderr,"Error removing statement\n RDF Model may be corrupted.\n");
								librdf_free_parser(parser);
								librdf_free_uri(document_uri);
								librdf_free_stream(document_stream);
								librdf_free_statement(current);
								return (-1);
						}
						librdf_free_statement(current);
						librdf_stream_next(document_stream);
						current=librdf_new_statement_from_statement(librdf_stream_get_object(document_stream));
				} while (current!=NULL);
		}
		/* 3. Free librdf stuff*/
		librdf_free_parser(parser);
  		librdf_free_uri(document_uri);
		librdf_free_stream(document_stream);
		return (0);
}
		
/* Merge two owl object with the aid of a map object*/
int owl_model_merge(owl_model_t *s, char* onto1_uri, char* onto2_uri, char* map_file_uri){
		owl_model_t t=NULL;
		list_data_t result=NULL;
		if (s==(owl_model_t*)NULL) return (-1);
		t=*s;
		if (list_find(&(t->uri_list),&result,(void*)&onto1_uri)) {
				fprintf (stderr,"Error searching for %s in RDF model\n",onto1_uri);
				return (-1);
		}
		if (result!=NULL) {
				if(owl_model_add(&t,onto1_uri)) {
						fprintf (stderr,"Error inserting %s in RDF model\n",onto1_uri);
						return (-1);
				}
		}
		if (list_find(&(t->uri_list),&result,(void*)&onto2_uri)) {
				fprintf (stderr,"Error searching for %s in RDF model\n",onto2_uri);
				return (-1);
		}
		if (result!=NULL) {
				if(owl_model_add(&t,onto2_uri)) {
						fprintf (stderr,"Error inserting %s in RDF model\n",onto2_uri);
						return (-1);
				}
		}
		return 0;
}
int owl_model_print_model(owl_model_t *s) {
		if (s==(owl_model_t*)NULL) return (-1);
		fprintf(stdout,"RDF model output:\n");
		librdf_model_print((*s)->model, stdout);
		return (0);
}
