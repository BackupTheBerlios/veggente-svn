#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "owl_storage.h"
#include "generic_list.h"
#ifndef LIBRDF
#include <redland.h>
	#define LIBRDF
#endif
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

int owl_storage_create(owl_storage_t *s, char* datadir){
		owl_storage_t t=NULL;
		char* storage_options=NULL;
		int options_size=0;
		int ret_code=0;

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
				ret_code=-1;
		}
		if((SNPRINTF(storage_options,options_size,"hash-type='bdb',dir='%s',contexts='yes',write='yes'",datadir))>options_size) {
				ret_code=-1;
				goto dealloc;
		}
		
		if(list_init(&(t->uri_list))!=0) {
				fprintf(stderr, "Failed to initialize URI list\n");
				ret_code=-1;
				goto dealloc;
		}
		/* libRDF init */
		t->rdf_world=librdf_new_world();
		if (t->rdf_world==NULL) {
				fprintf(stderr, "Failed to initialize RDF library\n");
				ret_code=-1;
				goto dealloc;
		}
		librdf_world_open(t->rdf_world);

		t->storage=librdf_new_storage(t->rdf_world,"hashes","owldb",storage_options);
		if (t->storage==NULL) {
				fprintf(stderr, "Failed to initialize RDF storage\n");
				ret_code=-1;
				goto dealloc;
		}
		t->model=librdf_new_model(t->rdf_world, t->storage, NULL);
	  	if(t->model==NULL) {
				fprintf(stderr, "Failed to create RDF model\n");
				ret_code=-1;
				goto dealloc;
	  	}	
		
dealloc:
		free(storage_options);
		if (ret_code!=0) {
				list_destroy(&(t->uri_list));
				free(t);
		}
		return (ret_code);
}
int owl_storage_destroy(owl_storage_t *s){
		owl_storage_t t=*s;
		int ret_code=0;
		if (s==(owl_storage_t*)NULL) return (-1);
		if (t->uri_list!=(list_data_t)NULL) {
				if (list_destroy(&(t->uri_list))!=0) {
						fprintf(stderr,"Failed to destroy uri list\n");
						ret_code=-1;
						goto dealloc;
				}
		}
dealloc:librdf_free_model(t->model);
		librdf_free_storage(t->storage);
		librdf_free_world(t->rdf_world);
		free(t);
		return (ret_code);
}

/* If overwrite==1 then first delete URI context, then insert data*/
int owl_storage_add(owl_storage_t *s, char* uri,  int overwrite) {
		owl_storage_t t=*s;
		librdf_stream* rdf_stream=NULL;
		librdf_parser* parser=NULL;
		librdf_uri* rdf_uri=NULL;
		librdf_statement* current=NULL;
		librdf_node* context=NULL;
		int ret_code=0;
		
		/* Initialize librdf and parse as a stream */
		if (s==(owl_storage_t*)NULL) return (-1);
		fprintf(stdout,"RDF: %s\n",uri);
		if (list_add(&(t->uri_list),(void*)uri)) {
				fprintf(stderr,"Failed to add URI to list\n");
				ret_code=-1;
				goto end;
		}
		
		parser=librdf_new_parser(t->rdf_world,"rdfxml",NULL,NULL);
		if (parser==NULL) {
				fprintf(stderr,"Failed to create parser\n");
				ret_code=-1;
				goto end;
		}
		rdf_uri=librdf_new_uri(t->rdf_world,(unsigned char*)uri);
		if (rdf_uri==NULL) {
				fprintf(stderr,"Malformed or nonexistent URI\n");
				ret_code=-1;
				goto parser;
		}
		fprintf(stdout,"Parsing URI %s\n",uri);
		rdf_stream=librdf_parser_parse_as_stream(parser,rdf_uri,NULL);
		if (rdf_stream==NULL) {
				fprintf(stderr,"Failed to parse %s into model\n", uri);
				ret_code=-1;
				goto uri;
		}
		/* Add RDF stream to model with context */
		context=librdf_new_node_from_uri(t->rdf_world,rdf_uri);
		if (context==NULL) {
				fprintf(stderr,"Failed to parse %s into model\n", uri);
				ret_code=-1;
				goto stream;
		}
		if ((overwrite==1)&&(librdf_model_contains_context(t->model,context))) {
				if (owl_storage_remove(&t,uri)!=0) {
						fprintf(stderr,"Failed to overwrite %s triples in model\n", uri);
						ret_code=-1;
						goto context;
				}
		}
		while (!librdf_stream_end(rdf_stream)) {
				current=librdf_new_statement_from_statement(librdf_stream_get_object(rdf_stream));
				if (current!=NULL) {
						if (librdf_model_context_add_statement(t->model,context,current)) {
								fprintf(stderr,"Failed to add %s triples into model\n", uri);
								librdf_free_statement(current);
								ret_code=-1;
								goto context;
						}
						librdf_free_statement(current);
						librdf_stream_next(rdf_stream);
				}
		}
context:librdf_free_node(context);
stream:	librdf_free_stream(rdf_stream);
uri:	librdf_free_uri(rdf_uri);
parser:	librdf_free_parser(parser);
end:	return (ret_code);
}

/* Remove all statements associated with the URI context from storage */
int owl_storage_remove(owl_storage_t *s, char* uri) {
		owl_storage_t t=NULL;
		librdf_node* context=NULL;
		librdf_uri* document_uri=NULL;
		int ret_code=0;
		
		if ((s==(owl_storage_t*)NULL)||(uri==NULL)) return (-1);
		t=*s;
		document_uri=librdf_new_uri(t->rdf_world,(unsigned char*)uri);
		if (document_uri==NULL) {
				fprintf(stderr,"Malformed or nonexistent URI\n");
				ret_code=-1;
				goto end;
		}
		context=librdf_new_node_from_uri(t->rdf_world,document_uri);
		if (context==NULL) {
				fprintf(stderr,"Failed to parse %s into model\n", uri);
				ret_code=-1;
				goto uri;
		}
  		if (librdf_model_context_remove_statements(t->model,context)!=0) {
				fprintf(stderr,"Failed to remove %s from storage\n", uri);
				ret_code=-1;
				goto context;
		}
context:librdf_free_node(context);
uri:	librdf_free_uri(document_uri);
end:	return (ret_code);
}
		
/* Merge two owl object with the aid of a map object*/
int owl_storage_merge(owl_storage_t *s,  char* onto1_uri, char* onto2_uri, char* map_file_uri){
		owl_storage_t t=NULL;
		list_data_t result=NULL;
		int ret_code=0;
		if (s==(owl_storage_t*)NULL) return(-1);
		t=*s;
		if (list_find(&(t->uri_list),&result,(void*)&onto1_uri)) {
				fprintf (stderr,"Error searching for %s in RDF model\n",onto1_uri);
				ret_code=-1;
				goto end;
		}
		if (result!=NULL) {
				if(owl_storage_add(&t,onto1_uri,1)) {
						fprintf (stderr,"Error inserting %s in RDF model\n",onto1_uri);
						ret_code=-1;
						goto end;
				}
		}
		if (list_find(&(t->uri_list),&result,(void*)&onto2_uri)) {
				fprintf (stderr,"Error searching for %s in RDF model\n",onto2_uri);
				ret_code=-1;
				goto end;
		}
		if (result!=NULL) {
				if(owl_storage_add(&t,onto2_uri,1)) {
						fprintf (stderr,"Error inserting %s in RDF model\n",onto2_uri);
						ret_code=-1;
						goto end;
				}
		}
end:	return (ret_code);
}

int owl_storage_print_model(owl_storage_t *s) {
		if (s==(owl_storage_t*)NULL) return(-1);
		fprintf(stdout,"RDF model output:\n");
		librdf_model_print((*s)->model, stdout);
		return (0);
}
