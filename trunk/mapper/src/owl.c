#include <stdio.h>
#include <stdlib.h>

#include "owl.h"
#ifndef librdf
#include <redland.h>
#define librdf
#endif

struct owl_obj {
		char* uri;
		/* Redland stuff*/
		librdf_world* rdf_world;
		librdf_storage* storage;
		librdf_model* model;
};

int owl_obj_create(owl_obj_t *s){
		owl_obj_t t=NULL;

		if (s==(owl_obj_t*)NULL) return (-1);
		t=(owl_obj_t)calloc(1,sizeof(struct owl_obj));
		if (!t) return (-1);
		*s=t;
		t->rdf_world=NULL;
		t->storage=NULL;
		t->model=NULL;
		
		t->rdf_world=librdf_new_world();
		if (t->rdf_world==NULL) {
				fprintf(stderr, "Failed to initialize RDF library\n");
				return (-1);
		}
#ifdef VERBOSE
		fprintf(stdout,"RDF library initialized\n");
#endif
		librdf_world_open(t->rdf_world);

		t->storage=librdf_new_storage(t->rdf_world,"memory",NULL,NULL);
		if (t->storage==NULL) {
				fprintf(stderr, "Failed to initialize RDF storage\n");
				return (-1);
		}
#ifdef VERBOSE
		fprintf(stdout,"RDF storage initialized\n");
#endif
		t->model=librdf_new_model(t->rdf_world, t->storage, NULL);
	  	if(t->model==NULL) {
				fprintf(stderr, "Failed to create RDF model\n");
				return(-1);
	  	}	
#ifdef VERBOSE
		fprintf(stdout,"RDF model initialized\n");
#endif
		
		return (0);
}
int owl_obj_destroy(owl_obj_t *s){
		owl_obj_t t=*s;
		if (t->uri)
		{
				free(t->uri);
				t->uri=NULL;
		}
		librdf_free_model(t->model);
		librdf_free_storage(t->storage);
		librdf_free_world(t->rdf_world);
		free(t);
		return 0;
}
int owl_obj_add(owl_obj_t *s, char* uri) {
		owl_obj_t t=*s;
		librdf_parser* parser=NULL;
		librdf_uri* rdf_uri=NULL;
		
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
  		librdf_free_parser(parser);
		return (0);
}
/* Merge two owl object with the aid of a map object*/
int owl_obj_merge(owl_obj_t *s, char* onto1_uri, char* onto2_uri, char* map_file_uri){
		return 0;
}
int owl_obj_print_model(owl_obj_t *s) {
		fprintf(stdout,"RDF model output:\n");
		librdf_model_print((*s)->model, stdout);
		return (0);
}
