#ifndef __OWL_MODEL_H
#define __OWL_MODEL_H

struct owl_storage;
typedef struct owl_storage *owl_storage_t;

int owl_storage_create(owl_storage_t *s,char* datadir);
int owl_storage_destroy(owl_storage_t *s);
int owl_storage_add(owl_storage_t *s, char* uri, int overwrite);
int owl_storage_remove(owl_storage_t *s, char* uri);
int owl_storage_merge(owl_storage_t *s, char* onto1_uri, char* onto2_uri, char* map_file_uri);
int owl_storage_print_model(owl_storage_t *s);

#endif
