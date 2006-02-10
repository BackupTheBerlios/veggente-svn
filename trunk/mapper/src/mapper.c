#include <stdio.h>
#include "owl.h"

int main (int argc, char **argv);

int main (int argc, char **argv) {
		owl_obj_t a=NULL;
		char* rdf="file:///tmp/foaf.rdf";
		if (owl_obj_create(&a)!=0) {
				fprintf(stderr,"ERRORE!\n");
				return (-1);
		}
		if (owl_obj_add(&a,rdf)!=0) {
				fprintf(stderr,"ERRORE nel parsing\n");
				return (-1);
		}
		fprintf(stdout,"Resulting model\n");
		owl_obj_print_model(&a);
		return (0);
}
