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
    st_list=[]
    rdf_ns='http://www.w3.org/1999/02/22-rdf-syntax-ns#'
    rdfs_ns='http://www.w3.org/2000/01/rdf-schema#'
    owl_ns='http://www.w3.org/2002/07/owl#'
    counter=0

    def __init__(self,server_url='http://localhost:10000'):
        if (server_url is None) or (server_url==''):
            raise "Server URL must not be empty"
        self.server=server_url
        SOAPpy.Config.simplify_objects=1
        self.repository=SOAPpy.SOAPProxy(self.server)
        doc_store=RDF.Storage(storage_name="hashes",name="memstore",options_string="hash-type='memory'")
        if self.doc_store is None:
            raise "Failend creating in memory storage"
        self.rdf_model=RDF.Model(doc_store)
        if self.rdf_model is None:
            raise "Failed creating in memory RDF model"
 
    def lift(self,document):
        doc = xml.dom.minidom.parse(document)
        self.doc_context=RDF.Node(document)
        self.root_node=RDF.Node(doc.documentElement)
        self.walk(self.root_node,RDF.Node(RDF.Uri(document)))
        doc.unlink()
    
    def walk(self,node,active_res):
        """
        node: xml Node
        active_res: RDF.Node
        """
        # Extract node info
        node_name=node.namespaceURI+node.nodeName
        node_value=node.nodeValue
        node_onto_type=repository.check_type(node_name)
        node_onto_name=repository.check_onto_name(node_name)
        if node.type==node.TEXT_NODE:
            if self.unfinished_statements is None:
                print "No unfinished statements: PANIC!!!!"
                self.rdf_model.add_statement(RDF.Statement(active_res,
                                                            RDF.Node(RDF.Uri()),
                                                            RDF.Node(node_value)
                                                            ))
            else:
                for m in self.unfinished_statements:
                    if (m.object is None) and (m.predicate!=None) and (m.subject==active_res):
                        m.object=RDF.Node(node_value)
                        self.rdf_model.add_statement(m)
                        del self.unfinished_statements[m]
                        break
        elif (node.type==node.ELEMENT_NODE) or (node.type==node.ATTRIBUTE_NODE):
            # Properties
            if (node_onto_type==self.owl_ns+'ObjectProperty') or (node_type==self.owl_ns+'DatatypeProperty'):
                if self.unfinished_statements is None:
                    self.unfinished_statements.append(RDF.Statement(active_res,RDF.Node(RDF.Uri(node_onto_name))))
                else:
                    for m in self.unfinished_statements:
                        if (m.object is None) and (m.predicate!=None) and (m.subject==active_res):
                            m.object=RDF.Node(blank='pr'+self.counter)
                            self.rdf_model.add_statement(m)
                            self.unfinished_statements.append(RDF.Statement(m.object,RDF.Node(RDF.Uri(node_onto_name))))
                            self.counter=self.counter+1
                            active_res=m.object
                            del self.unfinished_statements[m]
                            break
            # Class
            else if node_onto_type==self.owl_ns+'Class':
                # Document is the active resource
                if active_res==self.root_node:
                    self.rdf_model.add_statement(RDF.Statement(active_res,
                                                                RDF.Node(RDF.Uri(self.rdf_ns+'type')),
                                                                RDF.Node(RDF.Uri(node_onto_name))
                                                                ))
                else:
                    if self.unfinished_statements is None:
                        self.rdf_model.add_statement(RDF.Statement(active_res,
                                                                    RDF.Node(RDF.Uri(self.rdf_ns+'type')),
                                                                    RDF.Node(RDF.Uri(node_onto_name))
                                                                    ))
                    else:
                        for m in self.unfinished_statements:
                            if (m.object is None) and (m.predicate!=None) and (m.subject==active_res):
                                m.object=RDF.Node(blank='cls'+self.counter)
                                self.rdf_model.add_statement(m)
                                self.rdf_model.add_statement(RDF.Statement(m.object,
                                                                            RDF.Node(RDF.Uri(self.rdf_ns+'type')),
                                                                            RDF.Node(RDF.Uri(node_onto_name))
                                                                            ))
                                self.counter=self.counter+1
                                active_res=m.object
                                del self.unfinished_statements[m]
                                break
        for child in node.childNodes:
            self.walk(node,active_res)

if __name__=='__main__':
    lift_inst=Lifter()
    lift_inst.lift(sys.argv[1])

