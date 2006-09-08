#!/usr/bin/python

#	Veggente - owlrepository
#	Conan OWL repository
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
from repository import Repository
from swap.stringwriter import StringWriter
import swap.cwm
import RDF
import SOAPpy
import getopt,sys
import datetime

__version__='0.0.1'

class OWLRepository(Repository):
    """
    OWL-enabled RDF repository
    """
    
    debug_flag=True
    inf_prefix='think_'
    
    # These two variables have to be defined in some config file!!!
    owl_classes='http://veggente.berlios.de/think/owl.n3'
#    rdfs_classes='http://veggente.berlios.de/think/rdfs.n3'
#    xsd_classes='http://veggente.berlios.de/think/xsd.n3'
    owl_rules='http://veggente.berlios.de/think/owl_complete.n3'
    instance_rules='http://veggente.berlios.de/think/instance_rules.n3'

    # Namespaces
    rdf_ns='http://www.w3.org/1999/02/22-rdf-syntax-ns#'
    rdfs_ns='http://www.w3.org/2000/01/rdf-schema#'
    owl_ns='http://www.w3.org/2002/07/owl#'

    accepted_types=[owl_ns+'Class',owl_ns+'ObjectProperty',owl_ns+'DatatypeProperty']

    def add_ontology(self, uri, overwrite=False):
        """
        uri:string : ontology uri
        @overwrite:boolean   : if True overwrite previously stored ontology
        Returns:
            0 on success
            -1 on failure
        """
        if (uri is None) or (uri==''):
            return -1
        if (not self.isPresent(uri)):          
            print "Document %s is NOT in store, downloading..."%uri
            self.add_document(uri)
        else:
            print "Document %s is in store"%uri
            inf_onto=self.isPresent(self.inf_prefix+uri)
            if (overwrite==True):
                self.remove_document(uri)
                if (inf_onto==True):
                    self.remove_document(inf_onto)
            else:
                if inf_onto==True:
                    print "Inference over %s is in store"%uri
                    return 0
        import_list=self.find_imports(uri)
        for doc in import_list:
            self.add_ontology(doc,overwrite)
        inferred_statements=self.exec_ontology_inference(uri,import_list,overwrite)
        if inferred_statements is None:
            return -1
        for s in inferred_statements:
            self.model.add_statement(s,RDF.Node(RDF.Uri(self.inf_prefix+uri)))
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
        if (self.remove_document(uri)==0 and self.remove_document(self.inf_prefix+uri)==0):
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
            
        
    def add_instance_document(self, uri, ontologies=[], longterm=False, overwrite=True):
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
        if (longterm==True):
            working_model=self.model
            self.add_document(uri)
        else:
            working_model=self.mem_model
            self.add_inmem_document(uri)
        for o in ontologies:
           self.add_ontology(o,False)
        inferred_statements=self.exec_instance_inference(uri, ontologies, overwrite)
        if inferred_statements is None:
            return -1
        for s in inferred_statements:
            working_model.add_statement(s,RDF.Node(RDF.Uri(self.inf_prefix+uri)))
        return 0

    def remove_instance_document(self,uri,longterm=False):
        """
        Removes an instance document and its inferred triples
        uri:string : document uri
        Returns:
            0 on success
            -1 on failure
        """
        if (uri is None) or (uri==''):
            return -1
        if (longterm==True):
            if (self.remove_document(uri)==0 and self.remove_document(self.inf_prefix+uri)==0):
                return 0
            else:
                return -1
        else:
            if (self.remove_inmem_document(uri)==0 and self.remove_inmem_document(self.inf_prefix+uri)==0):
                return 0
            else:
                return -1

    def exec_ontology_inference(self, original_uri, uri_list=[], overwrite=False):
        """
        Explode an ontology 
        uri:string : ontology uri
        uri_list[]:string[] : list of import uri
        @overwrite:boolean   : if True overwrite previously inferred data
        Returns:
            list of inferred statements about selected ontology
        """
        if (original_uri is None) or (original_uri==''):
            return None
        cmd=[]
        output=StringWriter()
        # I know it this UGLY, but if it works...
        cmd.append('cwm')
        cmd.append('--rdf')
        cmd.append(self.__find_location(original_uri))
        for i in uri_list:
            cmd.append(self.__find_location(i))
            if self.isPresent(self.inf_prefix+i):
                cmd.append(self.__find_location(self.inf_prefix+i))
        cmd.append('--n3')
        cmd.append(self.owl_classes)
#        cmd.append(self.xsd_classes)
#        cmd.append(self.rdfs_classes)
        cmd.append('--filter='+self.owl_rules)
        cmd.append('--think')
        cmd.append('--ugly')
        cmd.append('--purge') 
        cmd.append('--rdf')
        print "Executing inference on document %s"%original_uri
        swap.cwm.doCommand(cmd,output)
        cwm_out=output.getContent()
        if (cwm_out is None) or (cwm_out==[]):
            return None
        return self.parser.parse_string_as_stream(cwm_out,original_uri)

    def exec_instance_inference(self, original_uri, uri_list=[], overwrite=False):
        """
        uri:string : ontology uri
        uri_list[]:string[] : list of import uri
        @overwrite:boolean   : if True overwrite previously inferred data
        Returns:
            list of inferred statements about selected instance document
        """
        if (original_uri is None) or (original_uri==''):
            return None
        cmd=[]
        output=StringWriter()
        # I know it this UGLY, but if it works...
        cmd.append('cwm')
        cmd.append('--rdf')
        cmd.append(self.__find_location(original_uri))
        for i in uri_list:
            cmd.append(self.__find_location(i))
            if self.isPresent(self.inf_prefix+i):
                cmd.append(self.__find_location(self.inf_prefix+i))
        cmd.append('--n3')
        cmd.append(self.owl_classes)
        cmd.append(self.xsd_classes)
        cmd.append(self.rdfs_classes)
        cmd.append('--filter='+self.instance_rules)
        cmd.append('--think')
        cmd.append('--ugly')
        cmd.append('--purge') 
        cmd.append('--rdf')
        print "Executing inference on document %s"%original_uri
        swap.cwm.doCommand(cmd,output)
        cwm_out=output.getContent()
        if (cwm_out is None) or (cwm_out==[]):
            return None
        return self.parser.parse_string_as_stream(cwm_out,original_uri)
    

    # Query functions
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
        global_imports=self.model.find_statements(st,RDF.Node(RDF.Uri(uri)))
        if global_imports is None:
            return None
        doc_imports=[]
        for m in global_imports:
            doc_imports.append(str((m.object).uri))
        return doc_imports

    def find_imports_cluster(self,ontology):
        cluster=[]
        for i in self.find_imports(ontology):
            if not (i in cluster):
                cluster.append(i)
        if cluster!=None:
            for imp in cluster:
                for new_imp in self.find_imports_cluster(imp):
                    cluster.append(new_imp)
        return cluster

    def get_ontology(self, uri):
        if (uri is None) or (uri==''):
            return -1
        temp_model=RDF.Model()
        print 'copying uri '+uri
        for st in self.model.find_statements(RDF.Statement(subject=None,predicate=None,object=None),RDF.Node(RDF.Uri(uri))):
            temp_model.add_statement(st)
        for ist in self.model.find_statements(RDF.Statement(subject=None,predicate=None,object=None),RDF.Node(RDF.Uri('think_'+uri))):
            temp_model.add_statement(ist)
        print "Copied "+str(temp_model.size())+' statements'
        return temp_model.to_string(base_uri=uri)

    def get_onto_name(self,resource,ontology):
        """
        Find a class or a property with a given name in an ontology or in its imports
        resource: string to search
        ontology: string uri of an ontology entry point
        returns: 
            string uri of the resource
        """
        if (resource is None) or (ontology is None) or (resource=='') or (ontology==''):
            return None
        if ontology.endswith('#'):
            ontology=ontology.split('#')[0]
        results=self.model.find_statements(RDF.Statement(subject=RDF.Uri(ontology+'#'+resource),predicate=RDF.Uri(self.rdf_ns+'type')),RDF.Node(RDF.Uri(ontology)))
        if results!=[]:
            for i in results:
                if i.subject.is_resource():
                    return str(i.subject.uri)
        for imp in self.find_imports(ontology):
            return self.get_onto_name(resource,imp)
        
    def get_property_range(self,resource,ontology):
        """
        Return a list of a property's range
        """
        res_list=[]
        results=self.model.find_statements(RDF.Statement(subject=RDF.Uri(resource),predicate=RDF.Uri(self.rdfs_ns+'range')),RDF.Node(RDF.Uri(ontology)))
        for i in results:
            res_list.add(str(i.object.uri))
        return res_list

    def get_type(self,uri):
        """
        Return a list of types associated with a resource URI
        uri: string of the resource 
        """
        res_list=[]
        ns=uri.split('#')[0]
        res=uri.split('#')[1]
        results=self.model.find_statements(RDF.Statement(subject=RDF.Uri(uri),
                                                        predicate=RDF.Uri(self.rdf_ns+'type')),
                                                        RDF.Node(RDF.Uri(ns))
                                                        )
        for i in results:
            if i.object.is_resource():
                res_list.append(str(i.object.uri))
        if res_list!=[]:
            return res_list
        results=self.model.find_statements(RDF.Statement(subject=RDF.Uri(uri),
                                                        predicate=RDF.Uri(self.rdf_ns+'type')),
                                                        RDF.Node(RDF.Uri('think_'+ns))
                                                        )
        for i in results:
            if i.object.is_resource():
                res_list.append(str(i.object.uri))
        return res_list

    def onto_identify(self,resource,ontology):
        res_name=None
        res_type=None
        ontology=ontology.split('#')[0]
        
        results=self.model.find_statements(RDF.Statement(subject=RDF.Uri(ontology+'#'+resource),predicate=RDF.Uri(self.rdf_ns+'type')),RDF.Node(RDF.Uri(ontology)))
        for i in results:
            if str(i.object.uri) in self.accepted_types:
                res_name=ontology+'#'+resource
                res_type=str(i.object.uri)
                break
        if res_name!=None:
            return res_name, res_type
        for imp in self.find_imports(ontology):
            return self.onto_identify(resource,imp)
        return None, None

    def get_class_properties(self,class_name,ontology):
        prop_list=[]
        imports=self.find_imports(ontology)
        for (st,context) in self.model.find_statements_context(RDF.Statement(predicate=RDF.Uri(self.rdfs_ns+'domain'),object=RDF.Uri(class_name))):
            if (context.uri in imports) or (context.uri==ontology):
                prop_list.add(st.uri)
        return prop_list


def usage():
    print "Veggente project: Conan OWL repository"
    print "owlrepository [-h] [-d] [-v] [-p n]"
    print "-h: print this help"
    print "-v: show version"
    print "-d: debug flag"
    print "--port n: start the server on a specified port"
    print "--db dir: use the specified dir for db backend"
    
def version():
    print "Veggente project: Conan OWL repository v. "+__version__

soap_server=None
repository=None
try:
    if __name__=="__main__":
        db_dir='.'
        db_type='sqlite'
        db_host='localhost'
        db_port='3306'
        db_user=None
        db_pass=None

        try:
            opts, args=getopt.getopt(sys.argv[1:],"hp:vd",['help','port=','db=','db_type=','version','debug','db_user=','db_pass=','db_host=','db_port='])
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
            if opt in ('--db'):
                db_dir=val
            if opt in ('--db_port'):
                db_port=val
            if opt in ('--db_host'):
                db_host=val
            if opt in ('--db_pass'):
                db_pass=val
            if opt in ('--db_user'):
                db_user=val
            if opt in ('--db_type'):
                db_type=val
            if opt in ('-p','--port'):
                soap_port=val
            
        print "Veggente project: Conan OWL repository"
        print "Starting repository"
        if db_type=='sqlite':
            repository=OWLRepository(dbname='conan',db=db_dir,database_type='sqlite')
        elif db_type=='mysql':
            repository=OWLRepository(dbname='conan',database_type='mysql',host=db_host,port=db_port,username=db_user,password=db_pass)
        repository.debug_flag=debug
        print "SOAP interface on port "+str(soap_port)
        SOAPpy.Config.simplify_objects=1
        soap_server=SOAPpy.ThreadingSOAPServer(('localhost',soap_port))
        soap_server.registerObject(repository)
        soap_server.serve_forever()
except KeyboardInterrupt:
    print "OWL server shutdown"
    soap_server.server_close()
    del repository
    print "Data saved"
    sys.exit(0)
