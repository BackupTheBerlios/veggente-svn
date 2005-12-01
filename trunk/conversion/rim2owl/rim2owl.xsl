<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:xsd="http://www.w3.org/2001/XMLSchema"
	xmlns:hl7="urn:hl7-org:v3/mif"
	xmlns:dc="http://purl.org/dc/elements/1.1/"
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
	xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#"
	xmlns:owl="http://www.w3.org/2002/07/owl#">
	<xsl:output method="xml" indent="yes" encoding="UTF-8"/>
	<xsl:template match="/">
			<rdf:RDF>
					<xsl:apply-templates select="hl7:staticModel"/>
			</rdf:RDF>
	</xsl:template>
	<xsl:template match="hl7:staticModel">
			<owl:Ontology>
					<rdfs:comment>Translated HL7 RIM ontology</rdfs:comment>	
					<rdfs:label>HL7 RIM ontology</rdfs:label>
					<dc:description>
							<xsl:value-of select="hl7:historyItem/hl7:description"/>
					</dc:description>
			</owl:Ontology>
			<xsl:apply-templates/>
	</xsl:template>
	<xsl:template match="hl7:historyItem"><!-- Eliminare ed integrare nell'about dell'ontologia --></xsl:template>
	<xsl:template match="hl7:header"><!-- Informazioni su autori e revisioni --></xsl:template>
	<xsl:template match="hl7:annotations"></xsl:template>
	<xsl:template match="hl7:figure"></xsl:template>
	<xsl:template match="hl7:ownedClass">
			<xsl:apply-templates/>	
	</xsl:template>
	<!-- RIM Association -->
	<xsl:template match="hl7:ownedAssociation">
			<xsl:apply-templates/>
	</xsl:template>
	<!-- RIM Class -->
	<xsl:template match="hl7:class">
			<owl:Class>
					<xsl:attribute name="rdf:ID"><xsl:value-of select="@name"/></xsl:attribute>
			</owl:Class>
			<xsl:apply-templates/>
	</xsl:template>
	<xsl:template match="hl7:attribute">
			<owl:ObjectProperty>
					<xsl:attribute name="about">
							<xsl:value-of select="@name"/>
					</xsl:attribute>
					<rdfs:range>
							<xsl:attribute name="rdf:resource">
									<xsl:value-of select="hl7:type/@name"/>
							</xsl:attribute>
					</rdfs:range>
					<rdfs:domain>
							<xsl:attribute name="rdf:resource">
									<xsl:text>#</xsl:text><xsl:value-of select="../@name"/>
							</xsl:attribute>
					</rdfs:domain>
			</owl:ObjectProperty>
	</xsl:template>
</xsl:stylesheet>
	
