#!/usr/bin/python
# OWL-enabled RDF repository

from os import sys
from repository import Repository
from swap.stringwriter import StringWriter
import swap.cwm
import RDF
import SOAPpy
import getopt,sys

__version__='0.0.1'

class OWLRepository(Repository):
    """
    OWL-enabled RDF repository
    """
    
    debug_flag=True
    
    # These two variables have to be defined in some config file!!!
    owl_classes='http://veggente.berlios.de/think/owl.n3'
    owl_rules='http://veggente.berlios.de/think/owl_rules.n3'
    instance_rules='http://veggente.berlios.de/think/instance_rules.n3'

    def add_ontology(self, uri, overwrite=True):
        """
        uri:string : ontology uri
        @overwrite:boolean   : if True overwrite previously stored ontology
        Returns:
            0 on success
            -1 on failure
        """
        if (uri is None) or (uri==''):
            return -1
        if self.isPresent(uri):
            if overwrite==True:
                self.remove_ontology(uri,recursive=False)
            else:
                return 0
        Repository.add_document(uri)
        import_list=self.find_imports(uri)
        for doc in import_list:
            self.add_ontology(doc,False)
        inferred_statements=self.exec_ontology_inference(uri,import_list,overwrite)
        if inferred_statements is None:
            return -1
        for s in inferred_statements:
            self.model.add_statement(s,RDF.Node(RDF.Uri('think_'+uri)))
        return 0

    def remove_ontology(self,uri,recursive=False):
        """
        Removes an ontology and its inferred triples
        uri:string : ontology uri
        Returns:
            0 on success
            -1 on failure
        """
        if (uri is None) or (uri==''):
            return -1
        if (Repository.remove_document(uri)==0 and Repository.remove_document('think_'+uri)==0):
            if debug:
                print 'After remove'
                print self.list_documents()
            return 0
        else:
            return -1
        if recursive:
            for  m in self.find_imports(uri):
                remove_document(m,recursive)
            return 0
            
        
    def add_instance_document(self, uri, ontologies=[], think=True, overwrite=True):
        """
        uri:string : document uri
        ontologies: [string]: list of ontologies referred by the document
        overwrite:boolean   : if True overwrite previously stored ontology
        Returns:
            0 on success
            -1 on failure
        """
        if (uri is None) or (uri==''):
            return -1
        if ontologies==[]:
            return Repository.add_document(uri)
        for o in ontologies:
           self.add_ontology(o,False)
        inferred_statements=self.exec_instance_inference(uri, ontologies, overwrite)
        if inferred_statements is None:
            return -1
        for s in inferred_statements:
            self.model.add_statement(s,RDF.Node(RDF.Uri('think_'+uri)))
        return 0

    def remove_instance_document(self,uri):
        """
        Removes an instance document and its inferred triples
        uri:string : document uri
        Returns:
            0 on success
            -1 on failure
        """
        if (uri is None) or (uri==''):
            return -1
        if (Repository.remove_document(uri)==0 and Repository.remove_document('think_'+uri)==0):
            return 0
        else:
            return -1

    # TODO: move as an inner class method?
    def exec_ontology_inference(self, original_uri, uri_list=[], overwrite=False):
        """
        uri:string : ontology uri
        uri_list[]:string[] : list of import uri
        @overwrite:boolean   : if True overwrite previously inferred data
        Returns:
            list of statements
        """
        if (uri is None) or (uri==''):
            return None
        cmd=[]
        output=StringWriter()
        # I know it this UGLY, but if it works...
        cmd.append('cwm')
        cmd.append('--rdf')
        cmd.append(self.__find_location(uri))
        for i in uri_list:
            cmd.append(self.__find_location(i))
            if self.isPresent('think_'+i):
                cmd.append(self.__find_location('think_'+i))
        cmd.append(' --n3 ')
        cmd.append(self.owl_classes)
        cmd.append('--filter='+self.owl_rules)
        cmd.append('--think')
        cmd.append('--ugly')
        cmd.append('--purge') 
        cmd.append('--rdf')
        cwm.doCommand(cmd,output)
        cwm_out=output.getContent()
        if (cwm_out is None) or (cwm_out==[]):
            return None
        return self.parser.parse_string_as_stream(cwm_out,uri)

    # TODO: move as an inner class method?
    def exec_instance_inference(self, original_uri, uri_list=[], overwrite=False):
        """
        uri:string : ontology uri
        uri_list[]:string[] : list of import uri
        @overwrite:boolean   : if True overwrite previously inferred data
        Returns:
            list of statements
        """
        if (uri is None) or (uri==''):
            return None
        cmd=[]
        output=StringWriter()
        # I know it this UGLY, but if it works...
        cmd.append('cwm')
        cmd.append('--rdf')
        cmd.append(self.__find_location(uri))
        for i in uri_list:
            cmd.append(self.__find_location(i))
            if self.isPresent('think_'+i):
                cmd.append(self.__find_location('think_'+i))
        cmd.append(' --n3 ')
        cmd.append(self.owl_classes)
        cmd.append('--filter='+self.instance_rules)
        cmd.append('--think')
        cmd.append('--ugly')
        cmd.append('--purge') 
        cmd.append('--rdf')
        cwm.doCommand(cmd,output)
        cwm_out=output.getContent()
        if (cwm_out is None) or (cwm_out==[]):
            return None
        return self.parser.parse_string_as_stream(cwm_out,uri)
    
    def __find_location(self,uri):
        """
        Returns the original uri if the document is not in store, 
        or the Redland-VFS uri if already stored
        """
        for i in self.model.get_contexts():
            if str(i.uri)==uri:
                # URI is in store
                return self.db_uri+uri
        return uri
    
    def find_imports(self,uri):
        if (uri is None) or (uri==''):
            return None
        st=RDF.Statement(subject=RDF.Node(uri_string=uri),
                predicate=RDF.Node(uri_string='http://www.w3.org/2002/07/owl#imports'),
                object=None)
        global_imports=self.find_statements(st,uri)
        if global_imports is None:
            return None
        doc_imports=[]
        for i in global_imports:
            doc_imports.append(str(m.uri))
        return doc_imports
    
def usage():
    print "Veggente project: Conan OWL repository"
    print "owlrepository [-h] [-d] [-v] [-p n]"
    print "-h: print this help"
    print "-v: show version"
    print "-d: debug flag"
    print "-p port: start the server on a specified port"
    
def version():
    print "Veggente project: Conan OWL repository v. "+__version__

if __name__=="__main__":
    try:
        opts, args=getopt.getopt(sys.argv[1:],"hp:vd",['help','port=','version','debug'])
    except getopt.GetoptError:
        usage()
        sys.exit(-1)
    soap_port=10000
    debug=False
    for opt, val in opts:
        if opt in ('-h','--help'):
            usage()
            sys.exit(0)
        if opt in ('-v','--version'):
            version()
            sys.exit(0)
        if opt in ('-d','--debug'):
            debug=True
        if opt in ('-p','--port'):
            soap_port=val
        
    print "Veggente project: Conan OWL repository"
    print "Starting repository"
    repository=OWLRepository('conan')
    repository.debug_flag=debug
    print "Starting SOAP interface on port "+str(soap_port)
    SOAPpy.Config.simplify_objects=1
    soap_server=SOAPpy.SOAPServer(('localhost',soap_port))
    soap_server.registerObject(repository)
    for i in  repository.list_documents():
        print i
    soap_server.serve_forever()
