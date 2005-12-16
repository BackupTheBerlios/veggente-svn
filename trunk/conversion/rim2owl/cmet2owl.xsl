<?xml version="1.0"?>
<!-- 
	Transform HL7 Common Message Element Type expressed in MIF format
	into a W3C OWL ontology
     
     Copyright(c) 2005 Alessio Carenini <carenini@gmail.com>     

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.
     
     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

-->
<!DOCTYPE rdf:RDF [
	<!ENTITY rdf "http://www.w3.org/1999/02/22-rdf-syntax-ns#">
	<!ENTITY rdfs "http://www.w3.org/2000/01/rdf-schema#">
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
	<xsl:param name="rim_cm" select="'http://veggente.berlios.de/ns/RIM_CMET'"/>
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
					<rdfs:comment>CMET translated into OWL</rdfs:comment>	
					<rdfs:label>HL7 CMET data</rdfs:label>
					<dc:description>
							<xsl:value-of select="hl7:historyItem/hl7:description"/>
					</dc:description>
					<owl:imports>
							<xsl:attribute name="rdf:resource" select="$rim_ns"/>
					</owl:imports>
			</owl:Ontology>
			<owl:AnnotationProperty rdf:about="&rdfs;comment"/>
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
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="hl7:connections/hl7:traversableConnection[1]/@participantClassName"/>_<xsl:value-of select="hl7:connections/hl7:traversableConnection[1]/@name"/>
					</xsl:attribute>
					<xsl:call-template name="add_info"/>
					<rdfs:range>
							<xsl:attribute name="rdf:resource">
									<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="hl7:connections/hl7:traversableConnection[2]/@participantClassName"/>
							</xsl:attribute>
					</rdfs:range>
					<rdfs:domain>
							<xsl:attribute name="rdf:resource">
									<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="hl7:connections/hl7:traversableConnection[1]/@participantClassName"/>
							</xsl:attribute>
					</rdfs:domain>
			</owl:ObjectProperty>
			<owl:ObjectProperty>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="hl7:connections/hl7:traversableConnection[2]/@participantClassName"/>_<xsl:value-of select="hl7:connections/hl7:traversableConnection[2]/@name"/>
					</xsl:attribute>
					<owl:inverseOf>
							<xsl:attribute name="rdf:resource">
									<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="hl7:connections/hl7:traversableConnection[1]/@participantClassName"/>_<xsl:value-of select="hl7:connections/hl7:traversableConnection[1]/@name"/>
							</xsl:attribute>
					</owl:inverseOf>
			</owl:ObjectProperty>
	</xsl:template>
<!-- RIM Class -->
	<xsl:template match="hl7:class">
			<xsl:variable name="class_name" select="@name"/>
			<owl:Class>
					<xsl:attribute name="rdf:ID"><xsl:value-of select="@name"/></xsl:attribute>
					<xsl:call-template name="add_info"/>
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
					<xsl:apply-templates select="../../hl7:ownedAssociation/hl7:connections/hl7:traversableConnection[@participantClassName=$class_name]" mode="constraints"/>
			</owl:Class>
			<xsl:apply-templates select="hl7:attribute" mode="class"/>
	</xsl:template>
<!-- Class attribute-->
	<xsl:template match="hl7:attribute" mode="class">
			<owl:ObjectProperty>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@name"/>
					</xsl:attribute>
					<xsl:call-template name="add_info"/>
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
<!-- Class attribute constraints-->
	<xsl:template match="hl7:attribute" mode="constraints">
			<rdfs:subClassOf>
					<owl:Restriction>
							<owl:onProperty>
									<xsl:attribute name="rdf:resource">
											<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@name"/>
									</xsl:attribute>
							</owl:onProperty>
							<owl:minCardinality rdf:datatype="&xsd;nonNegativeInteger"><xsl:value-of select="@minimumMultiplicity"/></owl:minCardinality>
					</owl:Restriction>
			</rdfs:subClassOf>
			<xsl:if test="not(@maximumMultiplicity='*')">
					<rdfs:subClassOf>
							<owl:Restriction>
									<owl:onProperty>
											<xsl:attribute name="rdf:resource">
													<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@name"/>
											</xsl:attribute>
									</owl:onProperty>
									<owl:maxCardinality rdf:datatype="&xsd;nonNegativeInteger"><xsl:value-of select="@maximumMultiplicity"/></owl:maxCardinality>
							</owl:Restriction>
					</rdfs:subClassOf>
			</xsl:if>
	</xsl:template>
<!-- Associations constraints-->
	<xsl:template match="hl7:traversableConnection" mode="constraints">
			<rdfs:subClassOf>
					<owl:Restriction>
							<owl:onProperty>
									<xsl:attribute name="rdf:resource">
											<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@participantClassName"/>_<xsl:value-of select="@name"/>
									</xsl:attribute>
							</owl:onProperty>
							<owl:minCardinality rdf:datatype="&xsd;nonNegativeInteger"><xsl:value-of select="@minimumMultiplicity"/></owl:minCardinality>
					</owl:Restriction>
			</rdfs:subClassOf>
			<xsl:if test="not(@maximumMultiplicity='*')">
					<rdfs:subClassOf>
							<owl:Restriction>
									<owl:onProperty>
											<xsl:attribute name="rdf:resource">
													<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@participantClassName"/>_<xsl:value-of select="@name"/>
											</xsl:attribute>
									</owl:onProperty>
									<owl:maxCardinality rdf:datatype="&xsd;nonNegativeInteger"><xsl:value-of select="@maximumMultiplicity"/></owl:maxCardinality>
							</owl:Restriction>
					</rdfs:subClassOf>
			</xsl:if>
	</xsl:template>
<!-- Add Dublin Core info. Just for completeness... -->
	<xsl:template name="add_info">
			<!--	<xsl:if test="@name">
					<dc:identifier>
							<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@name"/>
					</dc:identifier>
			</xsl:if>-->
			<xsl:if test="hl7:annotations/hl7:definition">
					<rdfs:comment rdf:datatype="&xsd;string">
							<xsl:value-of select="hl7:annotations/hl7:definition"/>
					</rdfs:comment>
			</xsl:if>
	</xsl:template>
</xsl:stylesheet>
	
