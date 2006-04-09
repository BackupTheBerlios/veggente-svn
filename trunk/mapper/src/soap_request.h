//gsoap ns service name:	map
//gsoap ns service namespace:	http://veggente.berlios.de/ns/map.wsdl
//gsoap ns service location:	http://localhost:15000
//gsoap ns schema namespace:	urn:veggente_map
int ns__exec_doc_add_request(char* uri,int *result);
int ns__exec_doc_del_request(char* uri,int *result);
int ns__exec_map_request(char* source_rdf,char* map_file, char* dest_rdf, int* result);
