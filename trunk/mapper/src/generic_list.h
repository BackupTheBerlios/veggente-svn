/*	
 *	Veggente - generic list
 *	Generic list
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

#ifndef __GENERIC_LIST_H
#define __GENERIC_LIST_H

struct list_data;

typedef struct list_data *list_data_t;

int list_init(list_data_t* s);
int list_destroy(list_data_t* s);
int list_add(list_data_t* s, void *data);
int list_remove_data(list_data_t* s, void* data,int (*compare)(void*, void*));
int list_remove_node(list_data_t* s, list_data_t* node);
int list_find(list_data_t* s, list_data_t *result,void *data,int (*compare)(void*, void*));
int list_next(list_data_t *s,list_data_t* result);
int list_next_from_node(list_data_t *s, list_data_t* node, list_data_t* result);
int list_next_from_data(list_data_t *s, list_data_t* result,void* data,int (*compare)(void*, void*));
int list_dump(list_data_t* s);
int list_get_payload(list_data_t* node, void* payload);
#endif
