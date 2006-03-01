#include <stdio.h>
#include "generic_list.h"

int main (int argc, char** argv);

int main (int argc, char** argv) {
		list_data_t a=NULL;
		list_data_t res=NULL;
		char* el1="primo";
		char* el2="secondo";
		char* el3="terzo";
		char* el4="quarto";
		char* el5="quinto";
		void* load=NULL;
		
		if (list_init(&a)!=0) {
				fprintf(stderr,"Errore in creazione\n");
				return (-1);
		}
		if (list_add(&a,(void*)el1)!=0) {
				fprintf(stderr,"Errore di inserimento\n");
		}
		list_add(&a,(void*)el2);
		list_add(&a,(void*)el3);
		list_add(&a,(void*)el4);
		list_add(&a,(void*)el5);
		list_dump(&a);
		fprintf(stdout,"Rimozione passo passo\n");
		while (list_next_from_node(&a,&res,&res)!=-1) {
				list_get_payload(&res,&load);
				fprintf(stdout,"Vittima-> Elemento %s\n",load);
				if (list_remove_node(&a,&res)==0) {
						res=NULL;
				} else break;
		}
		list_add(&a,(void*)el4);
		list_add(&a,(void*)el5);
		list_dump(&a);
		while (list_next_from_node(&a,&res,&res)!=-1) {
				list_get_payload(&res,&load);
				fprintf(stdout,"Vittima-> Elemento %s\n",load);
				if (list_remove_node(&a,&res)==0) {
						res=NULL;
				} else break;
		}
		list_dump(&a);
		list_destroy(&a);
		return (0);
}
