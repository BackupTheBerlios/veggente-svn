<?xml version="1.0"?>
<!-- 
	Transform HL7 Reference Information Model (RIM) expressed in MIF format
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
	xmlns:xsd="http://www.w3.org/2001/XMLSchema#"
	xmlns:hl7="urn:hl7-org:v3/mif"
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
	xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#"
	xmlns:owl="http://www.w3.org/2002/07/owl#">
	<xsl:import href="datatype_generator.xsl"/>
	<xsl:output method="xml" indent="yes" encoding="UTF-8"/>
	<xsl:param name="rim_ns" select="'http://veggente.berlios.de/ns/RIMOntology'"/>
	<xsl:param name="rim_dt" select="'http://veggente.berlios.de/ns/RIMDatatype'"/>
	<xsl:template match="/">
			<xsl:comment>
	 Conversion of HL7 Reference Information Model into an OWL ontology
					
	 Copyright(c) 2006 Alessio Carenini &lt;carenini@gmail.com&gt;

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
			</xsl:comment>
			<rdf:RDF>
					<xsl:apply-templates select="hl7:staticModel"/>
			</rdf:RDF>
	</xsl:template>
	<xsl:template match="hl7:staticModel">
			<owl:Ontology>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="$rim_ns"/>
					</xsl:attribute>
					<rdfs:label>HL7 RIM ontology</rdfs:label>
					<rdfs:comment>	
							<xsl:value-of select="hl7:historyItem/hl7:description"/>
					</rdfs:comment>
					<owl:imports>
							<xsl:attribute name="rdf:resource"><xsl:value-of select="$rim_dt"/></xsl:attribute>
					</owl:imports>
			</owl:Ontology>
			<owl:AnnotationProperty rdf:about="&rdfs;comment"/>
			<owl:Class>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="concat($rim_ns,'#')"/><xsl:value-of select="'RIM_HL7'"/>
					</xsl:attribute>
			</owl:Class>
			<xsl:apply-templates select="hl7:ownedSubjectAreaPackage"/>
			<xsl:apply-templates select="hl7:ownedAssociation"/>
	</xsl:template>
	<xsl:template match="hl7:historyItem"><!-- Eliminare ed integrare nell'about dell'ontologia --></xsl:template>
	<xsl:template match="hl7:header"><!-- Informazioni su autori e revisioni --></xsl:template>
	<xsl:template match="hl7:annotations"></xsl:template>
	<xsl:template match="hl7:figure"></xsl:template>
	
	<xsl:template match="hl7:ownedSubjectAreaPackage">
			<xsl:variable name="package" select="@name"/>
			<owl:Class>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="concat($rim_ns,'#')"/><xsl:value-of select="@name"/>
					</xsl:attribute>
					<rdfs:subClassOf>
							<xsl:attribute name="rdf:resource"><xsl:value-of select="$rim_ns"/><xsl:text>#RIM_HL7</xsl:text></xsl:attribute>
					</rdfs:subClassOf>
					<xsl:if test="../../hl7:ownedSubjectAreaPackage">
							<rdfs:subClassOf>
									<xsl:attribute name="rdf:resource">
											<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="../../hl7:ownedSubjectAreaPackage/@name"/>
									</xsl:attribute>
							</rdfs:subClassOf>
					</xsl:if>
			</owl:Class>
			<xsl:apply-templates>
					<xsl:with-param name="parent" select="@name"/>
			</xsl:apply-templates>
	</xsl:template>
	
	<xsl:template match="hl7:ownedClass">
			<xsl:param name="parent"/>
			<xsl:variable name="class_name" select="@name"/>
			<xsl:apply-templates select="/hl7:staticModel/hl7:ownedClass/hl7:class[@name=$class_name]">
					<xsl:with-param name="parent" select="$parent"/>
			</xsl:apply-templates>
	</xsl:template>
	
	<!-- RIM Association -->
	<xsl:template match="hl7:ownedAssociation">
			<owl:ObjectProperty>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="concat($rim_ns,'#')"/><xsl:value-of select="hl7:connections/hl7:traversableConnection[1]/@participantClassName"/>.<xsl:value-of select="hl7:connections/hl7:traversableConnection[1]/@name"/>
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
							<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="hl7:connections/hl7:traversableConnection[2]/@participantClassName"/>.<xsl:value-of select="hl7:connections/hl7:traversableConnection[2]/@name"/>
					</xsl:attribute>
					<owl:inverseOf>
							<xsl:attribute name="rdf:resource">
									<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="hl7:connections/hl7:traversableConnection[1]/@participantClassName"/>.<xsl:value-of select="hl7:connections/hl7:traversableConnection[1]/@name"/>
							</xsl:attribute>
					</owl:inverseOf>
			</owl:ObjectProperty>
	</xsl:template>
<!-- RIM Class -->
	<xsl:template match="hl7:class">
			<xsl:param name="parent"/>
			<xsl:variable name="class_name" select="@name"/>
			<owl:Class>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="concat($rim_ns,'#')"/><xsl:value-of select="@name"/>
					</xsl:attribute>
					<xsl:call-template name="add_info"/>
					<xsl:for-each select="/hl7:staticModel/hl7:ownedClass/hl7:class">
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
					<xsl:if test="$parent">
							<rdfs:subClassOf>
									<xsl:attribute name="rdf:resource">
											<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="$parent"/>
									</xsl:attribute>
							</rdfs:subClassOf>
					</xsl:if>
					<xsl:apply-templates select="hl7:attribute" mode="constraints"/>
					<xsl:apply-templates select="../../hl7:ownedAssociation/hl7:connections/hl7:traversableConnection[@participantClassName=$class_name]" mode="constraints"/>
			</owl:Class>
			<xsl:apply-templates select="hl7:attribute" mode="class"/>
	</xsl:template>
<!-- Class attribute-->
	<xsl:template match="hl7:attribute" mode="class">
			<owl:ObjectProperty>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="concat($rim_ns,'#')"/><xsl:value-of select="../@name"/>.<xsl:value-of select="@name"/>
					</xsl:attribute>
					<xsl:call-template name="add_info"/>
					<rdfs:range>
							<xsl:choose>
									<xsl:when test="hl7:type/hl7:supplierBindingArgumentDatatype">
											<xsl:attribute name="rdf:resource">
													<xsl:value-of select="$rim_dt"/>#<xsl:value-of select="concat(hl7:type/@name,'_')"/><xsl:value-of select="hl7:type/hl7:supplierBindingArgumentDatatype/@name"/>
											</xsl:attribute>
									</xsl:when>
									<xsl:otherwise>
											<xsl:attribute name="rdf:resource">
													<xsl:value-of select="$rim_dt"/>#<xsl:value-of select="hl7:type/@name"/>
											</xsl:attribute>
									</xsl:otherwise>
							</xsl:choose>
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
											<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="../@name"/>.<xsl:value-of select="@name"/>
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
													<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="../@name"/>.<xsl:value-of select="@name"/>
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
											<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@participantClassName"/>.<xsl:value-of select="@name"/>
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
													<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@participantClassName"/>.<xsl:value-of select="@name"/>
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
	
