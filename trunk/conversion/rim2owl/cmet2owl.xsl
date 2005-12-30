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
	xmlns:owl="http://www.w3.org/2002/07/owl#"
	xmlns:cmet="http://veggente.berlios.de/ns/RIM_CMET#">
	<xsl:output method="xml" indent="yes" encoding="UTF-8"/>
	<xsl:param name="rim_ns" select="'http://veggente.berlios.de/ns/RIMOntology'"/>
	<xsl:param name="rim_cm" select="'http://veggente.berlios.de/ns/cmet/'"/>
	<!-- ID which identifies RIM in import declarations-->
	<xsl:variable name="rim_id" select="/hl7:serializedStaticModel/hl7:derivationSupplier/hl7:targetStaticModel[@artifact='RIM']/../@staticModelDerivationId"/>
	<!-- Code of the selected CMET -->
	<xsl:variable name="cmet_name"><xsl:value-of select="concat(/hl7:serializedStaticModel/hl7:packageLocation/@subSection,/hl7:serializedStaticModel/hl7:packageLocation/@domain)"/>_<xsl:value-of select="substring-before(/hl7:serializedStaticModel/hl7:packageLocation/@artifact,'-')"/><xsl:value-of select="/hl7:serializedStaticModel/@name"/><xsl:value-of select="/hl7:serializedStaticModel/hl7:packageLocation/@realm"/><xsl:value-of select="/hl7:serializedStaticModel/hl7:packageLocation/@version"/></xsl:variable>
	
	<xsl:template match="/">
			<rdf:RDF>
					<xsl:apply-templates select="hl7:serializedStaticModel"/>
			</rdf:RDF>
	</xsl:template>
	<xsl:template match="hl7:serializedStaticModel">
			<owl:Ontology>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="$rim_cm"/><xsl:value-of select="$cmet_name"/>
					</xsl:attribute>
					<rdfs:label><xsl:value-of select="hl7:ownedEntryPoint/hl7:specializedClass/hl7:class/@name"/></rdfs:label>
					<rdfs:comment><xsl:value-of select="hl7:ownedEntryPoint/hl7:annotations/hl7:description"/></rdfs:comment>	
					<xsl:for-each select="hl7:derivationSupplier">
							<xsl:choose>
									<xsl:when test="hl7:targetStaticModel[@artifact='RIM']">
											<owl:imports>
													<xsl:attribute name="rdf:resource" select="$rim_ns"/>
											</owl:imports>
									</xsl:when>
									<xsl:when test="contains(hl7:targetStaticModel/@artifact,'RM')"/>
									<xsl:otherwise>
											<owl:imports>
													<xsl:attribute name="rdf:resource">
															<xsl:value-of select="$rim_cm"/><xsl:call-template name="get_derivation_name">
																	<xsl:with-param name="id" select="@staticModelDerivationId"/>
															</xsl:call-template>
													</xsl:attribute>
											</owl:imports>
									</xsl:otherwise>
							</xsl:choose>
					</xsl:for-each>
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
	<!-- TODO: Class naming conventions 
		Model attributes as NEW ObjectProperties and then use restrictions on them
	-->
	<xsl:template match="hl7:class">
			<owl:Class>
					<xsl:attribute name="rdf:ID">
							<xsl:value-of select="$cmet_name"/>.<xsl:value-of select="@name"/>
					</xsl:attribute>
					<xsl:for-each select="hl7:derivationSupplier">
							<xsl:variable name="derivation_id" select="@staticModelDerivationId"/>
							<xsl:variable name="derivator">
									<xsl:copy-of select="/hl7:serializedStaticModel/hl7:derivationSupplier[@staticModelDerivationId=$derivation_id]"/>
							</xsl:variable>
							<xsl:choose>
									<xsl:when test="$derivation_id=$rim_id">
											<rdfs:subClassOf>
													<xsl:attribute name="rdf:resource">
															<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@className"/>
													</xsl:attribute>
											</rdfs:subClassOf>
									</xsl:when>
									<xsl:when test="contains($derivator//hl7:targetStaticModel/@artifact,'RM')"/>
									<xsl:otherwise>
											<rdfs:subClassOf>
													<xsl:attribute name="rdf:resource">
															<xsl:value-of select="$rim_cm"/>
															<xsl:call-template name="get_derivation_name">
																	<xsl:with-param name="id" select="$derivation_id"/>
															</xsl:call-template>#<xsl:call-template name="get_derivation_name">
																	<xsl:with-param name="id" select="$derivation_id"/>
															</xsl:call-template>.<xsl:value-of select="@className"/>
													</xsl:attribute>
											</rdfs:subClassOf>
									</xsl:otherwise>
							</xsl:choose>
					</xsl:for-each>
					<xsl:apply-templates select="hl7:annotations"/>
					<xsl:apply-templates select="hl7:attribute" mode="class"/>
			</owl:Class>
			<xsl:apply-templates select="hl7:attribute" mode="property"/>
			<xsl:apply-templates select="hl7:association"/>
	</xsl:template>

	<xsl:template match="hl7:annotations">
			<rdfs:comment><xsl:value-of select="."/></rdfs:comment>
	</xsl:template>

	<!-- TODO: ObjectProperties are associations between CLASSES! -->
	<xsl:template match="hl7:attribute" mode="class">
			<xsl:variable name="parent_ns">
					<xsl:value-of select="$rim_cm"/><xsl:value-of select="$cmet_name"/>#<xsl:value-of select="$cmet_name"/>.<xsl:value-of select="@name"/>
			</xsl:variable>
			<xsl:if test="@fixedValue">
					<rdfs:subClassOf>
							<owl:Restriction>
									<owl:onProperty>
											<xsl:attribute name="rdf:resource" select="$parent_ns"/>
									</owl:onProperty>
									<owl:hasValue>
											<xsl:attribute name="rdf:datatype">
													<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="hl7:type/@name"/>
											</xsl:attribute>
											<xsl:value-of select="@fixedValue"/>
									</owl:hasValue>
							</owl:Restriction>
					</rdfs:subClassOf>
			</xsl:if>
			<xsl:if test="@minimumMultiplicity">
					<rdfs:subClassOf>
							<owl:Restriction>
									<owl:onProperty>
											<xsl:attribute name="rdf:resource" select="$parent_ns"/>
									</owl:onProperty>
									<owl:minCardinality rdf:datatype="&xsd;nonNegativeInteger"><xsl:value-of select="@minimumMultiplicity"/></owl:minCardinality>
							</owl:Restriction>
					</rdfs:subClassOf>
			</xsl:if>
			<xsl:if test="not(contains(@maximumMultiplicity,'*'))">
					<rdfs:subClassOf>
							<owl:Restriction>
									<owl:onProperty>
											<xsl:attribute name="rdf:resource" select="$parent_ns"/>
									</owl:onProperty>
									<owl:maxCardinality rdf:datatype="&xsd;nonNegativeInteger"><xsl:value-of select="@maximumMultiplicity"/></owl:maxCardinality>
							</owl:Restriction>
					</rdfs:subClassOf>
			</xsl:if>
	</xsl:template>

	<xsl:template match="hl7:attribute" mode="property">
			<owl:ObjectProperty>
					<xsl:attribute name="rdf:ID">
							<xsl:value-of select="$cmet_name"/>.<xsl:value-of select="../@name"/>.<xsl:value-of select="@name"/>
					</xsl:attribute>
					<xsl:for-each select="hl7:derivationSupplier">
							<xsl:variable name="derivation_id" select="@staticModelDerivationId"/>
							<xsl:variable name="derivator">
									<xsl:copy-of select="/hl7:serializedStaticModel/hl7:derivationSupplier[@staticModelDerivationId=$derivation_id]"/>
							</xsl:variable>
							<xsl:choose>
									<xsl:when test="$derivation_id=$rim_id">
											<rdfs:subPropertyOf>
													<xsl:attribute name="rdf:resource">
															<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="@attributeName"/>
													</xsl:attribute>
											</rdfs:subPropertyOf>
									</xsl:when>
									<xsl:when test="contains($derivator//hl7:targetStaticModel/@artifact,'RM')"/>
									<xsl:otherwise>
											<rdfs:subPropertyOf>
													<xsl:attribute name="rdf:resource">
															<xsl:value-of select="$rim_cm"/>
															<xsl:call-template name="get_derivation_name">
																	<xsl:with-param name="id" select="$derivation_id"/>
															</xsl:call-template>#<xsl:call-template name="get_derivation_name">
																	<xsl:with-param name="id" select="$derivation_id"/>
															</xsl:call-template>.<xsl:value-of select="@className"/>.<xsl:value-of select="@attributeName"/>
													</xsl:attribute>
											</rdfs:subPropertyOf>
									</xsl:otherwise>
							</xsl:choose>
					</xsl:for-each>
			</owl:ObjectProperty>
	</xsl:template>

	<xsl:template match="hl7:association">
			<owl:ObjectProperty>
					<xsl:attribute name="rdf:ID">
							<xsl:value-of select="$cmet_name"/>.<xsl:value-of select="hl7:targetConnection/@name"/>
					</xsl:attribute>
					<rdfs:range>
							<xsl:attribute name="rdf:resource">
									<xsl:choose>
											<xsl:when test="hl7:targetConnection/hl7:participantClass/hl7:class">
													<xsl:value-of select="concat($rim_cm,$cmet_name)"/>#<xsl:value-of select="$cmet_name"/>.<xsl:value-of select="hl7:targetConnection/hl7:participantClass/hl7:class/@name"/>
											</xsl:when>
											<xsl:when test="hl7:targetConnection/hl7:participantClass/hl7:reference">
													<xsl:value-of select="concat($rim_cm,$cmet_name)"/>#<xsl:value-of select="$cmet_name"/>.<xsl:value-of select="hl7:targetConnection/hl7:participantClass/hl7:reference/@name"/>
											</xsl:when>
											<xsl:otherwise/>
									</xsl:choose>
							</xsl:attribute>
					</rdfs:range>
					<rdfs:domain>
							<xsl:attribute name="rdf:resource">
									<xsl:value-of select="concat($rim_cm,$cmet_name)"/>#<xsl:value-of select="$cmet_name"/>.<xsl:value-of select="hl7:sourceConnection/hl7:nonTraversableConnection/@participantClassName"/>
							</xsl:attribute>
					</rdfs:domain>
					<rdfs:subPropertyOf>
							<xsl:attribute name="rdf:resource">
									<xsl:value-of select="$rim_ns"/>#<xsl:value-of select="hl7:targetConnection/hl7:derivationSupplier[@staticModelDerivationId=$rim_id]/@associationEndName"/>
							</xsl:attribute>
					</rdfs:subPropertyOf>

			</owl:ObjectProperty>
			<xsl:apply-templates select="hl7:targetConnection/hl7:participantClass/hl7:class"/>
	</xsl:template>

	<xsl:template name="get_derivation_name">
			<xsl:param name="id"/>
			<xsl:variable name="selected_id">
					<xsl:copy-of select="/hl7:serializedStaticModel/hl7:derivationSupplier[@staticModelDerivationId=$id]/hl7:targetStaticModel"/>
			</xsl:variable>
			<xsl:value-of select="$selected_id//@subSection"/><xsl:value-of select="$selected_id//@domain"/>_<xsl:value-of select="substring-before($selected_id//@artifact,'-')"/><xsl:value-of select="$selected_id//@id"/><xsl:value-of select="$selected_id//@realm"/><xsl:value-of select="$selected_id//@version"/>
	</xsl:template>

</xsl:stylesheet>
