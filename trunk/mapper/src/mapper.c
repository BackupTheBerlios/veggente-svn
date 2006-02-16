#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include "system.h"
#include "owl_storage.h"

int main (int argc, char **argv);

static void usage (int status, char* program_name);
static int decode_switches (int argc, char **argv);

/* getopt_long return codes */
enum {DUMMY_CODE=129
      ,NOWARN_CODE
};

static struct option const long_options[] =
{
  {"verbose", no_argument, 0, 'v'},
  {"help", no_argument, 0, 'h'},
  {"version", no_argument, 0, 'V'},
  {NULL, 0, NULL, 0}
};

int want_verbose;		/* --verbose */
int want_no_warn;		/* --no-warn */

/* Set all the option flags according to the switches specified.
   Return the index of the first non-option argument.  */

static int decode_switches (int argc, char **argv) {
		int c;
		while ((c = getopt_long (argc, argv, 
										"v"	/* verbose */
										"h"	/* help */
										"V",	/* version */
										long_options, (int *) 0)) != EOF) {
				switch (c) {
						case 'v':		/* --verbose */
								want_verbose = 1;
								break;
						case NOWARN_CODE:	/* --no-warn */
								want_no_warn = 1;
								break;
						case 'V':
								printf ("mapper %s\n", VERSION);
								exit (0);
								
						case 'h':
								usage (0,argv[0]);
								
						default:
								usage (EXIT_FAILURE,argv[0]);
				}
		}
		return optind;
}

static void usage (int status, char* program_name) {
		printf (_("%s - Provide and execute semantic mapping\n"), program_name);
		printf (_("Usage: %s [OPTION]... [FILE]...\n"), program_name);
		printf (_("\
Options:\n\
	--verbose                  print more information\n\
	-h, --help                 display this help and exit\n\
	-V, --version              output version information and exit\n\
"));
	  	exit (status);
}

int main (int argc, char *argv[]) {
		owl_storage_t a=NULL;
		char* foaf="file:///tmp/foaf.rdf";
		char* doap="file:///tmp/redland.rdf";
		char* dir="/tmp/owldb";
		decode_switches(argc,argv);
		if (owl_storage_create(&a,dir)!=0) {
				fprintf(stderr,"ERRORE nella creazione del modello!\n");
				return (-1);
		}
		fprintf(stdout,"Storage creato\n");
		if (owl_storage_add(&a,foaf,1)!=0) {
				fprintf(stderr,"ERRORE nel parsing\n");
				owl_storage_destroy(&a);
				return (-1);
		}
		if (owl_storage_add(&a,doap,1)!=0) {
				fprintf(stderr,"ERRORE nel parsing\n");
				owl_storage_destroy(&a);
				return (-1);
		}
		fprintf(stdout,"Resulting model\n");
		owl_storage_print_model(&a);
		owl_storage_remove(&a,doap);
		fprintf(stdout,"\n Dopo la remove \n");
		owl_storage_print_model(&a);
		owl_storage_destroy(&a);
		return (0);
}
