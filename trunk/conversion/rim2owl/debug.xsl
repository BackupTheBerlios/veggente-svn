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
	<xsl:param name="rim_ns" select="'http://veggente.berlios.de/ns/RIMOntology'"/>
	<xsl:param name="rim_dt" select="'http://veggente.berlios.de/ns/RIMDatatype'"/>
	<xsl:param name="rim_voc" select="'http://veggente.berlios.de/ns/RIMVocabulary'"/>
	<xsl:template match="/rdf:RDF">
			<output>
					<xsl:apply-templates select="owl:Class/rdfs:subClassOf/owl:Restriction" mode="restriction"/>
					<xsl:apply-templates select="owl:ObjectProperty" mode="restriction"/>
			</output>
	</xsl:template>

	<xsl:template match="owl:ObjectProperty" mode="restriction">
			<xsl:variable name="range" select="substring-after(rdfs:range/@rdf:resource,'#')"/>
			<xsl:variable name="domain" select="substring-after(rdfs:domain/@rdf:resource,'#')"/>
			<xsl:if test="($range)or($domain)">
					<xsl:variable name="range_result">
							<xsl:choose>
									<xsl:when test="(/rdf:RDF/owl:Class[@rdf:ID=$range]) or (document($rim_dt)//rdf:RDF/owl:Class[@rdf:ID=$range])">OK</xsl:when>
									<xsl:when test="not($range)">NOT_PRESENT</xsl:when>
									<xsl:otherwise>FAILED</xsl:otherwise>
							</xsl:choose>
					</xsl:variable>
					<xsl:variable name="domain_result">
							<xsl:choose>
									<xsl:when test="(/rdf:RDF/owl:Class[@rdf:ID=$domain])or(document($rim_dt)//rdf:RDF/owl:Class[@rdf:ID=$domain])">OK</xsl:when>
									<xsl:when test="not($domain)">NOT_PRESENT</xsl:when>
									<xsl:otherwise>FAILED</xsl:otherwise>
							</xsl:choose>
					</xsl:variable>
					<xsl:variable name="remote_domain_result">
							<xsl:for-each select="/rdf:RDF/owl:Ontology/owl:imports">
									<xsl:variable name="doc_name">
											<xsl:value-of select="./@rdf:resource"/>
									</xsl:variable>
									<xsl:if test="document($doc_name)//rdf:RDF/owl:Class[@rdf:ID=$domain]">OK</xsl:if>
							</xsl:for-each>
					</xsl:variable>
					<xsl:variable name="remote_range_result">
							<xsl:for-each select="/rdf:RDF/owl:Ontology/owl:imports">
									<xsl:variable name="doc_name">
											<xsl:value-of select="./@rdf:resource"/>
									</xsl:variable>
									<xsl:if test="document($doc_name)//rdf:RDF/owl:Class[@rdf:ID=$range]">OK</xsl:if>
							</xsl:for-each>
					</xsl:variable>
					<xsl:if test="(($domain_result='FAILED')and(not(contains($remote_domain_result,'OK')))) or (($range_result='FAILED')and(not(contains($remote_range_result,'OK'))))">
							<objectProperty>
									<xsl:attribute name="name">
											<xsl:value-of select="./@rdf:ID"/>
									</xsl:attribute>
									<domain>
											<xsl:attribute name="name">
													<xsl:value-of select="$domain"/>
											</xsl:attribute>
											<xsl:attribute name="status">
													<xsl:value-of select="$domain_result"/>
											</xsl:attribute>
									</domain>
									<range>
											<xsl:attribute name="name">
													<xsl:value-of select="$range"/>
											</xsl:attribute>
											<xsl:attribute name="status">
													<xsl:value-of select="$range_result"/>
											</xsl:attribute>
									</range>
							</objectProperty>
					</xsl:if>
			</xsl:if>
	</xsl:template>
	<xsl:template match="owl:Restriction" mode="restriction">
			<xsl:variable name="property">
					<xsl:value-of select="../..//@rdf:ID"/>.<xsl:value-of select="substring-after(substring-after(owl:onProperty/@rdf:resource,'#'),'.')"/>
			</xsl:variable>
			<xsl:variable name="property_result">
					<xsl:choose>
							<xsl:when test="(/rdf:RDF/owl:ObjectProperty[@rdf:ID=$property])or(/rdf:RDF/owl:DatatypeProperty[@rdf:ID=$property])">OK</xsl:when>
							<xsl:when test="(document($rim_dt)//rdf:RDF/owl:ObjectProperty[@rdf:ID=$property])or(document($rim_dt)//rdf:RDF/owl:DatatypeProperty[@rdf:ID=$property])">OK</xsl:when>
							<xsl:otherwise>FAILED</xsl:otherwise>
					</xsl:choose>
			</xsl:variable>
			<xsl:variable name="remote_property_result">
					<xsl:for-each select="/rdf:RDF/owl:Ontology/owl:imports">
							<xsl:variable name="doc_name">
									<xsl:value-of select="./@rdf:resource"/>
							</xsl:variable>
							<xsl:if test="(document($doc_name)//rdf:RDF/owl:ObjectProperty[@rdf:ID=$property])or(document($doc_name)//rdf:RDF/owl:DatatypeProperty[@rdf:ID=$property])">OK</xsl:if>
					</xsl:for-each>
			</xsl:variable>
			<xsl:if test="($property_result='FAILED') and (not(contains($remote_property_result,'OK')))">
					<objectRestriction>
							<xsl:attribute name="property">
									<xsl:value-of select="$property"/>
							</xsl:attribute>
							<xsl:attribute name="status">
									<xsl:value-of select="$property_result"/>
							</xsl:attribute>
					</objectRestriction>
			</xsl:if>
	</xsl:template>
</xsl:stylesheet>
