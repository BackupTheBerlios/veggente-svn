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
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
	xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#"
	xmlns:owl="http://www.w3.org/2002/07/owl#">
	<xsl:output method="xml" indent="yes" encoding="UTF-8"/>
	<xsl:param name="rim_ns" select="'http://veggente.berlios.de/ns/RIMOntology'"/>
	<xsl:param name="rim_cm" select="'http://veggente.berlios.de/ns/RIM_CMET'"/>
	<xsl:variable name="rim_id" select="/hl7:serializedStaticModel/hl7:derivationSupplier/hl7:targetStaticModel[@artifact='RIM']/../@staticModelDerivationId"/>
	
	<xsl:template match="/">
			<rdf:RDF>
					<xsl:apply-templates select="hl7:serializedStaticModel"/>
			</rdf:RDF>
	</xsl:template>
	<xsl:template match="hl7:serializedStaticModel">
			<owl:Ontology>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="$rim_cm"/>
					</xsl:attribute>
					<rdfs:label><xsl:value-of select="hl7:ownedEntryPoint/hl7:specializedClass/hl7:class/@name"/></rdfs:label>
					<rdfs:comment><xsl:value-of select="hl7:ownedEntryPoint/hl7:annotations/hl7:description"/></rdfs:comment>	
					<owl:imports>
							<xsl:attribute name="rdf:resource" select="$rim_ns"/>
					</owl:imports>
			</owl:Ontology>
			<owl:AnnotationProperty rdf:about="&rdfs;comment"/>
			<xsl:apply-templates select="hl7:ownedEntryPoint"/>
	</xsl:template>
	<xsl:template match="hl7:ownedEntryPoint">
			<xsl:apply-templates select="hl7:specializedClass"/>
	</xsl:template>
	<xsl:template match="hl7:specializedClass">
			<xsl:apply-templates/>
	</xsl:template>
	<!-- Since we haven't any information about RMIMs or DMIMs we can only link to classes in RIM -->
	<!-- TODO: Class naming conventions -->
	<xsl:template match="hl7:class">
			<owl:Class>
					<xsl:attribute name="rdf:ID">
							<xsl:value-of select="$rim_cm"/>#<xsl:value-of select="@name"/>
					</xsl:attribute>
					<rdfs:subClassOf>
							<xsl:if test="$rim_id">
									<xsl:attribute name="rdf:resource">
											<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="hl7:derivationSupplier[@staticModelDerivationId=$rim_id]/@className"/>
									</xsl:attribute>
							</xsl:if>
					</rdfs:subClassOf>
					<xsl:apply-templates select="hl7:attribute"/>
					<xsl:apply-templates select="hl7:association"/>
			</owl:Class>
	</xsl:template>
	<xsl:template match="hl7:attribute">
			<xsl:if test="hl7:derivationSupplier[staticModelDerivationId=$rim_id]">
					<rdfs:subClassOf>
							<owl:Restriction>
									<owl:onProperty>
											<xsl:attribute name="rdf:resource">
													<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="hl7:derivationSupplier[staticModelDerivationId=$rim_id]/@attributeName">
											</xsl:attribute>
									</owl:onProperty>
							</owl:Restriction>
					</rdfs:subClassOf>
			</xsl:if>
	</xsl:template>
	<xsl:template match="hl7:association">
			<rdfs:subClassOf>
					<owl:Restriction>
							<owl:onProperty>
									<xsl:attribute name="rdf:resource">
									</xsl:attribute>
							</owl:onProperty>
					</owl:Restriction>
			</rdfs:subClassOf>
	</xsl:template>
</xsl:stylesheet>
