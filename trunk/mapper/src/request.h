/*	
 *	Veggente - request.c
 *	Requests for operations
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

#ifndef __REQUEST_H
#define __REQUEST_H

#define REQUEST_ADD 1
#define REQUEST_DEL 0
/* All request structures goes here */

struct request;
typedef struct request *request_t;
/* Document-level request on database */
int request_create(request_t *s, int action, char* uri);
int request_destroy(request_t *s);

struct map_request;
typedef struct map_request *map_request_t;
/* Mapping request*/
int map_request_create(map_request_t *s, char* base_onto_uri, char* dest_onto_uri, char* map_uri);
int map_request_destroy(map_request_t *s);
#endif
