/*	
 *	Veggente - generic_list.c
 *	Generic list implementation
 *	
 *	Copyright(c) 2006 Alessio Carenini <carenini@gmail.com>
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *	*/

#include <stdio.h>
#include <stdlib.h>
#include "generic_list.h"

struct list_data {
		void* payload;
		list_data_t next;
};


int list_init(list_data_t* s) {
		if (s==NULL) return (-1);
		*s=NULL;
		return (0);
}

int list_destroy(list_data_t* s) {
		list_data_t iter=NULL;
		if (s==(list_data_t*)NULL) return (-1);
		iter=*s;
		while(iter->next) {
				if (list_remove_node(s,&iter)!=0) return (-1);
				iter=*s;
		}
		if (list_remove_node(s,&iter)!=0) return (-1);
		return (0);
}

int list_add(list_data_t* s, void *data) {
		list_data_t t=NULL;
		list_data_t new_element=NULL;
		
		if (s==(list_data_t*)NULL) return (-1);
		
		/*Allocazione primo elemento*/
		new_element=(list_data_t)calloc(1,sizeof(struct list_data));
		if (new_element==NULL) return (-1);
		new_element->payload=data;
		fprintf(stdout,"Aggiunta dell'elemento %s\n",(char*)(new_element->payload));
		new_element->next=NULL;
		
		if (*s==NULL) {
				*s=new_element;
		}
		else {
				t=*s;
				while(t->next) t=t->next;
				t->next=new_element;
		}
		return (0);
}

int list_remove_node(list_data_t* s, list_data_t* node) {
		list_data_t iter1, iter2;
		if ((s==(list_data_t*)NULL)||(node==(list_data_t*)NULL)) return (-1);
		if (*s==NULL) return (-1);
		if ((*s)==*node) {
				iter1=*s;
				*s=(*s)->next;
				fprintf(stdout,"Rimozione dell'elemento %s\n",(char*)(iter1->payload));
				free(iter1);
				return(0);
		}
		iter1=*s;
		while (iter1->next)	{
				if (iter1->next==*node) {
						iter2=iter1->next;
						iter1->next=iter1->next->next;
						fprintf(stdout,"Rimozione di %s\n",(char*)(iter2->payload));
						free(iter2);
						return (0);
				}
				iter1=iter1->next;
		}
		return (-1);
}

int list_remove_data(list_data_t* s, void* data) {
		list_data_t iter1,iter2;
		if ((s==(list_data_t*)NULL)||(data==NULL)) return (-1);
		if (*s==NULL) return (-1);
		if ((*s)->payload==data) {
				iter1=*s;
				*s=(*s)->next;
				fprintf(stdout,"Rimozione dell'elemento %s\n",(char*)(iter1->payload));
				free(iter1);
				return(0);
		}
		iter1=*s;
		while(iter1->next) {
				if (iter1->next->payload==data) {
						iter2=iter1->next;
						iter1->next=iter1->next->next;
						fprintf(stdout,"Rimozione dell'elemento %s\n",(char*)(iter2->payload));
						free(iter2);
						return(0);
				}
				iter1=iter1->next;
		}
		return (-1);
}

int list_find(list_data_t* s, list_data_t *result,void *data) {
		list_data_t iter=NULL;
		if ((s==(list_data_t*)NULL)||(data==NULL)) return (-1);
		iter=*s;
		while(iter->next){
				if (iter->payload==data) {
						result=&iter;
						return(0);
				}
				iter=iter->next;
		}
		if (iter->payload==data) {
				result=&iter;
				return(0);
		}
		return (-1);
}

int list_next(list_data_t *s, list_data_t* node, list_data_t* result) {
		list_data_t iter=NULL;
		if ((s==(list_data_t*)NULL)||(node==(list_data_t*)NULL)) return (-1);
		while(iter->next){
				if (iter==*node) {
						result=&(iter->next);
						return(0);
				}
				iter=iter->next;
		}
		if (iter==*node) {
				result=&(iter->next);
				return(0);
		}
		iter=*s;
		return(0);
}

int list_dump(list_data_t* s) {
		list_data_t t=*s;
		if (s==(list_data_t*)NULL) return (-1);
		if (t==NULL) {
				fprintf(stdout,"Lista vuota\n");
				return (0);
		}
		while (t->next!=NULL) {
				fprintf(stdout,"Elemento %s\n",(char*)t->payload);
				t=t->next;
		}
		fprintf(stdout,"Elemento %s\n",(char*)t->payload);
		return(0);
}
