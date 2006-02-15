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
