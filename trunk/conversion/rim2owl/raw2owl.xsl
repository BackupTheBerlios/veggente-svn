<?xml version="1.0"?>
<!DOCTYPE rdf:RDF [
	<!ENTITY rdf "http://www.w3.org/1999/02/22-rdf-syntax-ns#">
	<!ENTITY rdfs "http://www.w3.org/2000/01/rdf-schema#">
	<!ENTITY xsd  "http://www.w3.org/2001/XMLSchema#" >
]>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:hl7_dt="http://veggente.berlios.de/ns/RIMDatatype#"
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
	xmlns:owl="http://www.w3.org/2002/07/owl#"
	xmlns:RIMDatatype="http://veggente.berlios.de/ns/RIMDatatype#"
	xmlns:protege="http://protege.stanford.edu/plugins/owl/protege#"
	xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#">
	<xsl:template match="/tbody">
			<rdf:RDF>
					<owl:Ontology rdf:about="http://veggente.berlios.de/ns/RIMDatatype"/>
					<xsl:apply-templates select="tr"/>
			</rdf:RDF>
	</xsl:template>
	<xsl:template match="tr">
			<owl:Class>
					<xsl:attribute name="rdf:resource">
							<xsl:value-of select="td[2]"/>
					</xsl:attribute>
					<rdfs:comment>
							<xsl:value-of select="td[3]"/>
					</rdfs:comment>
			</owl:Class>
	</xsl:template>

</xsl:stylesheet>
