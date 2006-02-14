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
/* TODO: 
 * 1. Change name from owl_model to owl_storage
 * 2. Add a list of models and functions to handle multiple models
 * 3. Handle db directory as a parameter to storage creation function
 * 4. Adapt add & remove to a DB use
 * */
int owl_model_create(owl_model_t *s){
		owl_model_t t=NULL;
		char* storage_options="hash-type='bdb',dir='/tmp/owldb',contexts='yes',write='yes'";

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

		t->storage=librdf_new_storage(t->rdf_world,"hashes","owldb",storage_options);
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
		librdf_stream* rdf_stream=NULL;
		librdf_parser* parser=NULL;
		librdf_uri* rdf_uri=NULL;
		librdf_statement* current=NULL;
		librdf_node* context=NULL;
		/* Initialize librdf and parse as a stream */
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
		rdf_uri=librdf_new_uri(t->rdf_world,(unsigned char*)uri);
		if (rdf_uri==NULL) {
  				librdf_free_parser(parser);
				fprintf(stderr,"Malformed or nonexistent URI\n");
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
  				librdf_free_uri(rdf_uri);
				librdf_free_stream(rdf_stream);
				librdf_free_parser(parser);
				return (-1);
		}
		while (!librdf_stream_end(rdf_stream)) {
				current=librdf_new_statement_from_statement(librdf_stream_get_object(rdf_stream));
				if ((current==NULL)||(librdf_model_context_add_statement(t->model,context,current))) {
						fprintf(stderr,"Failed to add %s triples into model\n", uri);
						librdf_free_uri(rdf_uri);
						librdf_free_stream(rdf_stream);
						librdf_free_parser(parser);
						return (-1);
				}
				librdf_free_statement(current);
				librdf_stream_next(rdf_stream);
		}
		librdf_free_stream(rdf_stream);
  		librdf_free_uri(rdf_uri);
  		librdf_free_parser(parser);
		return (0);
}

/* Remove all statements associated with the URI context from storage */
int owl_model_remove(owl_model_t *s, char* uri) {
		owl_model_t t=NULL;
		librdf_node* context=NULL;
		librdf_uri* document_uri=NULL;
		
		if ((s==(owl_model_t*)NULL)||(uri==NULL)) return (-1);
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
