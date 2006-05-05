# OWL-enabled RDF repository
class OWLRepository (Repository):
		"""
		OWL-enabled RDF repository
		"""
		
		#public:
		def exec_inference(self, context_uri,  results_uri):
				"""
				@context_uri:char*
				@results_uri:char* :Where to store inferred statements
				"""
				return None
		
		def is_allowed(self, property_uri,  class_uri):
				"""
				@property_uri:char*
				@class_uri:char*
				"""
				return None


