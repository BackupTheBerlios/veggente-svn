#ifndef __OWL_MODEL_H
#define __OWL_MODEL_H

struct owl_model;
/*TODO: 
 * permettere anche la cancellazione di un documento dal modello*/
typedef struct owl_model *owl_model_t;

int owl_model_create(owl_model_t *s);
int owl_model_destroy(owl_model_t *s);
int owl_model_add(owl_model_t *s, char* uri);
int owl_model_remove(owl_model_t *s, char* uri);
int owl_model_merge(owl_model_t *s, char* onto1_uri, char* onto2_uri, char* map_file_uri);
int owl_model_print_model(owl_model_t *s);

#endif
