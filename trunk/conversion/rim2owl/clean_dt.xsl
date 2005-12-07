<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:hl7_dt="http://veggente.berlios.de/ns/RIMDatatype#"
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
	xmlns:owl="http://www.w3.org/2002/07/owl#">
	<xsl:output method="xml" indent="yes" encoding="UTF-8"/>
	<xsl:param name="rim_ns" select="'http://veggente.berlios.de/ns/RIMOntology'"/>
	<xsl:param name="rim_dt" select="'http://veggente.berlios.de/ns/RIMDatatype'"/>
	<xsl:param name="rim_st" select="'http://veggente.berlios.de/ns/RIMSimpletype'"/>
	<!-- TODO 
		List/Vector/Bag types defined in a UML/C++ style are not processed
	-->
	<xsl:template match="/rdf:RDF">
			<rdf:RDF>
					<owl:Ontology>
							<xsl:attribute name="rdf:about" select="$rim_st"/>
							<owl:imports>
									<xsl:attribute name="rdf:resource" select="$rim_dt"/>
							</owl:imports>
					</owl:Ontology>
					<xsl:apply-templates select="hl7_dt:HL7_Data_Type/hl7_dt:alias"/>
			</rdf:RDF>
	</xsl:template>
	<xsl:template match="hl7_dt:alias">
			<xsl:if test="not(contains(.,'>'))">
					<owl:Class>
							<xsl:attribute name="rdf:ID" select="."/>
							<owl:equivalentClass>
									<xsl:attribute name="rdf:resource" select="concat($rim_dt,../@rdf:about)"/>
							</owl:equivalentClass>
					</owl:Class>
			</xsl:if>
	</xsl:template>
</xsl:stylesheet>

