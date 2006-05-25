#!/usr/bin/python
 
#	Veggente - repository
#	Conan RDF repository
#	
#	Copyright(c) 2006 Alessio Carenini <carenini@gmail.com>
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

from os import sys
import RDF
import SOAPpy

"""
	Veggente - repository
	Conan RDF repository
"""
class Repository(object):
    """
    Standard RDF repository
    """
    #public:
    db_dir=''
    db_name='conan'
    parser=None
    model=None
    storage=None
    mem_storage=None
    db_uri=''
    
    #constructor
    def __init__(self,dbname,db='.'):
        self.db_dir=db
        if (dbname!=None) and (dbname!=''):
            self.db_name=dbname
        self.parser=RDF.Parser(name='rdfxml')
        if self.parser is None:
                raise "Failed initializing RDF parser"
        if (self.__init_db()==0):
                print 'RDF repository initialized in '+db
        if (self.__init_memstore()==0):
                print 'RDF in-memory storage initialized'
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
        if (overwrite==False) and (self.isPresent(uri)):
            return 0
        if overwrite:
            if self.remove_statements_with_context(uri)!=0:
                return -1
        for s in self.parser.parse_as_stream(current_uri,current_uri):
                self.model.add_statement(s,context_node)
        self.model.sync()
        print "Adding URI "+uri
        
        return 0
    
    def add_inmem_document(self, uri,  context=None, overwrite=False):
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
        if (overwrite==False) and (self.isPresent(uri)):
            return 0
        if overwrite:
            if self.mem_model.remove_statements_with_context(uri)!=0:
                return -1
        for s in self.parser.parse_as_stream(current_uri,current_uri):
                self.mem_model.add_statement(s,context_node)
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

    def remove_inmem_document(self, context): 
        """
        @context:char*
        """
        if (context!=''):
            return self.mem_model.remove_statements_with_context(RDF.Uri(context))
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
    
    def list_documents(self):
        """
        Lists documents in store
        """
        doc_list=[]
        for i in self.model.get_contexts():
            doc_list.append(str(i))
        return doc_list

    def isPresent(self,uri):
        """
        Returns True if uri is present in context list
        """
        if (uri is None) or (uri==''):
            return False
        for i in self.model.get_contexts():
            if str(i.uri)==uri:
                return True
        return False

    def __init_db(self):
        storage_options="contexts='yes',dir='"+self.db_dir+"'"
        self.storage=RDF.Storage(storage_name="sqlite",
                        name=self.db_name,
                        options_string=storage_options)
        if self.storage is None:
                raise "Failed creating storage"
        self.model=RDF.Model(self.storage)
        if self.model is None:
                raise "Failed creating RDF model"
        return 0

    def __init_memstore(self):
        self.mem_storage=RDF.Storage(storage_name="hashes",name="memstore",options_string="contexts='yes',hash-type='memory'")
        if self.mem_storage is None:
            raise "Failed creating in memory storage"
        self.mem_model=RDF.Model(self.mem_storage)
        if self.mem_model is None:
            raise "Failed creating in memory RDF model"
        return 0

soap_server=None
repository=None
try:
    if __name__=="__main__":
            print "Veggente project: Conan RDF repository"
            repository=Repository('conan','.')
            soap_port=10000
            SOAPpy.Config.simplify_objects=1
            soap_server=SOAPpy.SOAPServer(('localhost',soap_port))
            soap_server.registerObject(repository)
            repository.check_documents()
            soap_server.serve_forever()
except KeyboardInterrupt:
    print "RDF server shutdown"
    soap_server.server_close()
    del repository
    print "Data saved"
    sys.exit(0)
