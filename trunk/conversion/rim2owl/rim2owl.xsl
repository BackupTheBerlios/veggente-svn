<?xml version="1.0"?>
<!DOCTYPE rdf:RDF [
 <!ENTITY xsd  "http://www.w3.org/2001/XMLSchema#" >
    ]>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:xsd="http://www.w3.org/2001/XMLSchema"
	xmlns:hl7="urn:hl7-org:v3/mif"
	xmlns:dc="http://purl.org/dc/elements/1.1/"
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
	xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#"
	xmlns:owl="http://www.w3.org/2002/07/owl#">
	<xsl:output method="xml" indent="yes" encoding="UTF-8"/>
	<xsl:param name="rim_ns" select="'http://veggente.berlios.de/ns/RIMOntology'"/>
	<xsl:param name="rim_dt" select="'http://veggente.berlios.de/ns/RIMDatatype'"/>
	<xsl:template match="/">
			<rdf:RDF>
					<xsl:apply-templates select="hl7:staticModel"/>
			</rdf:RDF>
	</xsl:template>
	<xsl:template match="hl7:staticModel">
			<owl:Ontology>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="$rim_ns"/>
					</xsl:attribute>
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
			<owl:ObjectProperty>
					<xsl:call-template name="add_dcinfo"/>
					<xsl:apply-templates select="hl7:connections"/>
			</owl:ObjectProperty>
	</xsl:template>
	<!-- RIM Class -->
	<xsl:template match="hl7:class">
			<xsl:variable name="class_name" select="@name"/>
			<owl:Class>
					<xsl:attribute name="rdf:ID"><xsl:value-of select="@name"/></xsl:attribute>
					<xsl:call-template name="add_dcinfo"/>
					<xsl:for-each select="../../hl7:ownedClass/hl7:class">
							<xsl:variable name="temp_name" select="@name"/>
							<xsl:for-each select="hl7:specializationChild">
									<xsl:if test="@childClassName=$class_name">
											<rdfs:subClassOf>
													<xsl:attribute name="rdf:resource">
														<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="$temp_name"/>
													</xsl:attribute>
											</rdfs:subClassOf>
									</xsl:if>
							</xsl:for-each>
					</xsl:for-each>
					<xsl:apply-templates select="hl7:attribute" mode="constraints"/>
			</owl:Class>
			<xsl:apply-templates select="hl7:attribute" mode="class"/>
	</xsl:template>
	<xsl:template match="hl7:attribute" mode="class">
			<owl:ObjectProperty>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@name"/>
					</xsl:attribute>
					<xsl:call-template name="add_dcinfo"/>
					<rdfs:range>
							<xsl:attribute name="rdf:resource">
									<xsl:value-of select="$rim_dt"/>#<xsl:value-of select="hl7:type/@name"/>
							</xsl:attribute>
					</rdfs:range>
					<rdfs:domain>
							<xsl:attribute name="rdf:resource">
									<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="../@name"/>
							</xsl:attribute>
					</rdfs:domain>
			</owl:ObjectProperty>
	</xsl:template>
	<xsl:template match="hl7:attribute" mode="constraints">
			<rdfs:subClassOf>
					<owl:Restriction>
							<owl:onProperty>
									<xsl:attribute name="rdf:resource">
											<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@name"/>
									</xsl:attribute>
							</owl:onProperty>
							<owl:minCardinality rdf:datatype="&xsd;nonNegativeInteger"><xsl:value-of select="@minimumMultiplicity"/></owl:minCardinality>
							<owl:maxCardinality rdf:datatype="&xsd;nonNegativeInteger"><xsl:value-of select="@maximumMultiplicity"/></owl:maxCardinality>
					</owl:Restriction>
			</rdfs:subClassOf>
	</xsl:template>
	<xsl:template match="hl7:connections">
			<rdfs:range>
					<xsl:attribute name="rdf:resource">
							<xsl:value-of select="$rim_ns"/>#<xsl:value-of select=""/>
					</xsl:attribute>
			</rdfs:range>
			<rdfs:domain>
					<xsl:attribute name="rdf:resource">
							<xsl:value-of select="$rim_ns"/>#<xsl:value-of select=""/>
					</xsl:attribute>
			</rdfs:domain>
	</xsl:template>
	<xsl:template name="add_dcinfo">
			<dc:identifier>
					<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@name"/>
			</dc:identifier>
			<dc:description>
					<xsl:value-of select="hl7:annotations/hl7:definition"/>
			</dc:description>
	</xsl:template>
</xsl:stylesheet>
	
