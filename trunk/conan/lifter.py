#!/usr/bin/python

#	Veggente - lifter
#   Implementation of a XML to RDF algorithm
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

import getopt
import SOAPpy
import RDF
from os import sys
import xml.dom.minidom
from xml.dom.minidom import Node

class Lifter:
    """
    Lifter class' rdf graph contains two contexts:
        instance: identify triples coming from the lifting process
        ontology: identify triples coming from the ontology cluster
    """
    server=''
    root_node=None
    repository=None
    rdf_model=None
    parser=None
    rdf_ns='http://www.w3.org/1999/02/22-rdf-syntax-ns#'
    rdfs_ns='http://www.w3.org/2000/01/rdf-schema#'
    owl_ns='http://www.w3.org/2002/07/owl#'
    base_onto=''
    uml_style=True
    unfinished_statements=[]
    counter=0

    def __init__(self,server_url='http://localhost:10000'):
        if (server_url is None) or (server_url==''):
            raise "Server URL must not be empty"
        self.server=server_url
        SOAPpy.Config.simplify_objects=1
        self.repository=SOAPpy.SOAPProxy(self.server)
        self.parser=RDF.Parser(name='rdfxml')
        if self.parser is None:
            raise "Failed initializing RDF parser"
        doc_store=RDF.Storage(storage_name="hashes",name="memstore",options_string="hash-type='memory'")
        if doc_store is None:
            raise "Failend creating in memory storage"
        self.rdf_model=RDF.Model(doc_store)
        if self.rdf_model is None:
            raise "Failed creating in memory RDF model"
        onto_store=RDF.Storage(storage_name="hashes",name="onto_store",options_string="hash-type='memory'")
        if onto_store is None:
            raise "Failend creating in memory storage"
        self.onto_model=RDF.Model(onto_store)
        if self.rdf_model is None:
            raise "Failed creating in memory RDF model"
 
    def lift(self,document,onto_list,uml_mode=True):
        """
        Convert the content of an XML document into corresponding RDF triples
        document: uri of the document to convert
        onto_list: list of ontologies intantiated by the document
        """
        if (onto_list is None) or (document is None):
            return None
        self.uml_style=uml_mode
        self.base_onto=onto_list
#        if (self.get_onto_cluster(onto_list)!=0):
#            print "Error: cannot retrieve ontologies from repository"
#            return -1
        # Parse XML Document
        doc = xml.dom.minidom.parse(document)
        self.doc_context=RDF.Node(document)
        self.root_node=doc.documentElement
        self.removeWhitespaceNodes(self.root_node)
        self.walk(self.root_node,None,RDF.Node(RDF.Uri(document)))
        doc.unlink()
    
    def walk(self,node,active_class,active_res):
        """
        Traverse XML node and build RDF triples
        node: xml Node
        active_res: RDF.Node
        """
        # Extract node info
        node_value=node.nodeValue
        if node.nodeType==Node.TEXT_NODE:
            tmp_res=self.handle_text(active_res,node_value)
            if tmp_res!=None:
                active_res=tmp_res
        elif (node.nodeType==Node.ELEMENT_NODE):
            tmp_class,tmp_res=self.handle_node(active_class,active_res,str(node.nodeName))
            if tmp_res!=None:
                active_res=tmp_res
            if tmp_class!=None:
                active_class=tmp_class
        for child in node.childNodes:
            self.walk(child,active_class,active_res)
            attrs = node.attributes
            for attrName in attrs.keys():
                # Dirty hack: filter out xsi:schemaLocation attributes
                if (not attrName.startswith('xmlns'))and(not attrName=='xsi:schemaLocation'):
                    attrNode = attrs.get(attrName)
                    node_name=attrNode.localName
                    attrValue = attrNode.nodeValue
                    active_class,tmp_res=self.handle_node(active_class,active_res,node_name)
                    if tmp_res!=None:
                        active_res=tmp_res
                        tmp_res=self.handle_text(active_res,attrValue)
                        if tmp_res!=None:
                            active_res=tmp_res

    def handle_text(self,active_res,node_value):
        if (self.unfinished_statements is None) or (self.unfinished_statements==[]):
            print "<Error>No unfinished statements: PANIC!!!!</Error>"
            self.rdf_model.add_statement(RDF.Statement(active_res,
                                                        RDF.Node(RDF.Uri('http://error.it#PanicError')),
                                                        RDF.Node(node_value)
                                                        ))
        else:
            for m in self.unfinished_statements:
                if (m.object is None) and (m.predicate!=None) and (m.subject!=None):
                    m.object=RDF.Node(node_value)
                    self.rdf_model.add_statement(m)
                    self.unfinished_statements.remove(m)
                    break
        return active_res

    def resolve_node(self,node,active_class):
        # Begin searching with UML style notation 
        xml_res_name=self.xml_to_internal(str(node.nodeName),active_class)
        node_onto_name,node_onto_type=self.repository.onto_identify(xml_res_name,self.base_onto)
        if (node_onto_name=='') or (node_onto_name is None) or (node_onto_type=="") or (node_onto_type is None):
            if (active_class!=None):
                #Retry with non UML notation 
                return self.resolve_node(node,None)
            else:
                return None,None
        else:
            return node_onto_name,node_onto_type

    def new_handle_element(self,subject_class,subject_node,xml_node):
        

        # Check if there is an explicit instantiation
#        attrs=node.attributes
#        for attrName in attrs.keys():
#            if "xsi:type" in attrName:
#                
#
#            if (not attrName.startswith('xmlns'))and(not attrName=='xsi:schemaLocation'):
#                attrNode = attrs.get(attrName)
#                node_name=attrNode.localName
#                attrValue = attrNode.nodeValue
#                if (attrName=="xsi:type"):
#                    # Explicit instantiation via xsi:type
#
#                else:
#
#
#
#        rdf_name,rdf_type=self.resolve_node(node,active_class)
#        if (rdf_name is None) or (rdf_type is None):
#            print '<Resource>\n  <name>'+node.nodeName+'</name>'
#            print "  <Error/>\n</Resource>"
    


    def handle_node(self,active_class,active_res,node_name):
        xml_res_name=self.xml_to_internal(node_name,active_class)
        node_onto_name,node_onto_type=self.repository.onto_identify(xml_res_name,self.base_onto)
        if (node_onto_name=='') or (node_onto_name is None):
            xml_res_name=self.xml_to_internal(node_name,None)
            node_onto_name,node_onto_type=self.repository.onto_identify(xml_res_name,self.base_onto)
            if (node_onto_name=='') or (node_onto_name is None):
                print '<Resource>\n  <name>'+xml_res_name+'</name>'
                print "  <Error/>\n</Resource>"
                return active_class,None

        print '<Resource>\n  <name>'+xml_res_name+'</name>'
        print '  <onto_res>'+node_onto_name+'</onto_res>\n  <type>'+node_onto_type+'</type>'
        print '</Resource>'
        
        # Properties
        if (node_onto_type==self.owl_ns+'ObjectProperty') or (node_onto_type==self.owl_ns+'DatatypeProperty'):
            if (self.unfinished_statements is None) or (len(self.unfinished_statements)==0):
                self.unfinished_statements.append(RDF.Statement(active_res,RDF.Node(uri_string=node_onto_name)))
            else:
                for m in self.unfinished_statements:
                    if (m.object is None) and (m.predicate!=None) and (m.subject!=None):
                        m.object=RDF.Node(blank='pr'+str(self.counter))
                        self.rdf_model.add_statement(m)
                        self.unfinished_statements.append(RDF.Statement(m.object,RDF.Node(RDF.Uri(node_onto_name))))
                        self.counter=self.counter+1
                        active_res=m.object
                        self.unfinished_statements.remove(m)
                        break
        
        # Class
        elif node_onto_type==self.owl_ns+'Class':
            active_class=node_onto_name.split('#')[1]
            # Document is the active resource
#            if active_res==self.root_node:
#                print "in Handle class: added root statement"
#                self.rdf_model.add_statement(RDF.Statement(active_res,
#                                                            RDF.Node(RDF.Uri(self.rdf_ns+'type')),
#                                                            RDF.Node(RDF.Uri(node_onto_name))
#                                                            ))
#            else:
            if (self.unfinished_statements is None) or (len(self.unfinished_statements)==0):
                self.rdf_model.add_statement(RDF.Statement(active_res,
                                                            RDF.Node(RDF.Uri(self.rdf_ns+'type')),
                                                            RDF.Node(RDF.Uri(node_onto_name))
                                                            ))
            else:
                for m in self.unfinished_statements:
                    print m
                    if (m.object is None) and (m.predicate!=None) and (m.subject!=None):
                        m.object=RDF.Node(blank='cls'+str(self.counter))
                        self.rdf_model.add_statement(m)
                        self.rdf_model.add_statement(RDF.Statement(m.object,
                                                                    RDF.Node(RDF.Uri(self.rdf_ns+'type')),
                                                                    RDF.Node(RDF.Uri(node_onto_name))
                                                                    ))
                        self.counter=self.counter+1
                        active_res=m.object
                        self.unfinished_statements.remove(m)
                        break
        return active_class,active_res

    def xml_to_internal(self,xmltag,base_class):
        """
        Returns the name to use while searching into ontology resources
        xmltag: Node
        """
        if (xmltag is None):
            return ''
        if (self.uml_style==False) or (base_class is None) or (base_class==''):
            return xmltag
        else:
            return base_class+'.'+xmltag

    def removeWhitespaceNodes(self,parent):
        for child in list(parent.childNodes):
            if child.nodeType==Node.TEXT_NODE and child.data.strip()=='':
                parent.removeChild(child)
            else:
                self.removeWhitespaceNodes(child)

    def onto_identify(self,resource,ontology):
        res_name=None
        res_type=None
        ontology=ontology.split('#')[0]
        
        results=self.onto_model.find_statements(RDF.Statement(subject=RDF.Uri(ontology+'#'+resource),predicate=RDF.Uri(self.rdf_ns+'type')))
        for i in results:
            res_name=ontology+'#'+resource
            res_type=str(i.object.uri)
            break
        if res_name!=None:
            return res_name, res_type
        for imp in self.repository.find_imports(ontology):
            return self.onto_identify(resource,imp)
        return None, None
    
    def get_onto_cluster(self, ontology_list):
        print ontology_list
        onto_list=[]
        if isinstance(ontology_list,list):
            onto_list=ontology_list
        elif isinstance(ontology_list,str):
            onto_list.append(ontology_list)
        print onto_list
        for i in onto_list:
            onto=self.repository.get_ontology(i)
            for st in self.parser.parse_string_as_stream(onto,onto):
                self.onto_model.add_statement(st)
            imports=self.repository.find_imports(i)
            if imports!=[]:
                self.get_onto_cluster(imports)
        return 0
   
    def add_class_instance(self,subject,class_res):
        """
        Add a class instance to the store
        Parameters:
            subject (RDF.Node): current RDF subject
            class_res (string): Class resource
        Returns:
            RDF.Node
        """
        if (subject is None) or (class_res is None):
            return None
        self.rdf_model.add_statement(RDF.Statement(subject,
                                                    RDF.Node(RDF.Uri(self.rdf_ns+'type')),
                                                    RDF.Node(RDF.Uri(class_res))
                                                    ))
        return subject
    
    def add_obj_property_instance(self,subject,property_res,dest_class):
        """
        Add an ObjectProperty instance to the store
        Parameters:
            subject (RDF.Node): current RDF subject
            property_res (string): ObjectProperty resource
            dest_class (string): destination class of the ObjectProperty
        Returns:
            RDF.Node
        """
        if (subject is None) or (property_res is None) or (dest_class is None):
            return None
        new_subject=RDF.Node(blank='cls'+str(self.counter))
        self.rdf_model.add_statement(RDF.Statement(subject,
                                                    RDF.Node(RDF.Uri(property_res)),
                                                    new_subject
                                                    ))
        return self.add_class_instance(new_subject,dest_class)

    def add_data_property_instance(self,subject,property_res,textual_content):
        """
        Add a DatatypeProperty instance to the store
        Parameters:
            subject (RDF.Node): current RDF subject
            property_res (string): DatatypeProperty resource
            textual_content (string): value of the DatatypeProperty
        Returns:
            RDF.Node
        """
        if (subject is None) or (property_res is None) or (textual_content is None):
            return None
        self.rdf_model.add_statement(RDF.Statement(subject,
                                                    RDF.Node(RDF.Uri(property_res)),
                                                    RDF.Node(literal=textual_content)
                                                    ))
        return subject
  
lift_inst=Lifter()
try:
    if __name__=='__main__':
        print '<?xml version="1.0"?>'
        print '<source_file name="'+sys.argv[1]+'"'
        print 'ontology_name="'+sys.argv[2]+'">'
        lift_inst.lift(sys.argv[1],sys.argv[2])
        print '</source_file>'
except KeyboardInterrupt:
    print "------- "+str(len(lift_inst.unfinished_statements))+" Unfinished Statements ---------"
    for st in lift_inst.unfinished_statements:
        print st
# Testing. Remove when completed
serializer=RDF.Serializer()
serializer.set_namespace("doc", RDF.Uri(sys.argv[2]+"#"))
serializer.set_namespace("rim", RDF.Uri('http://veggente.berlios.de/ns/RIMOntology#'))
serializer.set_namespace("rim_dt", RDF.Uri('http://veggente.berlios.de/ns/RIMDatatype#'))
serializer.serialize_model_to_file("test-out.rdf", lift_inst.rdf_model)
