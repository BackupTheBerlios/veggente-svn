#!/usr/bin/python
# Client for Veggente Conan OWL repository

import getopt
from os import sys

__version__='0.0.1'

def usage():
    print "Veggente project: Conan OWL repository client"
    print "owlrepository [-h] [-d] [-v] [-p n]"
    print "--help: print this help"
    print "--version: show version"
    print "--debug: debug flag"
    print "--server : start the server on a specified port"
 
def version():
    print "Veggente project: Conan OWL repository client v. "+__version__

def soap_exec(server,action,arguments[]):
    if action=='onto_add': 
    if action=='onto_del': 
    if action=='instance_add': 
    if action=='instance_del': 
    if action=='rdf_add': 
    if action=='rdf_del': 
    if action=='': 

if __name__=='__main__':
    server='http://localhost:10000'
    num_actions=0
    target=''
    if len(sys.argv)<2:
        usage()
        sys.exit(0)
    try:
        opts, args=getopt.getopt(sys.argv[1:],'hdv',['help','debug','version','onto_add=','onto_del=','instance_add=','instance_del=','rdf_add=','rdf_del='])
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
            target=val
        if opt in ('--onto_del'):
            num_actions=num_actions+1
            target=val
        if opt in ('--instance_add'):
            num_actions=num_actions+1
            target=val
        if opt in ('--instance_del'):
            num_actions=num_actions+1
            target=val
        if opt in ('--rdf_add'):
            num_actions=num_actions+1
            target=val
        if opt in ('--rdf_del'):
            num_actions=num_actions+1
            target=val
    
    if num_actions>1:
        print 'Error: only one action is allowed'
    soap_exec()
