#include <stdio.h>
#include <stdlib.h>
#include "request.h"
struct request {
		char* uri;
		int action;
};

struct map_request {
		char* base_onto;
		char* dest_onto;
		char* map_uri;
};

int request_create(request_t *s, int action, char* uri) {
		request_t t=NULL;
		if (s==(request_t*)NULL) return(-1);
		t=(request_t)calloc(1,sizeof(struct request));
		*s=t;
		if (t==NULL) return (-1);
		t->action=action;
		t->uri=uri;
		return(0);
}

int request_destroy(request_t *s) {
		request_t t=NULL;
		if (s==(request_t*)NULL) return(-1);
		*s=t;
		free(t->uri);
		free(t);
		return(0);
}

int map_request_create(map_request_t *s, char* base_onto_uri, char* dest_onto_uri, char* map_uri) {
		map_request_t t=NULL;
		if (s==(map_request_t*)NULL) return(-1);
		t=(map_request_t)calloc(1,sizeof(struct map_request));
		*s=t;
		if (t==NULL) return (-1);
		t->base_onto=base_onto_uri;
		t->dest_onto=dest_onto_uri;
		t->map_uri=map_uri;
		return(0);
}

int map_request_destroy(map_request_t *s) {
		map_request_t t=NULL;
		if (s==(map_request_t*)NULL) return(-1);
		*s=t;
		free(t->base_onto);
		free(t->dest_onto);
		free(t->map_uri);
		free(t);
		return(0);
}
