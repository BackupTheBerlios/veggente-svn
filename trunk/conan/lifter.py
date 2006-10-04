#!/usr/bin/python
"""
Veggente - lifter
  Implementation of an XML to RDF algorithm

Copyright(c) 2006 Alessio Carenini <carenini@gmail.com>
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
"""

import getopt
import SOAPpy
import RDF
from OWL import OWL_Resource, OWL_Class, OWL_Property
from os import sys
from sets import Set
import xml.dom.minidom
from xml.dom.minidom import Node


class Lifter:
    """
    Lifter class' rdf graph contains two contexts:
        instance: identify triples coming from the lifting process
        ontology: identify triples coming from the ontology cluster
    """
    __server=''
    __root_node=None
    __repository=None
    __rdf_model=None
    __parser=None
    
    __rdf_ns='http://www.w3.org/1999/02/22-rdf-syntax-ns#'
    __rdfs_ns='http://www.w3.org/2000/01/rdf-schema#'
    __owl_ns='http://www.w3.org/2002/07/owl#'
    
    __base_onto=''
    __uml_style=True
    __blank_counter=0
    __imports=[]
    # Caches for all properties that applies to a class (property domain)
    __class_dt_properties_cache={}
    __class_obj_properties_cache={}
    # Cache for property range
    __property_range_cache={}


    __datatypes_blacklisted=['http://veggente.berlios.de/ns/RIMDatatype#nullFlavor']

    # Stats attributes
    __node_counter=0
    __cache_hit=0
    __cache_miss=0
    __query_counter=0

    def __init__(self,server_url='http://localhost:10000'):
        """
        Class contructor
        """
        self.__resource_cache={}
        if (server_url is None) or (server_url==''):
            raise "Server URL must not be empty"
        self.__server=server_url
        SOAPpy.Config.simplify_objects=1
        self.__repository=SOAPpy.SOAPProxy(self.__server)
        self.__parser=RDF.Parser(name='rdfxml')
        if self.__parser is None:
            raise "Failed initializing RDF parser"
        doc_store=RDF.Storage(storage_name="hashes",name="memstore",options_string="hash-type='memory'")
        if doc_store is None:
            raise "Failed creating in-memory storage"
        self.__rdf_model=RDF.Model(doc_store)
        if self.__rdf_model is None:
            raise "Failed creating in-memory RDF model"
         
    def lift(self,document,ontology,uml_mode=True):
        """
        Convert the content of an XML document into the corresponding set of RDF triples
        document: uri of the document to convert
        onto_list: list of ontologies intantiated by the document
        uml_mode: tells lifter to use UML-style notation for element name resolution
        """
        if (ontology is None) or (document is None):
            return None
        self.__uml_style=uml_mode
        self.__base_onto=ontology
        doc = xml.dom.minidom.parse(document)
        self.doc_context=RDF.Node(document)
        self.__root_node=doc.documentElement
        self.__removeWhitespaceNodes(self.__root_node)
        active_classes=[]
        self.__imports=self.__repository.get_imports(self.__base_onto)
        self.__handle_element(self.__root_node,[],None,None,None)
        self.__output_stats()
        doc.unlink()

    def get_rdf_graph(self):
        """
        Returns the class' Redland RDF graph
        """
        return self.__rdf_model
    
    def __handle_element(self,node,active_classes,known_resource,known_type,known_subject):
        """
        Processes an XML node and converts it in the corresponding RDF triples, according to the ontology supplied to the main class
        """
        child_list=[]
        attr_list=[]
        attrs=None
        obj_target_found=False
        activator=None
        new_subject=None

        if (node is None):
            return None
        if (known_type is None) and (known_resource is None):
            rdf_name,rdf_type=self.__resolve_node(str(node.nodeName),active_classes)
        else:
            rdf_name=known_resource
            rdf_type=known_type
        self.__node_counter=self.__node_counter+1

        print '<node name="'+str(node.nodeName)+'">'
        if (rdf_name is None) or (rdf_type is None):
            print "<NotFoundError>"+str(node.nodeName)+"</NotFoundError>"
            return None
        for a in active_classes:
            print '<active>'+a.name+'</active>'

        # Handle OWL types "Class" and "DatatypeProperty"
        if (rdf_type==self.__owl_ns+'Class'):
            new_class=self.__add_class_instance(rdf_name,known_subject)
            if (new_class!=None):
                print '<Class>'+rdf_name+'</Class>'
                active_classes.insert(0,new_class)
        elif (rdf_type==self.__owl_ns+'DatatypeProperty'):
            if (node.nodeType==Node.ELEMENT_NODE):
                for n in node.childNodes:
                    if n.nodeType==Node.TEXT_NODE:
                        print '<Datatype>'+rdf_name+'</Datatype>'
                        new_subject=self.__add_data_property_instance(subject,rdf_name,n.nodeValue)
                        break
            elif (node.nodeType==Node.ATTRIBUTE_NODE):
                print '<Datatype>'+rdf_name+'</Datatype>'
                self.__add_data_property_instance(rdf_name,node.nodeValue)
        elif (rdf_type==self.__owl_ns+'ObjectProperty'):
            pass
        else:
            print "<type_error>"+rdf_type+"</type_error>"
        child_list=self.__child_elements_to_resources(node,active_classes)
        attr_list=self.__child_attributes_to_resources(node,active_classes)
        self.__node_counter=self.__node_counter+len(attr_list)
        node_list=attr_list
        node_list.extend(child_list)
        
        # Handle OWL type "ObjectProperty"
        if (rdf_type==self.__owl_ns+'ObjectProperty'):
            #print '<Object>'+rdf_name+'</Object>'
            subject_class=None
            target_class=None
            obj_range_list=None
            if rdf_name in self.__property_range_cache:
                obj_range_list=self.__property_range_cache[rdf_name]
            else:
                obj_range_list=self.__repository.get_property_range(rdf_name,self.__base_onto)
                self.__property_range_cache[rdf_name]=obj_range_list
            # Re-consider how to treat obj domains. There exists A CACHE!
            obj_domain_list=self.__repository.get_property_domain(rdf_name,self.__base_onto)
            # Subject search
            if (obj_range_list==[]):
                print '<no_range_warning/>\n</node>'
            else:
                #print '<range>'+str(obj_range_list)+'</range>'
                # --- Search for a subject ---
                if (active_classes==[]) and (len(obj_domain_list)==0):
                    # 1: The corresponding xml element is the root element
                    new_class=self.__add_class_instance(str(obj_domain_list[0]),None)
                    active_classes.insert(0,new_class)
                elif (len(obj_domain_list)==0):
                    # 2: The property has no domain, forcing the first active class as the current domain
                    #print '<forced_domain>'+(active_classes[0]).name+'</forced_domain>'
                    subject_class=active_classes[0]
                else:
                    # 3: Look if an active class is among the domains of the property
                    #print '<domain>'+str(obj_domain_list)+'</domain>'
                    for candidate_class in active_classes:
                        if candidate_class.get_resource() in obj_domain_list:
                            subject_class=candidate_class
                            break
                # --- Computing new triples ---
                if (subject_class!=None):
                    if (node.nodeType==Node.ELEMENT_NODE):
                        if (node_list is None):
                            # Create an empty class instance
                            new_class=self.__add_class_instance(str(obj_range_list[0]),None)
                        else:
                            for el in node_list:
                                if el.get_resource() in obj_range_list:
                                    activator=el
                                    break
                            if (activator!=None):
                                # Class activator found
                                new_subject=self.__add_obj_property_instance(subject_class,rdf_name,None)
                                for el in node_list:
                                    self.__handle_element(el.get_xml_node(),active_classes,el.get_resource(),el.get_type(),new_subject)
                            else: 
                                # Class activator NOT found
                                new_class=self.__add_obj_property_instance(subject_class,rdf_name,obj_range_list[0])
                                active_classes.insert(0,new_class)
                                # A new class has been inserted => recalculate node-ontology match
                                child_list=self.__child_elements_to_resources(node,active_classes)
                                attr_list=self.__child_attributes_to_resources(node,active_classes)
                                node_list=attr_list
                                node_list.extend(child_list)
                                for el in node_list:
                                    self.__handle_element(el.get_xml_node(),active_classes,el.get_resource(),el.get_type(),new_subject)
                                active_classes.remove(new_class)
                        print '</node>'
                        return None
                    elif node.nodeType==Node.ATTRIBUTE_NODE:
                        new_obj=self.__search_class_for_data(obj_range_list[0],node.nodeValue)
                        if new_obj!=None:
                            print new_obj
                            self.__add_obj_property_instance(subject_class,rdf_name,new_obj.get_rdf_node())
                        else:
                            # Restricted search failed, allowing the use of blacklisted properties
                            new_obj=self.__search_class_for_data(obj_range_list[0],node.nodeValue,[],True)
                            if new_obj!=None:
                                print new_obj
                                self.__add_obj_property_instance(subject_class,rdf_name,new_obj.get_rdf_node())
                        print '</node>'
                        return None
        print '</node>'
        for el in node_list:
            self.__handle_element(el.get_xml_node(),active_classes,el.get_resource(),el.get_type(),None)
        return None

    def __search_class_for_data(self,starting_class,textual_data,path=[],use_blacklist=True):
        """
        Resolve the case in which an attribute is linked to an ObjectProperty. Search a class with a 
        DatatypeProperty following the ObjectProperties
        starting_class(OWL_Class): starting point for the search
        textual_data (string): value of the DatatypeProperty which is the object of the search
        path (list of string): loop detection
        use_blacklist (boolean): if True exclude a set of DatatypeProperties from the search
        """
        dt_props=None
        obj_props=None
        current_class=None
        current_class_uri=None
        if (textual_data is None) or (starting_class is None):
            return None
        if (type(starting_class)==OWL_Class):
            current_class=starting_class
            current_class_uri=starting_class.get_resource()
        elif (type(starting_class==str)):
            current_class_uri=starting_class
        if current_class_uri in path:
            # Found loop, aborting
            return None
        self.__query_counter=self.__query_counter+1
        # --- DatatypeProperty cache update ---
        if current_class_uri in self.__class_dt_properties_cache:
            dt_props=self.__class_dt_properties_cache[current_class_uri]
        else:
            dt_props=self.__repository.get_datatype_properties(current_class_uri,self.__base_onto)
            self.__class_dt_properties_cache[current_class_uri]=dt_props
        # ---
        for p in dt_props:
            if use_blacklist:
                if (not (p in self.__datatypes_blacklisted)):
                    if current_class is None:
                        current_class=self.__add_class_instance(current_class_uri)
                    self.__add_data_property_instance(current_class.get_rdf_node(),p,str(textual_data))
            else:
                if current_class is None:
                    current_class=self.__add_class_instance(current_class_uri)
                self.__add_data_property_instance(current_class.get_rdf_node(),p,str(textual_data))
            return current_class
        # --- ObjectProperty cache update ---
        if current_class_uri in self.__class_obj_properties_cache:
            obj_props=self.__class_obj_properties_cache[current_class_uri]
        else:
            obj_props=self.__repository.get_object_properties(current_class_uri,self.__base_onto)
            self.__class_obj_properties_cache[current_class_uri]=obj_props
        # ---
        for prop in self.__class_obj_properties_cache[current_class_uri]:
            print 'Selected property: '+prop
            ranges=None
            # --- Range Cache update ---
            if prop in self.__property_range_cache:
                ranges=self.__property_range_cache[prop]
            else:
                ranges=self.__repository.get_property_range(prop,self.__base_onto)
                self.__property_range_cache[prop]=ranges
            # ---
            # Recursion step
            if (len(ranges)>1):
                print '<warning prop="'+prop+'">multiple RANGES</warning>'
            for r in ranges:
                path.append(current_class_uri)
                obj_class=self.__search_class_for_data(r,textual_data,path)
                if obj_class!=None:
                    current_class=self.__add_class_instance(current_class_uri)
                    self.__add_obj_property_instance(current_class,prop,obj_class.get_rdf_node())
                    return current_class
                else:
                    path.remove(current_class_uri)
        return None

        
        
    def __child_attributes_to_resources(self,xml_node,active_classes):
        """
        Return a list of OWLResource, resolving all childs XML attributes
        """
        attrs = xml_node.attributes
        attr_list=[]
        if (attrs!=None):
            for attrName in attrs.keys():
                if (not attrName.startswith('xmlns'))and(not attrName=='xsi:schemaLocation'):
                    attrNode = attrs.get(attrName)
                    child_rdf_res,child_rdf_type=self.__resolve_node(str(attrNode.localName),active_classes)
                    if (child_rdf_res!=None) and (child_rdf_type!=None):
                        child_obj=OWL_Resource(child_rdf_res,child_rdf_type,attrNode)
                        attr_list.append(child_obj)
                    else:
                        print '<resolution_error>'+str(attrNode.localName)+'</resolution_error>'
        return attr_list

    def __child_elements_to_resources(self,xml_node,active_classes):
        """
        Return a list of OWLResource, resolving all childs XML elements
        """
        child_list=[]
        for child in xml_node.childNodes:
            if (child.nodeType==Node.ELEMENT_NODE):
                child_rdf_res,child_rdf_type=self.__resolve_node(str(child.localName),active_classes)
                if (child_rdf_res!=None) and (child_rdf_type!=None):
                    child_obj=OWL_Resource(child_rdf_res,child_rdf_type,child)
                    child_list.append(child_obj)
                else:
                    print '<resolution_error>'+str(child.localName)+'</resolution_error>'
        return child_list

    def __update_cache(self,active_classes):
        """
        Updates both ObjectProperties' and DatatypeProperties' domain caches
        active_classes (list of OWL_Class): entries to be updated
        """
        for el in active_classes:
            class_uri=el.get_resource()
            if (not (class_uri in self.__class_obj_properties_cache)) and (not (class_uri in self.__class_dt_properties_cache)):
                object_properties=self.__repository.get_object_properties(class_uri,self.__base_onto)
                if (object_properties!=[]):
                    self.__class_obj_properties_cache[class_uri]=object_properties
                datatype_properties=self.__repository.get_datatype_properties(el.get_resource(),self.__base_onto)
                if (datatype_properties!=[]):
                    self.__class_dt_properties_cache[class_uri]=datatype_properties
        return None

    def __resolve_node(self,node,active_classes):
        """
        Search in the OWL __repository for a matching resource, using a list of classes in case of an UML-style notation
            node (string): XML node name
            active_classes (list of OWL_Class): classes to be used in UML notation search
        Returns:
            (string) Found URI resource
            (string) Found RDF type
        """
        # --- Fill in info on newly added classes
        self.__update_cache(active_classes)
        # --- Begin cache search ---
        cached_uri=None
        cached_type=None
        cached_uri,cached_type=self.__resolve_cached_node(node,active_classes)
        if (cached_uri!=None) and (cached_type!=None):
            self.__cache_hit=self.__cache_hit+1
            #print '<cache_hit>'+cached_uri+'</cache_hit>'
            return cached_uri,cached_type
        # --- Cache search failed, proceeding with remote search ---
        #print '<cache_miss>'+node+'</cache_miss>'
        self.__cache_miss=self.__cache_miss+1
        return self.__resolve_uncached_node(node,active_classes)

    def __resolve_cached_node(self,node,active_classes):
        """
        Check if a node has already been resolved and it's in the cache
        """
        plain_name=self.__xml_to_internal(node,None)
        # Property search
        for el in active_classes:
            class_uri=el.get_resource()
            uml_name=self.__xml_to_internal(node,class_uri)
            if class_uri in self.__class_obj_properties_cache:
                if uml_name in self.__class_obj_properties_cache[class_uri]:
                    return uml_name,self.__owl_ns+'ObjectProperty'
                if plain_name in self.__class_obj_properties_cache[class_uri]:
                    return plain_name,self.__owl_ns+'ObjectProperty'
            if class_uri in self.__class_dt_properties_cache:
                if uml_name in self.__class_dt_properties_cache[class_uri]:
                    return uml_name,self.__owl_ns+'DatatypeProperty'
                if plain_name in self.__class_dt_properties_cache[class_uri]:
                    return plain_name,self.__owl_ns+'DatatypeProperty'
        # Class search
        for imp in self.__imports:
            candidate_uri=imp+'#'+node
            if (candidate_uri in self.__class_obj_properties_cache) or (candidate_uri in self.__class_dt_properties_cache):
                return candidate_uri,self.__owl_ns+'Class'
        # Last chance. UML-notation property inherited from another class
        for el in active_classes:
            class_uri=el.get_resource()
            if class_uri in self.__class_obj_properties_cache:
                for obj in self.__class_obj_properties_cache[class_uri]:
                    if obj.split('.')[1]==plain_name:
                        return obj,self.__owl_ns+'ObjectProperty'
            if class_uri in self.__class_dt_properties_cache:
                for dt in self.__class_dt_properties_cache[class_uri]:
                    if dt.split('.')[1]==plain_name:
                        return dt,self.__owl_ns+'DatatypeProperty'
        return None,None

    def __resolve_uncached_node(self,node,active_classes):
        """
        Check what kind of OWL resource match the input name
        """
        # Begin searching with UML style notation
        node_onto_name=None
        node_onto_type=None
        xml_res_name=self.__xml_to_internal(node,None)
        node_onto_name,node_onto_type=self.__repository.onto_identify(xml_res_name,self.__base_onto)
        self.__query_counter=self.__query_counter+1
        if (node_onto_name!='') and (node_onto_name!=None) and (node_onto_type!="") and (node_onto_type!=None):
            return node_onto_name,node_onto_type
        else:
            return None,None

    def __xml_to_internal(self,xmltag,base_class):
        """
        Returns the name to use while searching into ontology resources
        xmltag: string
        """
        if (xmltag is None):
            return ''
        if (self.__uml_style==False) or (base_class is None) or (base_class==''):
            return xmltag
        else:
            return base_class+'.'+xmltag

    def __removeWhitespaceNodes(self,parent):
        for child in list(parent.childNodes):
            if (child.nodeType==Node.TEXT_NODE) and (child.data.strip()==''):
                parent.removeChild(child)
            else:
                self.__removeWhitespaceNodes(child)

    def __add_class_instance(self,class_res,subj_node=None):
        """
        Add a class instance to the store
        Parameters:
            class name (string)
        Returns:
            OWL_Class
        """
        new_subject=subj_node
        if (class_res is None):
            return None
        if (subj_node is None):
            new_subject=RDF.Node(blank='inst'+str(self.__blank_counter))
            self.__blank_counter=self.__blank_counter+1
        instance=OWL_Class(class_res,new_subject)
        self.__rdf_model.add_statement(RDF.Statement(new_subject,
                                                    RDF.Node(RDF.Uri(self.__rdf_ns+'type')),
                                                    RDF.Node(RDF.Uri(class_res))
                                                    ))
        return instance

    
    def __add_obj_property_instance(self,subject,property_res,dest):
        """
        Add an ObjectProperty instance to the store
        Parameters:
            subject (OWL_Class): current class
            property_res (string): ObjectProperty resource
            dest (string, Blank Node or None): destination class of the ObjectProperty
        Returns:
            RDF.Node if no class has been instantiated
            OWL_Class if a new Class instance has been added to model
        """
        object=None
        new_class_instance=None
        if (subject is None) or (property_res is None):
            print 'ObjProp instance failed'
            return None
        if isinstance(dest,RDF.Node):
            object=dest
        elif type(dest)==str:
            new_class_instance=self.__add_class_instance(dest,None)
            object=new_class_instance.get_rdf_node()
        elif dest is None:
            object=RDF.Node(blank='inst'+str(self.__blank_counter))
            self.__blank_counter=self.__blank_counter+1
        new_st=RDF.Statement(subject.get_rdf_node(),RDF.Node(RDF.Uri(property_res)),object)
        self.__rdf_model.add_statement(new_st)
        if new_class_instance is None:
            return object
        else:
            return new_class_instance

    def __add_data_property_instance(self,subject,property_res,textual_content):
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
        self.__rdf_model.add_statement(RDF.Statement(subject,
                                                    RDF.Node(RDF.Uri(property_res)),
                                                    RDF.Node(literal=textual_content)
                                                    ))
        return subject

    def __output_stats(self):
        print '<stats><node_count>'+str(self.__node_counter)+'</node_count>'
        print '<query_count>'+str(self.__query_counter)+'</query_count>'
        print '<cache_hit>'+str(self.__cache_hit)+'</cache_hit>'
        print '<cache_miss>'+str(self.__cache_miss)+'</cache_miss></stats>'
        print '</source_file>'

lift_inst=Lifter()
try:
    if __name__=='__main__':
        print '<?xml version="1.0"?>'
        print '<source_file name="'+sys.argv[1]+'"'
        print 'ontology_name="'+sys.argv[2]+'">'
        lift_inst.lift(sys.argv[1],sys.argv[2])
except KeyboardInterrupt:
    print "Called interrupt"
# Testing. Remove when completed
serializer=RDF.Serializer()
serializer.set_namespace("doc", RDF.Uri(sys.argv[2]+"#"))
serializer.set_namespace("rim", RDF.Uri('http://veggente.berlios.de/ns/RIMOntology#'))
serializer.set_namespace("rim_dt", RDF.Uri('http://veggente.berlios.de/ns/RIMDatatype#'))
serializer.serialize_model_to_file("test-out.rdf", lift_inst.get_rdf_graph())
