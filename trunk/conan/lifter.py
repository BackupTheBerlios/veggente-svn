#!/usr/bin/python

#	Veggente - lifter
#   Implementation of an XML to RDF algorithm
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

# TODO: active_classes has to be treated as a STACK!!!!

import getopt
import SOAPpy
import RDF
from OWL import OWL_Resource, OWL_Class, OWL_Property
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
    blank_counter=0

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
            raise "Failed creating in-memory storage"
        self.rdf_model=RDF.Model(doc_store)
        if self.rdf_model is None:
            raise "Failed creating in-memory RDF model"
         
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
        doc = xml.dom.minidom.parse(document)
        self.doc_context=RDF.Node(document)
        self.root_node=doc.documentElement
        self.removeWhitespaceNodes(self.root_node)
        active_classes=[]
        self.handle_element(self.root_node,[],None,None)
        doc.unlink()
    
    def handle_element(self,node,active_classes,known_resource,known_type):
        """
        Processes an XML node and converts it in the corresponding RDF triples, according to the ontology supplied to the main class
        """
        tmp_classes=active_classes
        child_list=[]
        attr_list=[]
        attrs=None
        obj_target_found=False

        if (node is None):
            return None
        if (known_type is None) and (known_resource is None):
            rdf_name,rdf_type=self.resolve_node(str(node.nodeName),active_classes)
        else:
            rdf_name=known_resource
            rdf_type=known_type

        print '<node>\n <name>'+str(node.nodeName)+'</name>'
        if (rdf_name is None) or (rdf_type is None):
            print "<NotFoundError>"+str(node.nodeName)+"</NotFoundError>"
            return None

        attrs = node.attributes

        #TODO:  per le proprieta' inserire i controlli di applicabilita' rispetto alla classe attiva
        #       inserire controllo per vedere se una classe ha tutte le proprieta' richieste

        # Handle OWL types "Class" and "DatatypeProperty"
        if (rdf_type==self.owl_ns+'Class'):
            new_class=self.add_class_instance(rdf_name)
            if (new_class!=None):
                print ' <Class>'+rdf_name+'</Class>'
                tmp_classes.insert(0,new_class)
        elif (rdf_type==self.owl_ns+'DatatypeProperty'):
            if (node.nodeType==Node.ELEMENT_NODE):
                for n in node.childNodes:
                    if n.nodeType==Node.TEXT_NODE:
                        print ' <Datatype>'+rdf_name+'</Datatype>'
                        new_subject=self.add_data_property_instance(subject,rdf_name,n.nodeValue)
                        break
            elif (node.nodeType==Node.ATTRIBUTE_NODE):
                print ' <Datatype>'+rdf_name+'</Datatype>'
                self.add_data_property_instance(rdf_name,node.nodeValue)
        elif (rdf_type==self.owl_ns+'ObjectProperty'):
            pass
        else:
            print "<type_error>"+rdf_type+"</type_error"
        # Flatten attributes and childs in one list
        print '<info>'
        if (attrs!=None):
            for attrName in attrs.keys():
                if (not attrName.startswith('xmlns'))and(not attrName=='xsi:schemaLocation'):
                    attrNode = attrs.get(attrName)
                    child_rdf_res,child_rdf_type=self.resolve_node(str(attrNode.localName),active_classes)
                    if (child_rdf_res!=None) and (child_rdf_type!=None):
#                        print '<resolved>\n <name>'+child_rdf_res+'</name>\n <type>'+child_rdf_type+'</type>\n</resolved>'
                        child_obj=OWL_Resource(child_rdf_res,child_rdf_type,attrNode)
                        attr_list.append(child_obj)
                    else:
                        print '<resolution_error>'+str(attrNode.localName)+'</resolution_error>'
        for child in node.childNodes:
            if (child.nodeType==Node.ELEMENT_NODE):
                child_rdf_res,child_rdf_type=self.resolve_node(str(child.localName),active_classes)
                if (child_rdf_res!=None) and (child_rdf_type!=None):
 #                   print '<resolved>\n <name>'+child_rdf_res+'</name>\n <type>'+child_rdf_type+'</type>\n</resolved>'
                    child_obj=OWL_Resource(child_rdf_res,child_rdf_type,child)
                    child_list.append(child_obj)
                else:
                    print '<resolution_error>'+str(child.localName)+'</resolution_error>'
        print '</info>'
        # Handle OWL type "ObjectProperty"
        if (rdf_type==self.owl_ns+'ObjectProperty'):
            obj_range_list=self.repository.get_property_range(rdf_name,self.base_onto)
            print ' <Object>\n <name>'+rdf_name+'</name>'
            if (obj_range_list==[]):
                print '<range_error>'+rdf_name+'</range_error>'
            else:
                print ' <range>'+str(obj_range_list)+'</range>'
                print '<active_classes>'+str(active_classes)+'</active_classes>'
                print '</Object>\n </node>'
                if (node.nodeType==Node.ATTRIBUTE_NODE):
                # Begin searching for a datatype property
                    pass
                elif (node.nodeType==Node.ELEMENT_NODE):
                    for range,c in zip(obj_range_list,attr_list):
                        # Search for a class activator
                        if (range==c.resource):
                            # A 'class element' exists
                            # far ritornare alla handle_element il nuovo soggetto
                            self.handle_element(c.get_xml_node(),active_classes,c.get_uri(),c.get_type())
                            child_list.remove(c)
                            obj_target_found=True
                            break
                    if (not obj_target_found):
                        for range,c in zip(obj_range_list,child_list):
                            # Search for a class activator
                            if (range==c.resource):
                                # A 'class element' exists
                                # far ritornare alla handle_element il nuovo soggetto
                                self.handle_element(c.get_xml_node(),active_classes,c.get_uri(),c.get_type())
                                child_list.remove(c)
                                obj_target_found=True
                                break
                        if (not obj_target_found):
                            #Class activator not found, pre-instantiation of the object Class
                            class_instance=self.add_obj_property_instance(tmp_classes[0],rdf_name,obj_range_list[0])
                            tmp_classes.insert(0,class_instance)

        for el in child_list:
            self.handle_element(el.get_xml_node(),active_classes,el.get_uri(),el.get_type())
        return None
        

    def resolve_node(self,node,active_classes):
        """
        Search in the OWL repository for a matching resource, using a list of classes in case of an UML-style notation
            node (string): XML node name
            active_classes (list of OWL_Class): classes to be used in UML notation search
        Returns:
            (string) Found URI resource
            (string) Found RDF type
        """
        # Begin searching with UML style notation 
        for class_candidate in active_classes:
            xml_res_name=self.xml_to_internal(node,class_candidate.name)
            node_onto_name,node_onto_type=self.repository.onto_identify(xml_res_name,self.base_onto)
            if (node_onto_name!='') and (node_onto_name!=None) and (node_onto_type!="") and (node_onto_type!=None):
                return node_onto_name,node_onto_type
        # UML-style search failed
        xml_res_name=self.xml_to_internal(node,None)
        node_onto_name,node_onto_type=self.repository.onto_identify(xml_res_name,self.base_onto)
        if (node_onto_name!='') and (node_onto_name!=None) and (node_onto_type!="") and (node_onto_type!=None):
            return node_onto_name,node_onto_type
        else:
            return None,None

    def xml_to_internal(self,xmltag,base_class):
        """
        Returns the name to use while searching into ontology resources
        xmltag: string
        """
        if (xmltag is None):
            return ''
        if (self.uml_style==False) or (base_class is None) or (base_class==''):
            return xmltag
        else:
            return base_class+'.'+xmltag

    def removeWhitespaceNodes(self,parent):
        for child in list(parent.childNodes):
            if (child.nodeType==Node.TEXT_NODE) and (child.data.strip()==''):
                parent.removeChild(child)
            else:
                self.removeWhitespaceNodes(child)

    def search_class_for_data(self,obj_property,textual_data):
        """
        obj_property (OWL_Property): starting point for the search
        textual_data (string): value of the DatatypeProperty which is the object of the search
        """
        statement_list=[]
#        for r in obj_property.get_ranges():
#            if r
        # Heuristic valid only on HL7 v.3 domain   
    
    def add_class_instance(self,class_res):
        """
        Add a class instance to the store
        Parameters:
            class name (string)
        Returns:
            OWL_Class
        """
        if (class_res is None):
            return None
        new_subject=RDF.Node(blank='inst'+str(self.blank_counter))
        instance=OWL_Class(class_res,new_subject)
        self.rdf_model.add_statement(RDF.Statement(new_subject,
                                                    RDF.Node(RDF.Uri(self.rdf_ns+'type')),
                                                    RDF.Node(RDF.Uri(class_res))
                                                    ))
        self.blank_counter=self.blank_counter+1
        return instance
    
    def add_obj_property_instance(self,subject,property_res,dest_class):
        """
        Add an ObjectProperty instance to the store
        Parameters:
            subject (OWL_Class): current class
            property_res (string): ObjectProperty resource
            dest_class (string): destination class of the ObjectProperty
        Returns:
            RDF.Node
        """
        if (subject is None) or (property_res is None) or (dest_class is None):
            return None
        new_class_instance=self.add_class_instance(dest_class)
        new_subject=new_class_instance.get_rdf_node()
        self.rdf_model.add_statement(RDF.Statement(subject.get_rdf_node(),
                                                    RDF.Node(RDF.Uri(property_res)),
                                                    new_subject
                                                    ))
        return new_class_instance

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
    print "Called interrupt"
# Testing. Remove when completed
serializer=RDF.Serializer()
serializer.set_namespace("doc", RDF.Uri(sys.argv[2]+"#"))
serializer.set_namespace("rim", RDF.Uri('http://veggente.berlios.de/ns/RIMOntology#'))
serializer.set_namespace("rim_dt", RDF.Uri('http://veggente.berlios.de/ns/RIMDatatype#'))
serializer.serialize_model_to_file("test-out.rdf", lift_inst.rdf_model)
