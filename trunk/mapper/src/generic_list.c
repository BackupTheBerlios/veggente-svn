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
		if (iter==NULL) return (0);
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
				free(iter1);
				return(0);
		}
		iter1=*s;
		while (iter1->next)	{
				if (iter1->next==*node) {
						iter2=iter1->next;
						iter1->next=iter1->next->next;
						free(iter2);
						return (0);
				}
				iter1=iter1->next;
		}
		return (-1);
}

int list_remove_data(list_data_t* s, void* data,int (*compare)(void*, void*)) {
		list_data_t iter1,iter2;
		if ((s==(list_data_t*)NULL)||(data==NULL)) return (-1);
		if (*s==NULL) return (-1);
		iter1=*s;
		if (compare(&(iter1->payload),&data)==0) {
				*s=(*s)->next;
				free(iter1);
				return(0);
		}
		while(iter1->next) {
				if (compare(&(iter1->next->payload),&data)==0) {
						iter2=iter1->next;
						iter1->next=iter1->next->next;
						free(iter2);
						return(0);
				}
				iter1=iter1->next;
		}
		return (-1);
}

int list_find(list_data_t* s, list_data_t *result,void *data,int (*compare)(void*, void*)) {
		list_data_t iter=NULL;
		if ((s==(list_data_t*)NULL)||(data==NULL)) return (-1);
		iter=*s;
		while(iter->next){
				if (compare(&(iter->payload),&data)==0) {
						result=&iter;
						return(0);
				}
				iter=iter->next;
		}
		if (compare(&(iter->payload),&data)==0) {
				result=&iter;
				return(0);
		}
		return (-1);
}

int list_next(list_data_t *s,list_data_t* result) {
		list_data_t iter=NULL;
		if (s==(list_data_t*)NULL) return (-1);
		iter=*s;
		*result=iter->next;
		return (0);
}

int list_next_from_node(list_data_t *s, list_data_t* node, list_data_t* result) {
		list_data_t iter=NULL;
		if (s==(list_data_t*)NULL) return (-1);
		iter=*s;
		if (node==(list_data_t*)NULL) {
				*result=iter;
				return (0);
		}
		while(iter->next){
				if (iter==*node) {
						*result=iter->next;
						return(0);
				}
				iter=iter->next;
		}
		if (iter==*node) {
				*result=iter->next;
				return(0);
		}
		return(0);
}

int list_next_from_data(list_data_t *s, list_data_t* result,void* data,int (*compare)(void*, void*)) {
		list_data_t iter=NULL;
		if (s==(list_data_t*)NULL) return (-1);
		iter=*s;
		if (data==(void*)NULL) {
				result=&iter;
				return (0);
		}
		while(iter->next){
				if (compare(&(iter->payload),&data)==0) {
						result=&(iter->next);
						return(0);
				}
				iter=iter->next;
		}
		if (compare(&(iter->payload),&data)==0) {
				result=&(iter->next);
				return(0);
		}
		return(0);
}

int list_get_payload(list_data_t* node, void** payload) {
		list_data_t t=NULL;
		if (node==(list_data_t*)NULL) return (-1);
		t=*node;
		*payload=t->payload;
		return (0);
}

int list_get_head(list_data_t *s, list_data_t* node){
		if (s==(list_data_t*)NULL) return (-1);
		*node=*s;
		return (0);
}

/* Don't use, only for debug purpose */
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
