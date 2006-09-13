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

class OWL_Class(OWL_Resource):
    obj_properties=[]
    dt_properties=[]
    mandatory_properties=[]
    
    def __init__(self,resource,node):
        rdf_type='http://www.w3.org/2002/07/owl#Class'
        OWL_Resource.__init__(self,resource,node)

class OWL_Property(OWL_Resource):
    domains=[]
    ranges=[]


