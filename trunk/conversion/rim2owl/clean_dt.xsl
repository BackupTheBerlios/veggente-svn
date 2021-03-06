<?xml version="1.0"?>
<!-- 
	Transform HL7 Reference Information Model (RIM) Datatypes as modeled by 
	Samson Tu (Stanford University) into a W3C OWL ontology compatible with RIM
	model generated by rim2owl.xsl
     
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
	xmlns:hl7_dt="http://veggente.berlios.de/ns/RIMDatatype#"
	xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
	xmlns:owl="http://www.w3.org/2002/07/owl#"
	xmlns:RIMDatatype="http://veggente.berlios.de/ns/RIMDatatype#"
	xmlns:protege="http://protege.stanford.edu/plugins/owl/protege#"
	xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#">
	<xsl:output method="xml" indent="yes" encoding="UTF-8"/>
	<xsl:param name="rim_ns" select="'http://veggente.berlios.de/ns/RIMOntology'"/>
	<xsl:param name="rim_dt" select="'http://veggente.berlios.de/ns/RIMDatatype'"/>
	<!-- WARNING: 
		"<" and ">" are converted into "_". RDF doesn't admin such characters in attribute text
		Functional Property "Concept" is defined but not used. Concepts are represented by annotations
	-->
	<xsl:template match="/rdf:RDF">
			<xsl:comment>
	Conversion of HL7 Reference Information Model (RIM) Datatypes as modeled by 
	Samson Tu (Stanford University) into a W3C OWL ontology compatible with RIM
	model generated by rim2owl.xsl
     
     Copyright(c) 2005 Alessio Carenini &lt;carenini@gmail.com&gt;

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
					<xsl:copy-of select="owl:*"/>
					<xsl:apply-templates select="hl7_dt:HL7_Data_Type"/>
					<xsl:apply-templates select="hl7_dt:HL7_Data_Value"/>
					<xsl:apply-templates select="hl7_dt:TopLevel_Class"/>
			</rdf:RDF>
	</xsl:template>
	<xsl:template match="hl7_dt:alias">
			<owl:Class>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="concat($rim_dt,'#')"/><xsl:value-of select="translate(.,'&lt;&gt;','__')"/>
					</xsl:attribute>
					<owl:equivalentClass>
							<xsl:attribute name="rdf:resource" select="concat($rim_dt,../@rdf:about)"/>
					</owl:equivalentClass>
			</owl:Class>
	</xsl:template>
	<xsl:template match="hl7_dt:TopLevel_Class">
			<owl:Class>
					<xsl:attribute name="rdf:about" select="@rdf:about"/>
					<rdfs:subClassOf>
							<xsl:attribute name="rdf:resource" select="concat($rim_dt,@rdf:about)"/>
					</rdfs:subClassOf>
			</owl:Class>
	</xsl:template>
	<xsl:template match="hl7_dt:HL7_Data_Value">
			<owl:Class>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="concat($rim_dt,'#')"/><xsl:value-of select="substring-after(translate(@rdf:about,'&lt;&gt;','__'),'#')"/>
					</xsl:attribute>
					<rdfs:subClassOf>
							<xsl:attribute name="rdf:resource" select="concat($rim_dt,'#HL7_Data_Value')"/>
					</rdfs:subClassOf>
					<xsl:if test="hl7_dt:concept">
							<rdfs:comment>
									<xsl:value-of select="hl7_dt:concept"/>
							</rdfs:comment>
					</xsl:if>
					<xsl:copy-of select="owl:*"/>
					<xsl:copy-of select="rdf:*"/>
					<xsl:copy-of select="rdfs:*"/>
			</owl:Class>
	</xsl:template>
	<xsl:template match="hl7_dt:HL7_Data_Type">
			<owl:Class>
					<xsl:attribute name="rdf:about">
							<xsl:value-of select="concat($rim_dt,'#')"/><xsl:value-of select="substring-after(translate(@rdf:about,'&lt;&gt;','__'),'#')"/>
					</xsl:attribute>
					<rdfs:subClassOf>
							<xsl:attribute name="rdf:resource" select="concat($rim_dt,'#HL7_Data_Type')"/>
					</rdfs:subClassOf>
					<xsl:copy-of select="owl:*"/>
					<xsl:copy-of select="rdf:*"/>
					<xsl:copy-of select="rdfs:*"/>
			</owl:Class>
			<xsl:apply-templates select="hl7_dt:alias"/>
	</xsl:template>
</xsl:stylesheet>

