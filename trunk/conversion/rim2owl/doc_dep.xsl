<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
	xmlns:owl="http://www.w3.org/2002/07/owl#"
	xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#">
	<xsl:output method="xml" indent="yes" encoding="UTF-8"/>
	<xsl:param name="rim_ns" select="'http://veggente.berlios.de/ns/RIMOntology'"/>
	<xsl:param name="rim_dt" select="'http://veggente.berlios.de/ns/RIMDatatype'"/>

	<xsl:template match="/rdf:RDF">
			<dependencies>
					<xsl:apply-templates select="owl:Ontology/owl:imports"/>
			</dependencies>
	</xsl:template>
	<xsl:template match="owl:imports">
			<xsl:variable name="doc_name" select="@rdf:resource"/>
			<dep><xsl:value-of select="$doc_name"/></dep>
			<xsl:apply-templates select="(document($doc_name))//rdf:RDF/owl:Ontology/owl:imports"/>					
	</xsl:template>
</xsl:stylesheet>


