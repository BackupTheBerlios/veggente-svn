#!/usr/bin/python

from os import sys

# Standard RDF repository
class Repository:
		"""
		Standard RDF repository
		"""
		#public:
		db_dir='' # char*
		
		#constructor
		def __init__(self,db):
				self.db_dir=db
				if (self.init_db(self.db)==0):
						print 'RDF repository initialized'

		#public:
		def add_document(self, uri,  context):
				"""
				@uri:char*
				@context:char*
				"""
				return None
		
		def remove_document(self, context): 
				"""
				@context:char*
				"""
				return None
		
		def query_model(self, query_str,  context):
				"""
				@query_str:char*
				@context:char*
				"""
				return None

		def check_class(self, uri):
				"""
				@uri:char*
				"""
				return None
		
		def check_property(self, uri): 
				"""
				@uri:char*
				"""
				return None
		
		def __init_db(self, db):

				return 0

if __name__=="__main__":
		print "Veggente project: Conan RDF repository"
		sys.exit(0)

