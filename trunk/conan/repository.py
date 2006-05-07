#!/usr/bin/python

from os import sys
import RDF

# Standard RDF repository
class Repository(object):
        """
        Standard RDF repository
        """
        #public:
        db_dir='' # char*
        parser=None
        model=None
        storage=None
        
        #constructor
        def __init__(self,db='.'):
                self.db_dir=db
                self.parser=RDF.Parser('raptor')
                if self.parser is None:
                        raise "Failed initializing RDF parser"
                if (self.__init_db()==0):
                        print 'RDF repository initialized'
                        
        def __del__(self):
                self.model.sync()

        #public:
        def add_document(self, uri,  context=None):
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
                        if (self.model.remove_statements_with_context(RDF.Uri(context))!=0):
                                print "Error removing URI "+context
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
                        print i
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
                                name="conan",
                                options_string=storage_options)
                if self.storage is None:
                        raise "Failed creating storage"
                self.model=RDF.Model(self.storage)
                if self.model is None:
                        raise "Failed creating RDF model"
                return 0

if __name__=="__main__":
        print "Veggente project: Conan RDF repository"
        repository=Repository('.')
        repository.add_document('http://veggente.berlios.de/ns/RIMOntology')
        repository.add_document('file:///tmp/foaf.rdf')
        repository.check_documents()
