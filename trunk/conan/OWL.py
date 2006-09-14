#!/usr/bin/python

#	Veggente - OWL
#   Some helper functions for OWL
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

class OWL_Resource:
    name=''
    resource=''
    rdf_type=''
    rdf_node=None
    xml_node=None
    def __init__(self,resource,node):
        self.name=resource.split('#')[1]
        self.resource=resource
        if (type(node)==RDF.Node):
            self.rdf_node=node
        elif (type(node)==Node):
            self.xml_node=node
    def __init__(self,redland_model,resource,type,node):
        self.rdf_type=type
        self.__init__(resource,node)
    def __str__(self):
        return self.name
    def get_xml_node(self):
        return self.xml_node
    def get_rdf_node(self):
        return self.rdf_node
    def get_uri(self):
        return self.resource
    def set_xml_node(self,node):
        self.xml_node=node
    def set_rdf_node(self,node):
        self.rdf_node=node

class OWL_Class(OWL_Resource):
    obj_properties=[]
    dt_properties=[]
    mandatory_properties=[]
    
    def __init__(self,resource,node):
        OWL_Resource.__init__(self,resource,node)
        self.set_type('http://www.w3.org/2002/07/owl#Class')
    
    def get_mandatory_properties(self):
        return self.mandatory_properties
    def get_dt_properties(self):
        return self.dt_properties
    def get_obj_properties(self):
        return self.obj_properties

    def set_mandatory_properties(self,prop_list):
        self.mandatory_properties=prop_list
    def set_dt_properties(self,prop_list):
        self.dt_properties=prop_list
    def set_obj_properties(self,prop_list):
        self.obj_properties=prop_list


class OWL_Property(OWL_Resource):
    domains=[]
    ranges=[]

    def get_domains(self):
        return self.domains
    def get_ranges(self):
        return self.ranges

    def set_ranges(self,range_list):
        self.ranges=range_list
    def set_domains(self,domain_list):
        self.domains=domain_list
    
