#!/usr/bin/python
import getopt
from os import sys
import RDF
from grounding import GroundingAgent

lift_inst=GroundingAgent()
rdf_graph=None
try:
    if __name__=='__main__':
        print '<?xml version="1.0"?>'
        print '<source_file name="'+sys.argv[1]+'"'
        print 'ontology_name="'+sys.argv[2]+'">'
        rdf_graph=lift_inst.lift(sys.argv[1],sys.argv[2])
except KeyboardInterrupt:
    print "Called interrupt"
# Testing. Remove when completed
serializer=RDF.Serializer()
serializer.set_namespace("doc", RDF.Uri(sys.argv[2]+"#"))
serializer.set_namespace("rim", RDF.Uri('http://veggente.berlios.de/ns/RIMOntology#'))
serializer.set_namespace("rim_dt", RDF.Uri('http://veggente.berlios.de/ns/RIMDatatype#'))
serializer.serialize_model_to_file("test-out.rdf",get_rdf_graph)

