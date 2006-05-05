<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
	xmlns:owl="http://www.w3.org/2002/07/owl#"
	xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#">
	<xsl:output method="text" indent="yes" encoding="UTF-8"/>
	<xsl:param name="rim_ns" select="'http://veggente.berlios.de/ns/RIMOntology'"/>
	<xsl:param name="rim_dt" select="'http://veggente.berlios.de/ns/RIMDatatype'"/>

	<xsl:template match="/dependencies">
			<xsl:for-each select="dep[not(.=preceding-sibling::dep)]">
					<xsl:value-of select="."/><xsl:text> </xsl:text>
			</xsl:for-each>
	</xsl:template>
</xsl:stylesheet>



