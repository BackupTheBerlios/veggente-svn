/* 
   Veggente - mapper SOAP server

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

#include <sys/types.h>
#include <getopt.h>
#include "system.h"

#include "engine.h"
#include "context.h"

static void usage (int status);

/* The name the program was run with, stripped of any leading path. */
char *program_name;

/* getopt_long return codes */
enum {DUMMY_CODE=129
      ,PORT,DB_DIR
};

/* Option flags and variables */

int port;	/* Port number */
char *db_dir;	/* Database dir */
int want_verbose;		/* --verbose */

static struct option const long_options[] =
{
  {"verbose", no_argument, 0, 'v'},
  {"db_dir", required_argument, 0, DB_DIR},
  {"port", required_argument, 0, PORT},
  {"help", no_argument, 0, 'h'},
  {"version", no_argument, 0, 'V'},
  {NULL, 0, NULL, 0}
};

static int decode_switches (int argc, char **argv);

int main (int argc, char **argv) {
		int i;
		context_t config_data=NULL;
		engine_data_t s=NULL;
		
		program_name = argv[0];
		if (argc<2) usage(-1);
		i = decode_switches (argc, argv);
		if (context_init(&config_data,port,db_dir)!=0) {
				fprintf(stderr,"[Engine] Error initializing context application data\n");
				return (-1);
		}
		if (engine_init(&s,10,&config_data,&dummy_group,NULL)!=0) {
				fprintf(stderr,"Engine creation failed\n");
				return (-1);
		}
		if (engine_start(&s)!=0) {
				fprintf(stderr,"Engine error\n");
				return (-1);
		}
		if (engine_destroy(&s)!=0) {
				fprintf(stderr,"Error destroying engine\n");
				return(-1);
		}
		if (context_destroy(&config_data)!=0) {
				fprintf(stderr,"Error destroying config data\n");
				return(-1);
		}
		free (db_dir);
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
				  case DB_DIR: /* --db_dir */
						  db_dir=(char*)calloc(strlen(optarg)+1,sizeof(char));
						  memcpy(db_dir,optarg,sizeof(char)*strlen(optarg));
						  break;
				  case PORT:	/* --port */
						  port=atoi(optarg);
						  break;
				  case 'V':
						  fprintf (stdout,"Veggente SOAP mapper server %s\n", VERSION);
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
Veggente SOAP mapper server\n"), program_name);
  printf (_("Usage: %s [OPTION]... [FILE]...\n"), program_name);
  printf (_("\
Options:\n\
  --verbose                  print more information\n\
  --db_dir DIR 		         storage directory\n\
  --port PORT 		         port number\n\
  -h, --help                 display this help and exit\n\
  -V, --version              output version information and exit\n\
"));
  exit (status);
}
