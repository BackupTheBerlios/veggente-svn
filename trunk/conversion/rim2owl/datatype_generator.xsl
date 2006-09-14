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
	xmlns:cmet="http://veggente.berlios.de/ns/RIM_CMET#"
	xmlns:rim_dt="http://veggente.berlios.de/ns/RIMDatatype#">
	
	<xsl:output method="xml" indent="yes" encoding="UTF-8"/>
	
	<xsl:param name="rim_ns" select="'http://veggente.berlios.de/ns/RIMOntology'"/>
	<xsl:param name="rim_dt" select="'http://veggente.berlios.de/ns/RIMDatatype'"/>
	<xsl:param name="rim_cm" select="'http://veggente.berlios.de/ns/cmet/'"/>
	<xsl:param name="mif_input_path" select="'../input/MIF'"/>

	<xsl:template name="instance">
			<xsl:param name="type"/>
			<xsl:param name="value"/>
			<xsl:param name="uri"/>
			<!--			<hl7:ciao>
			<xsl:value-of select="$type"/>
	</hl7:ciao>-->
			<xsl:choose>
					<xsl:when test="$type='CS'">
							<xsl:element name="rim_dt:CS">
									<xsl:attribute name="rdf:about">
											<xsl:value-of select="$uri"/>
									</xsl:attribute>
									<rim_dt:code>
											<rim_dt:ST>
													<xsl:attribute name="rdf:about">
															<xsl:value-of select="substring-before($uri,'#')"/>#<xsl:value-of select="concat('ST',generate-id(.))"/>
													</xsl:attribute>
													<rim_dt:data rdf:datatype="http://www.w3.org/2001/XMLSchema#string">
															<xsl:value-of select="$value"/>
													</rim_dt:data>
											</rim_dt:ST>
									</rim_dt:code>
							</xsl:element>
					</xsl:when>
					<xsl:when test="$type='INT'">
							<xsl:element name="rim_dt:INT">
									<xsl:attribute name="rdf:about">
											<xsl:value-of select="$uri"/>
									</xsl:attribute>
									<rim_dt:INT.value rdf:datatype="http://www.w3.org/2001/XMLSchema#integer">
											<xsl:value-of select="$value"/>
									</rim_dt:INT.value>
							</xsl:element>
					</xsl:when>
					<xsl:otherwise/>
			</xsl:choose>
	</xsl:template>

</xsl:stylesheet>
