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
    doc_uri=None
    root_node=None
    repository=None
    doc_store=None
    rdf_model=None
    rdf_ns='http://www.w3.org/1999/02/22-rdf-syntax-ns#'
    rdfs_ns='http://www.w3.org/2000/01/rdf-schema#'
    owl_ns='http://www.w3.org/2002/07/owl#'
    current_statement=None
    counter=0

    def __init__(self,server_url='http://localhost:10000'):
        if (server_url is None) or (server_url==''):
            raise "Server URL must not be empty"
        self.server=server_url
        SOAPpy.Config.simplify_objects=1
        self.repository=SOAPpy.SOAPProxy(self.server)
        self.doc_store=RDF.Storage(storage_name="hashes",name="memstore",options_string="hash-type='memory'")
        if self.doc_store is None:
            raise "Failend creating in memory storage"
        self.rdf_model=RDF.Model(self.doc_store)
        if self.rdf_model is None:
            raise "Failed creating in memory RDF model"
 
    def lift(self,document):
        doc = xml.dom.minidom.parse(document)
        self.doc_context=RDF.Node(document)
        self.root_node=doc.documentElement
        self.current_statement=Statement(subject=document)
        self.walk(self.root_node,document)
        doc.unlink()
    
    # TODO: stabilire i valori ritornati dalla check. Dovrebbe essere una coppia qualified name+type
    def walk(self,node,active_res):
        # Extract node info
        node_name=node.namespaceURI+node.nodeName
        node_value=node.nodeValue
        node_type=repository.check_type(node_name)
        # Check node type
        if (node_type==owl_ns+'ObjectProperty') or (node_type==owl_ns+'DatatypeProperty'):
            pass
        else if node_type==owl_ns+'Class':
            # Document is the active resource
            if node==self.root_node:
                rdf_model.add_statement(RDF.Statement(RDF.Uri(document),
                                                        RDF.Uri(rdf_ns+'type'),
                                                        RDF.Uri(self.node_name),
                                                        self.doc_context)
                                                        ))
                for child in node.childNodes:
                    self.walk(node,document)
            else:
                unfinished_statements=self.rdf_model.find_statements(RDF.Statement(RDF.Node(active_res)))
                if unfinished_statements is None:
                else:

if __name__=='__main__':
    lift_inst=Lifter()
    lift_inst.lift(sys.argv[1])

