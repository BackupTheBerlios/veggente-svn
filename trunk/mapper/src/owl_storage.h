/*	
 *	Veggente - owl_storage.h
 *	OWL storage and API for operations on OWL documents
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

#ifndef __OWL_MODEL_H
#define __OWL_MODEL_H

struct owl_storage;
typedef struct owl_storage *owl_storage_t;

#define OVERWRITE 	1
#define KEEP		0

int owl_storage_create(owl_storage_t *s,char* datadir);
int owl_storage_destroy(owl_storage_t *s);
int owl_storage_add(owl_storage_t *s, char* uri, int overwrite);
int owl_storage_remove(owl_storage_t *s, char* uri);
int owl_storage_merge(owl_storage_t *s, char* source_rdf_uri, char* dest_rdf_uri, char* map_file_uri);
int owl_storage_clone(owl_storage_t *s,owl_storage_t *dest);
int owl_storage_print_model(owl_storage_t *s);

#endif
