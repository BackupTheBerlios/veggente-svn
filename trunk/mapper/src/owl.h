struct owl_obj;
/*TODO: 
 * inserire in owl_obj una lista di uri
 * permettere anche la cancellazione di un documento dal modello*/
typedef struct owl_obj *owl_obj_t;

int owl_obj_create(owl_obj_t *s);
int owl_obj_destroy(owl_obj_t *s);
int owl_obj_add(owl_obj_t *s, char* uri);
int owl_obj_merge(owl_obj_t *s, char* onto1_uri, char* onto2_uri, char* map_file_uri);
int owl_obj_print_model(owl_obj_t *s);

