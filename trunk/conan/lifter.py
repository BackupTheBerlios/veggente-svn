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
    server=''
    root_node=None
    repository=None
    rdf_model=None
    rdf_ns='http://www.w3.org/1999/02/22-rdf-syntax-ns#'
    rdfs_ns='http://www.w3.org/2000/01/rdf-schema#'
    owl_ns='http://www.w3.org/2002/07/owl#'
    base_onto=''
    unfinished_statements=[]
    counter=0

    def __init__(self,server_url='http://localhost:10000'):
        if (server_url is None) or (server_url==''):
            raise "Server URL must not be empty"
        self.server=server_url
        SOAPpy.Config.simplify_objects=1
        self.repository=SOAPpy.SOAPProxy(self.server)
        doc_store=RDF.Storage(storage_name="hashes",name="memstore",options_string="hash-type='memory'")
        if doc_store is None:
            raise "Failend creating in memory storage"
        self.rdf_model=RDF.Model(doc_store)
        if self.rdf_model is None:
            raise "Failed creating in memory RDF model"
 
    def lift(self,document,onto_list):
        """
        Convert the content of an XML document into corresponding RDF triples
        document: uri of the document to convert
        onto_list: list of ontologies intantiated by the document
        """
        self.base_onto=onto_list
        doc = xml.dom.minidom.parse(document)
        self.doc_context=RDF.Node(document)
        self.root_node=doc.documentElement
        self.removeWhitespaceNodes(self.root_node)
        self.walk(self.root_node,RDF.Node(RDF.Uri(document)))
        doc.unlink()
    
    def walk(self,node,active_res):
        """
        Traverse XML node and build RDF triple
        node: xml Node
        active_res: RDF.Node
        """
        # Extract node info
        node_name=self.xml_to_internal(node)
        node_value=node.nodeValue
#        print "Searching "+node_name+' using base ontology '+self.base_onto
        if node.nodeType==Node.TEXT_NODE:
            tmp_res=self.handle_text(active_res,node_value)
            if tmp_res!=None:
                active_res=tmp_res
        elif (node.nodeType==Node.ELEMENT_NODE):
            tmp_res=self.handle_node(active_res,node_name)
            if tmp_res!=None:
                active_res=tmp_res
        for child in node.childNodes:
            tmp_res=self.walk(child,active_res)
            if tmp_res!=None:
                active_res=tmp_res
            attrs = node.attributes
            for attrName in attrs.keys():
                if not attrName.startswith('xmlns:'):
                    attrNode = attrs.get(attrName)
                    node_name=attrNode.localName
                    attrValue = attrNode.nodeValue
                    tmp_res=self.handle_node(active_res,node_name)
                    if tmp_res!=None:
                        active_res=tmp_res
                        tmp_res=self.handle_text(active_res,attrValue)
                        if tmp_res!=None:
                            active_res=tmp_res

    def handle_text(self,active_res,node_value):
        if (self.unfinished_statements is None) or (self.unfinished_statements==[]):
            print "No unfinished statements: PANIC!!!!"
            self.rdf_model.add_statement(RDF.Statement(active_res,
                                                        RDF.Node(RDF.Uri('http://error.it#PanicError')),
                                                        RDF.Node(node_value)
                                                        ))
        else:
        #    print "in Handle text: "+str(len(self.unfinished_statements))+"\n"
            for m in self.unfinished_statements:
                if (m.object is None) and (m.predicate!=None) and (m.subject!=None):
                    m.object=RDF.Node(node_value)
                    self.rdf_model.add_statement(m)
                    self.unfinished_statements.remove(m)
                    break
        return active_res

    def handle_node(self,active_res,node_name):
        node_onto_name=self.repository.get_onto_name(node_name,self.base_onto)
        if (node_onto_name=='') or (node_onto_name is None):
            print "Error: resource "+node_name+" not found"
            return None
        node_onto_type=self.repository.get_type(node_onto_name)
        print "Found resource: "+node_onto_name+' with type '+node_onto_type[0]
        
        # Properties
        if (node_onto_type[0]==self.owl_ns+'ObjectProperty') or (node_type[0]==self.owl_ns+'DatatypeProperty'):
            if (self.unfinished_statements is None) or (len(self.unfinished_statements)==0):
                self.unfinished_statements.append(RDF.Statement(active_res,RDF.Node(RDF.Uri(node_onto_name))))
#                print "in Handle property node: "+str(len(self.unfinished_statements))+"\n"
            else:
#               print "Active: "+str(active_res)+" found "+str(len(self.unfinished_statements))+" statements"
                for m in self.unfinished_statements:
                    if (m.object is None) and (m.predicate!=None) and (m.subject!=None):
                        m.object=RDF.Node(blank='pr'+str(self.counter))
                        self.rdf_model.add_statement(m)
                        self.unfinished_statements.append(RDF.Statement(m.object,RDF.Node(RDF.Uri(node_onto_name))))
#                        print "in Handle property node (for section): "+str(len(self.unfinished_statements))+"\n"
                        self.counter=self.counter+1
                        active_res=m.object
                        self.unfinished_statements.remove(m)
                        break
        
        # Class
        elif node_onto_type[0]==self.owl_ns+'Class':
#            print "Active resource: "+active_res
            # Document is the active resource
            if active_res==self.root_node:
#                print "in Handle class: added root statement"
                self.rdf_model.add_statement(RDF.Statement(active_res,
                                                            RDF.Node(RDF.Uri(self.rdf_ns+'type')),
                                                            RDF.Node(RDF.Uri(node_onto_name))
                                                            ))
            else:
                if (self.unfinished_statements is None) or (len(self.unfinished_statements)==0):
                    self.rdf_model.add_statement(RDF.Statement(active_res,
                                                                RDF.Node(RDF.Uri(self.rdf_ns+'type')),
                                                                RDF.Node(RDF.Uri(node_onto_name))
                                                                ))
                else:
                    for m in self.unfinished_statements:
                        print m
                        if (m.object is None) and (m.predicate!=None) and (m.subject.uri!=None):
                            m.object=RDF.Node(blank='cls'+str(self.counter))
                            self.rdf_model.add_statement(m)
                            self.rdf_model.add_statement(RDF.Statement(m.object,
                                                                        RDF.Node(RDF.Uri(self.rdf_ns+'type')),
                                                                        RDF.Node(RDF.Uri(node_onto_name))
                                                                        ))
                            self.counter=self.counter+1
                            active_res=m.object
#                            print "Removed unfinished statement "+m
                            self.unfinished_statements.remove(m)
                            break
        return active_res

    def xml_to_internal(self,xmltag):
        """
        Returns the name to use while searching into ontology resources
        xmltag: Node
        """
        if (xmltag is None):
            return ''
        return str(xmltag.nodeName)

    def removeWhitespaceNodes(self,parent):
        for child in list(parent.childNodes):
            if child.nodeType==Node.TEXT_NODE and child.data.strip()=='':
                parent.removeChild(child)
            else:
                self.removeWhitespaceNodes(child)


lift_inst=Lifter()
try:
    if __name__=='__main__':
        print "XML file: "+sys.argv[1]
        print "Ontology: "+sys.argv[2]
        lift_inst.lift(sys.argv[1],sys.argv[2])
except KeyboardInterrupt:
    print "------- "+str(len(lift_inst.unfinished_statements))+" Unfinished Statements ---------"
    for st in lift_inst.unfinished_statements:
        print st
serializer=RDF.Serializer()
serializer.set_namespace("doc", RDF.Uri(sys.argv[2]+"#"))
serializer.set_namespace("rim", RDF.Uri('http://veggente.berlios.de/ns/RIMOntology#'))
serializer.set_namespace("rim_dt", RDF.Uri('http://veggente.berlios.de/ns/RIMDatatype#'))
serializer.serialize_model_to_file("test-out.rdf", lift_inst.rdf_model)
