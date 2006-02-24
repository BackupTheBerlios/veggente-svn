bin_PROGRAMS=mapper mapper_client

# gSOAP code generation
SOAP=soapcpp2
SOAP_C_CLIENT=soapClient.c soapC.c
SOAP_C_SERVER=soapServer.c soapC.c

SOAPHEADER=soap_request.h

SOAP_NS=map.wsdl

SOAP_C_SRC=$(SOAP_C_CLIENT) soapServer.c $(SOAP_H_FILES) $(SOAP_NS)

$(SOAP_C_SRC) : $(SOAPHEADER)
	$(SOAP) -c $(SOAP_FLAGS) $< 

BUILT_SOURCES=$(SOAP_C_SRC) 
CLEANFILES=$(SOAP_C_SRC) soapStub.h soapH.h soapClientLib.c soapServerLib.c *.req.xml *.res.xml *.wsdl *.nsmap *.xsd *Proxy\.h *~
# end of gSOAP stuff

#mapper_SOURCES= mapper.c owl_storage.c generic_list.c system.h ../aclocal.m4
mapper_SOURCES= thread_test.c engine.c generic_list.c owl_storage.c context.c request.c $(SOAP_C_SERVER) system.h ../aclocal.m4
#mapper_LDADD=@LIBS@
mapper_LDFLAGS=-lpthread -lstdsoap2 -lrdf -lcurl -ldl -lz -lidn

mapper_client_SOURCES= soap_client.c generic_list.c $(SOAP_C_CLIENT) system.h ../aclocal.m4
mapper_client_LDFLAGS=-lstdsoap2
