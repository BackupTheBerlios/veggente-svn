#!/usr/bin/python

#	Veggente - conan_client
#   Client for Veggente Conan OWL repository
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
from os import sys

__version__='0.0.1'

def usage():
    print "Veggente project: Conan OWL repository client"
    print "owlrepository [-h] [-d] [-v] [-p n]"
    print "--help: print this help"
    print "--version:           show version"
    print "--debug:             debug flag"
    print "--server uri:        start the server on a specified port"
    print "--list:              List documents in store"
    print "--onto_add uri:      Adds an ontology"
    print "--onto_del uri:      Removes an ontology"
    print "--instance_add uri:  Adds an instance document"
    print "--instance_del uri:  Removes an instance document"
    print "--rdf_add uri:       Adds an RDF document"
    print "--rdf_del uri:       Removes an RDF document"
 
def version():
    print "Veggente project: Conan OWL repository client v. "+__version__

def soap_exec(server,action,arguments):
    SOAPpy.Config.simplify_objects=1
    remote=SOAPpy.SOAPProxy(server)
    if action=='list':
        print "Server documents:"
        for i in remote.list_documents():
            print i
        return 0
    if action=='onto_add':
        return remote.add_ontology(arguments[0],False)
    if action=='onto_del': 
        return remote.remove_ontology(arguments[0],False)
    if action=='instance_add': 
        return remote.add_instance_document(arguments[0],False)
    if action=='instance_del': 
        return remote.remove_instance_document(arguments[0],False)
    if action=='rdf_add': 
        return remote.add_document(arguments[0],False)
    if action=='rdf_del': 
        return remote.remove_document(arguments[0])

if __name__=='__main__':
    server='http://localhost:10000'
    num_actions=0
    target=[]
    if len(sys.argv)<2:
        usage()
        sys.exit(0)
    try:
        opts, args=getopt.getopt(sys.argv[1:],'hdv',['help','debug','version','list','server=','onto_add=','onto_del=','instance_add=','instance_del=','rdf_add=','rdf_del='])
    except getopt.GetoptError:
        usage()
        sys.exit(-1)
    for opt, val in opts:
        if opt in ('--help'):
            usage()
            sys.exit(0)
        if opt in ('--version'):
            version()
            sys.exit(0)
        if opt in ('--server'):
            server=val
        if opt in ('--onto_add'):
            num_actions=num_actions+1
            action='onto_add'
            target.append(val)
        if opt in ('--list'):
            num_actions=num_actions+1
            action='list'
            target.append(val)
        if opt in ('--onto_del'):
            num_actions=num_actions+1
            action='onto_del'
            target.append(val)
        if opt in ('--instance_add'):
            num_actions=num_actions+1
            action='instance_add'
            target.append(val)
        if opt in ('--instance_del'):
            num_actions=num_actions+1
            action='instance_del'
            target.append(val)
        if opt in ('--rdf_add'):
            num_actions=num_actions+1
            action='rdf_add'
            target.append(val)
        if opt in ('--rdf_del'):
            num_actions=num_actions+1
            action='rdf_del'
            target.append(val)
    
    if num_actions>1:
        print 'Error: only one action is allowed'
    if soap_exec(server,action,target)==0:
        print 'Operation '+action+' on server '+server+' completed'
