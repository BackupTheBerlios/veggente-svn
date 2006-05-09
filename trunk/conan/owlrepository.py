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

    def add_ontology(self, uri, think=True, overwrite=True):
        """
        @uri:string : ontology uri
        @overwrite:boolean   : if True overwrite previously stored ontology
        Returns:
            0 on success
            1 on failure
        """
        if (uri is None) or (uri==''):
            return 1
        Repository.add_document(uri)
        import_list=self.find_imports(uri)
        for doc in import_list:
            Repository.add_document(doc)
        if think==True:
            inferred_statements=self.exec_inference(uri,import_list,overwrite)
            if inferred_statements is None:
                return 1
            for s in inferred_statements:
                self.model.add_statement(s,RDF.Node(RDF.Uri('think+'uri)))
        return 0
            
    def exec_ontology_inference(self, original_uri, uri_list=[], overwrite=False):
        """
        @uri:string : ontology uri
        @uri_list[]:string[] : list of import uri
        @overwrite:boolean   : if True overwrite previously inferred data
        Returns:
            list of statements
        """
        if (uri is None) or (uri==''):
            return None
        cmd=[]
        # I know it this UGLY, but if it works...
        cmd.append('cwm')
        cmd.append('--rdf')
        cwm.append(uri)
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
        if (cwm_out is None) or (cwm_out==[]):
            return None
        return self.parser.parse_string_as_stream(cwm_out,uri)
    
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
    
    def find_imports(self,uri):
        query=SPARQLQuery('select ?obj ')
    
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

