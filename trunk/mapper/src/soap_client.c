/* 
   Veggente - mapper SOAP client

   Copyright (C) 2006 Alessio Carenini

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  

*/

#include <stdio.h>

#include "soapH.h"
#include "map.nsmap"

#include <sys/types.h>
#include <getopt.h>
#include "system.h"

static void usage (int status);

/* The name the program was run with, stripped of any leading path. */
char *program_name;

/* getopt_long return codes */
enum {DUMMY_CODE=129
      ,ADD_URI,DEL_URI
};

/* Option flags and variables */

char *add_uri;	/* --add URI */
char *del_uri;	/* --del URI */
int want_verbose;		/* --verbose */

static struct option const long_options[] =
{
  {"verbose", no_argument, 0, 'v'},
  {"add", required_argument, 0, ADD_URI},
  {"del", required_argument, 0, DEL_URI},
  {"help", no_argument, 0, 'h'},
  {"version", no_argument, 0, 'V'},
  {NULL, 0, NULL, 0}
};

static int decode_switches (int argc, char **argv);

int main (int argc, char **argv) {
		int i;
		struct soap *soap_env;
		
		program_name = argv[0];
		if (argc==1) usage(-1);
		i = decode_switches (argc, argv);
		soap_init(soap_env);
		if (add_uri!=NULL) {
				fprintf(stdout,"Add URI: %s\n",add_uri);
				/* Cleanup */
				free(add_uri);
		}
		if (del_uri!=NULL) {
				fprintf(stdout,"Del URI: %s\n",del_uri);
				/* Cleanup */
				free(del_uri);
		}
		soap_destroy(soap_env);
		soap_end(soap_env);
		soap_done(soap_env);
		/* do the work */
		return (0);
}

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
				  case ADD_URI:	/* --directory */
						  add_uri=(char*)malloc(sizeof(char)*strlen(optarg));
						  memcpy(add_uri,optarg,sizeof(char)*strlen(optarg));
						  break;
				  case DEL_URI:	/* --directory */
						  if (add_uri==NULL) {
								  del_uri=(char*)malloc(sizeof(char)*strlen(optarg));
								  memcpy(del_uri,optarg,sizeof(char)*strlen(optarg));
								  break;
						  }
						  else {
								  fprintf(stderr,"Error: only one option between add and remove can be present\n");
								  usage(-1);
						  }
				  case 'V':
						  fprintf (stdout,"Veggente SOAP mapper client %s\n", VERSION);
						  exit (0);
				  case 'h':
						  usage (0);
				  default:
						  usage (-1);
				}
		}
		return optind;
}


static void
usage (int status)
{
  printf (_("%s - \
Veggente SOAP mapper client\n"), program_name);
  printf (_("Usage: %s [OPTION]... [FILE]...\n"), program_name);
  printf (_("\
Options:\n\
  --verbose                  print more information\n\
  --add URI 		         add URI to repository\n\
  --del URI 		         remove URI from repository\n\
  -h, --help                 display this help and exit\n\
  -V, --version              output version information and exit\n\
"));
  exit (status);
}
