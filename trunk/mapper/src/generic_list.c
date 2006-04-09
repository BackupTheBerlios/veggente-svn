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
#include <pthread.h>
#include "generic_list.h"

struct list_data {
		void* payload;
		list_data_t next;
		int count;
		pthread_mutex_t mutex;
		pthread_cond_t signal;
};


int list_init(list_data_t* s) {
		list_data_t t=NULL;
		if (s==NULL) return (-1);
		t=(list_data_t)calloc(1,sizeof(struct list_data));
		pthread_mutex_init(&(t->mutex),NULL);
		pthread_cond_init(&(t->signal),NULL);
		t->count=0;
		*s=t;
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
		list_data_t head=NULL;
		list_data_t new_element=NULL;
		
		if ((s==(list_data_t*)NULL)||(data==NULL)) return (-1);
		t=*s;
		head=*s;
		pthread_mutex_lock(&(head->mutex));
		t->count=t->count+1;
		new_element=(list_data_t)calloc(1,sizeof(struct list_data));
		if (new_element==NULL) return (-1);
		new_element->payload=data;
		new_element->next=NULL;
		
		while(t->next) t=t->next;
		t->next=new_element;
		pthread_mutex_unlock(&(head->mutex));
		return (0);
}

int list_add_node(list_data_t* s, list_data_t *node) {
		list_data_t iter=NULL;
		list_data_t head=NULL;

		if ((s==(list_data_t*)NULL)||(node==(list_data_t*)NULL)) return (-1);
		if ((*s==NULL) && (*node!=NULL)) return (-1);
		if ((*s==NULL) && (*node==NULL)) return (0);
		
		iter=*s;
		head=*s;
		pthread_mutex_lock(&(head->mutex));
		iter->count=iter->count+1;
		while (iter->next) iter=iter->next;
		iter->next=*node;
		pthread_mutex_unlock(&(head->mutex));
		return (0);
}

int list_remove_node(list_data_t* s, list_data_t* node) {
		list_data_t iter=NULL;
		list_data_t head=NULL;
		list_data_t victim=NULL;
		
		if ((s==(list_data_t*)NULL)||(node==(list_data_t*)NULL)) return (-1);
		if ((*s==NULL) && (*node!=NULL)) return (-1);
		if ((*s==NULL) && (*node==NULL)) return (0);
		
		iter=*s;
		head=*s;
		pthread_mutex_lock(&(head->mutex));
		while (iter->next) {
				if (iter->next==*node) {
						victim=iter;
						iter->next=iter->next->next;
						free(victim);
						head->count=head->count-1;
						pthread_mutex_unlock(&(head->mutex));
						return (0);
				}
				iter=iter->next;
		}
		pthread_mutex_unlock(&(head->mutex));
		return (-1);
		
}

int list_remove_data(list_data_t* s, void* data,int (*compare)(void*, void*)) {
		list_data_t iter1,iter2;
		list_data_t head=NULL;

		if ((s==(list_data_t*)NULL)||(data==NULL)) return (-1);
		if (*s==NULL) return (-1);

		iter1=*s;
		head=*s;
		pthread_mutex_lock(&(head->mutex));
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
						head->count=head->count-1;
						pthread_mutex_unlock(&(head->mutex));
						return(0);
				}
				iter1=iter1->next;
		}
		pthread_mutex_unlock(&(head->mutex));
		return (-1);
}

int list_move_node(list_data_t *source_list, list_data_t *dest_list, list_data_t* node) {
		list_data_t iter=NULL;
		list_data_t head=NULL;
		list_data_t victim=NULL;
		
		if ( (source_list==(list_data_t*)NULL) || (node==(list_data_t*)NULL) ) return (-1);
		if ((*source_list==NULL) && (*node!=NULL)) return (-1);
		if ((*source_list==NULL) && (*node==NULL)) return (0);
	
		iter=*source_list;
		head=*source_list;
		pthread_mutex_lock(&(head->mutex));
		while (iter->next) {
				if (iter->next==*node) {
						victim=iter;
						iter->next=iter->next->next;
						head->count=head->count-1;
						pthread_mutex_unlock(&(head->mutex));
						return (list_add_node(dest_list,node));
				}
				iter=iter->next;
		}
		pthread_mutex_unlock(&(head->mutex));
		return (-1);
}

int list_find(list_data_t* s, list_data_t *result,void *data,int (*compare)(void*, void*)) {
		list_data_t iter=NULL;
		list_data_t head=NULL;
		
		if ((s==(list_data_t*)NULL)||(data==NULL)) return (-1);
		
		iter=*s;
		head=*s;
		pthread_mutex_lock(&(head->mutex));
		while(iter->next){
				if (compare(&(iter->payload),&data)==0) {
						result=&iter;
						pthread_mutex_unlock(&(head->mutex));
						return(0);
				}
				iter=iter->next;
		}
		if (compare(&(iter->payload),&data)==0) {
				result=&iter;
				pthread_mutex_unlock(&(head->mutex));
				return(0);
		}
		pthread_mutex_unlock(&(head->mutex));
		return (-1);
}

int list_next(list_data_t *s,list_data_t* result) {
		*result=(*result)->next;
		return (0);
}

int list_next_from_node(list_data_t *s, list_data_t* node, list_data_t* result) {
		list_data_t iter=NULL;
		list_data_t head=NULL;
	
		if ((s==(list_data_t*)NULL)||(*s==(list_data_t)NULL)) return (-1);
	
		if (*node==(list_data_t)NULL) {
				*result=*s;
				return (0);
		}
		iter=*s;
		head=*s;
		pthread_mutex_lock(&(head->mutex));
		if (iter==*node) {
				*result=iter->next;
				pthread_mutex_unlock(&(head->mutex));
				return (0);
		}
		while ((iter->next)!=NULL) {
				if (iter==*node) {
						*result=iter;
						pthread_mutex_unlock(&(head->mutex));
						return (0);
				}
				iter=iter->next;
		}
		pthread_mutex_unlock(&(head->mutex));
		return(-1);
}

int list_next_from_data(list_data_t *s, list_data_t* result,void* data,int (*compare)(void*, void*)) {
		list_data_t iter=NULL;
		list_data_t head=NULL;
		
		if (s==(list_data_t*)NULL) return (-1);
		
		iter=*s;
		head=*s;
		pthread_mutex_lock(&(head->mutex));
		if (data==(void*)NULL) {
				result=&iter;
				pthread_mutex_unlock(&(head->mutex));
				return (0);
		}
		while(iter->next){
				if (compare(&(iter->payload),&data)==0) {
						result=&(iter->next);
						pthread_mutex_unlock(&(head->mutex));
						return(0);
				}
				iter=iter->next;
		}
		if (compare(&(iter->payload),&data)==0) {
				result=&(iter->next);
				pthread_mutex_unlock(&(head->mutex));
				return(0);
		}
		pthread_mutex_unlock(&(head->mutex));
		return(0);
}

int list_get_payload(list_data_t* node, void** payload) {
		list_data_t t=NULL;
		if (node==(list_data_t*)NULL) return (-1);
		t=*node;
		if (t==(list_data_t)NULL) {
				*payload=NULL;
				return (0);
		}
		*payload=t->payload;
		return (0);
}

int list_get_head(list_data_t *s, list_data_t* node){
		if (s==(list_data_t*)NULL) return (-1);
		*node=*s;
		return (0);
}

int  list_count(list_data_t s) {
		if (s==(list_data_t)NULL) return (-1);
		return (s->count);
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
