#include <stdio.h>
#include "owl.h"

int main (int argc, char **argv);

int main (int argc, char **argv) {
		owl_model_t a=NULL;
		char* foaf="file:///tmp/foaf.rdf";
		char* doap="file:///tmp/redland.rdf";
		if (owl_model_create(&a)!=0) {
				fprintf(stderr,"ERRORE nella creazione del modello!\n");
				return (-1);
		}
		if (owl_model_add(&a,foaf)!=0) {
				fprintf(stderr,"ERRORE nel parsing\n");
				return (-1);
		}
		if (owl_model_add(&a,doap)!=0) {
				fprintf(stderr,"ERRORE nel parsing\n");
				return (-1);
		}
		fprintf(stdout,"Resulting model\n");
		owl_model_print_model(&a);
		owl_model_destroy(&a);
		return (0);
}
