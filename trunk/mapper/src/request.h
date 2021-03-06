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

/* Type codes for request */
#define REQUEST_DOC 1
#define REQUEST_MAP 0

/* Operation type codes */
#define REQUEST_DOC_ADD 1
#define REQUEST_DOC_DEL 0

#define OPERATION_PENDING 0
#define OPERATION_COMPLETED 1
/* All request structures goes here */

/* Generic request */
struct request;
typedef struct request *request_t;

/* Generic response */
struct response;
typedef struct response *response_t;

/* Generic operation */
struct operation;
typedef struct operation *operation_t;

/* Document-level request on database */
struct doc_request;
typedef struct doc_request *doc_request_t;
int doc_request_create(doc_request_t *s, int action, char* uri);
int doc_request_destroy(doc_request_t *s);

/* Mapping request */
struct map_request;
typedef struct map_request *map_request_t;
int map_request_create(map_request_t *s, char* base_rdf_uri, char* dest_rdf_uri, char* map_uri);
int map_request_destroy(map_request_t *s);

/* Get request type, useful for correct casting */
int request_get_type(request_t* s);

/* Operation */
int operation_create(operation_t *op, request_t *req);
int operation_destroy(operation_t *op);
int operation_get_request(operation_t *op, request_t *req);
int operation_get_response(operation_t *op, response_t *res);
int operation_lock(operation_t *op);
int operation_unlock(operation_t *op);
int operation_signal(operation_t *op);

/* Response */
int response_create(response_t *res);
int response_destroy(response_t *res);

/* Functions to execute a request */
int exec_request(request_t* req,response_t* res);

#endif
