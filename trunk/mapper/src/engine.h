/*	
 *	Veggente - engine.h
 *	Generic multithreaded request processor
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
#ifndef __ENGINE_H
#define __ENGINE_H
#include "request.h"
#include "generic_list.h"
#include "context.h"

#define THREAD_RUNNING 	0
#define THREAD_DEAD		1

#define REQUEST_THREAD 	0
#define QUEUE_THREAD 	1

struct engine_data;
typedef struct engine_data *engine_data_t;
struct op_data;
typedef struct op_data *op_data;
int engine_init(engine_data_t *s, int op_limit, context_t *context, int (*group_ops)(list_data_t*,list_data_t*), int (*load_function)());
int engine_start(engine_data_t *s);
int engine_destroy(engine_data_t *s);
int engine_post_request(engine_data_t *s,void* request_data,void* response_data);

#endif
