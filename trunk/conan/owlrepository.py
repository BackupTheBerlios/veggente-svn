#!/usr/bin/python
# OWL-enabled RDF repository

from os import sys
from repository import Repository
import swap.cwm
import RDF

class OWLRepository(Repository):
    """
    OWL-enabled RDF repository
    """
    
    # These two variables have to be defined in some config file!!!
    owl_classes='http://veggente.berlios.de/think/owl.n3'
    owl_rules='http://veggente.berlios.de/think/owl_rules.n3'
    def add_document(self, uri, think=True, context=None, overwrite=True):
        Repository.add_document(self,uri,context)
        
    def add_document(self, uri, think=True, context=None, overwrite=True):
        Repository.add_document(self,uri,context)
        if think==True:
            self.exec_inference(uri,'think+'+uri)
            
    def exec_ontology_inference(self, uri_list,  results_uri):
        """
        @context_uri:char*
        @results_uri:char* :Where to store inferred statements
        """
        target=self.find_deps()
        cmd=[]
        # I know it this UGLY, but if it works...
        cmd.append('cwm')
        cmd.append('--rdf')
        for i in uri_list:
            cmd.append(uri_list)
        cmd.append(' --n3 ')
        cmd.append(self.owl_classes)
        cmd.append('--filter='+self.owl_rules)
        cmd.append('--think')
        cmd.append('--ugly')
        cmd.append('--purge') 
        cmd.append('--rdf')
        cwm_out=cwm.doCommand(cmd)
        if cwm_out!=None:
            
        return None
    
    def __find_location(self,uri):
        """
        Returns the original uri if the document is not in store, 
        or the Redland-VFS uri if already stored
        """
        for i in self.model.get_contexts():
            if i.uri==uri:
                # URI is in store
                return self.db_uri+uri
        return uri
    
    def is_allowed(self, property_uri,  class_uri):
        """
        @property_uri:char*
        @class_uri:char*
        """
        return None


if __name__=="__main__":
    print "Veggente project: Conan OWL repository"
    repository=OWLRepository('.')
    return 0

