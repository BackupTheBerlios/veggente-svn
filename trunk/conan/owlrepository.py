#!/usr/bin/python
# OWL-enabled RDF repository

from os import sys
from repository import Repository
import RDF

class OWLRepository(Repository):
        """
        OWL-enabled RDF repository
        """
        def __init__(self,db):
                Repository.__init__(self)
        
        #public:
        def add_document(self, uri, context=None):
                Repository.add_document(self,uri,context)
        
        def exec_inference(self, context_uri,  results_uri):
                """
                @context_uri:char*
                @results_uri:char* :Where to store inferred statements
                """
                return None
        
        def is_allowed(self, property_uri,  class_uri):
                """
                @property_uri:char*
                @class_uri:char*
                """
                return None


if __name__=="__main__":
        print "Veggente project: Conan OWL repository"
        repository=OWLRepository('.')
        sys.exit(0)

