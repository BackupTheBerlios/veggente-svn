#!/usr/bin/python

from os import sys
import RDF
import SOAPpy

# Standard RDF repository
class Repository(object):
    """
    Standard RDF repository
    """
    #public:
    db_dir='' # char*
    db_name='conan'
    parser=None
    model=None
    storage=None
    db_uri=''
    
    #constructor
    def __init__(self,dbname,db='.'):
        self.db_dir=db
        if (dbname!=None) and (dbname!=''):
            self.db_name=dbname
        self.parser=RDF.Parser('raptor')
        if self.parser is None:
                raise "Failed initializing RDF parser"
        if (self.__init_db()==0):
                print 'RDF repository initialized'
        self.db_uri=self.db_name+'://'+self.db_dir+'+'
                    
    def __del__(self):
        self.model.sync()

    #public:
    def add_document(self, uri,  context=None, overwrite=False):
        """
        @uri:char*
        @context:char*
        """
        current_uri=None
        context_node=None
        if uri is None:
                return -1
        current_uri=RDF.Uri(uri)
        if context is None:
                context_node=RDF.Node(RDF.Uri(string=uri))
        else:
                context_node=RDF.Node(RDF.Uri(context))
        if overwrite and self.remove_statements_with_context(uri)!=0:
                return -1
        for s in self.parser.parse_as_stream(current_uri,current_uri):
                self.model.add_statement(s,context_node)
        self.model.sync()
        print "Adding URI "+uri
        
        return 0
    
    def remove_document(self, context): 
        """
        @context:char*
        """
        if (context!=''):
            return self.model.remove_statements_with_context(RDF.Uri(context))
        return None
    
    def query_model(self, query_str):
        """
        @query_str:char*
        @context:char*
        """
        if query_str!=None:
            q=RDF.SPARQLQuery(query_str)
            return q.execute(self.model)
        return None
    
    def check_documents(self):
        """
        Lists documents in store
        """
        print "Stored URI list:"
        for i in self.model.get_contexts():
            print i.uri
        return None

    def check_class(self, uri):
        """
        @uri:char*
        """
        return None
    
    def check_property(self, uri): 
        """
        @uri:char*
        """
        return None
    
    def __init_db(self):
        storage_options="hash-type='bdb',contexts='yes',dir='"+self.db_dir+"'"
        self.storage=RDF.Storage(storage_name="hashes",
                        name=self.db_name,
                        options_string=storage_options)
        if self.storage is None:
                raise "Failed creating storage"
        self.model=RDF.Model(self.storage)
        if self.model is None:
                raise "Failed creating RDF model"
        return 0

if __name__=="__main__":
        print "Veggente project: Conan RDF repository"
        repository=Repository('conan','.')
        soap_port=10000
        SOAPpy.Config.simplify_objects=1
        soap_server=SOAPpy.SOAPServer(('localhost',soap_port))
        soap_server.registerObject(repository)
        repository.check_documents()
        soap_server.serve_forever()