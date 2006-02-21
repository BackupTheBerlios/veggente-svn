#include <stdio.h>
#include <stdlib.h>
#include "engine.h"



int main (int argc, char* argv[]);


int main (int argc, char* argv[]) {
		engine_data_t s=NULL;
		if (engine_init(&s,10,NULL,NULL)!=0) {
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
		return (0);
}


