#ifndef __GENERIC_LIST_H
#define __GENERIC_LIST_H

struct list_data;

typedef struct list_data *list_data_t;

int list_init(list_data_t* s);
int list_destroy(list_data_t* s);
int list_add(list_data_t* s, void *data);
int list_remove_data(list_data_t* s, void* data);
int list_remove_node(list_data_t* s, list_data_t* node);
int list_find(list_data_t* s, list_data_t* result,void *data);
int list_next(list_data_t* s, list_data_t* node, list_data_t* result);
int list_dump(list_data_t* s);

#endif
