<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    xmlns:xsd="http://www.w3.org/2001/XMLSchema">
    <xsl:output method="xml" indent="no" encoding="UTF-8"/>
    
    <xsl:template match="/">
        <xsl:apply-templates select="xsd:schema"/>
    </xsl:template>

    <xsl:template match="xsd:schema">
        <marshal>
            <object class="XMLSchema" module="XMLSchema">
                <tuple/>
                <dictionary>
                    <xsl:element name="string">
                        <xsl:text>types</xsl:text>
                    </xsl:element>
                    <dictionary>
                        <xsl:apply-templates select="xsd:complexType[@name]"/>
                    </dictionary>
                    <xsl:choose>
                        <xsl:when test="xsd:element">
                            <xsl:element name="string">
                                <xsl:text>entry_point</xsl:text>
                            </xsl:element>
                            <xsl:apply-templates select="xsd:element"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:element name="string">
                                <xsl:text>entry_point</xsl:text>
                            </xsl:element>
                            <xsl:call-template name="fake_element">
                                <xsl:with-param name="name">
                                    <xsl:value-of select="substring-after(xsd:complexType[1]/@name,'.')"/>
                                </xsl:with-param>
                                <xsl:with-param name="type" select="xsd:complexType[1]/@name"/>
                            </xsl:call-template>
                        </xsl:otherwise>
                    </xsl:choose>
                </dictionary>
            </object>
        </marshal>
    </xsl:template>

    <xsl:template name="fake_element">
        <xsl:param name="name"/>
        <xsl:param name="type"/>
        <object class="Element" module="XMLSchema">
            <tuple/>
            <dictionary>
                <xsl:element name="string">
                    <xsl:text>name</xsl:text>
                </xsl:element>
                <xsl:element name="string">
                    <xsl:value-of select="$name"/>
                </xsl:element>
                <xsl:element name="string">
                    <xsl:text>type</xsl:text>
                </xsl:element>
                <xsl:element name="string">
                    <xsl:value-of select="$type"/>
                </xsl:element>
                <xsl:element name="string">
                    <xsl:text>nillable</xsl:text>
                </xsl:element>
                <xsl:element name="string">
                    <xsl:value-of select="'false'"/>
                </xsl:element>
            </dictionary>
        </object>
    </xsl:template>
    
    <xsl:template match="xsd:element">
        <xsl:variable name="type">
            <xsl:value-of select="@type"/>
        </xsl:variable>
        <object class="Element" module="XMLSchema">
            <tuple/>
            <dictionary>
               <xsl:choose>
                    <xsl:when test="@name">
                        <xsl:element name="string">
                            <xsl:text>name</xsl:text>
                        </xsl:element>
                        <xsl:element name="string">
                            <xsl:value-of select="@name"/>
                        </xsl:element>
                    </xsl:when>
                    <xsl:otherwise/>
                </xsl:choose>
                <xsl:choose>
                    <xsl:when test="@type">
                        <xsl:element name="string">
                            <xsl:text>type</xsl:text>
                        </xsl:element>
                        <xsl:element name="string">
                            <xsl:value-of select="@type"/>
                        </xsl:element>
                    </xsl:when>
                    <xsl:otherwise/>
                </xsl:choose>
                <xsl:choose>
                    <xsl:when test="@minOccurs">
                        <xsl:element name="string">
                            <xsl:text>minOccurs</xsl:text>
                        </xsl:element>
                        <xsl:element name="int">
                            <xsl:value-of select="@minOccurs"/>
                        </xsl:element>
                    </xsl:when>
                    <xsl:otherwise/>
                </xsl:choose>
                <xsl:choose>
                    <xsl:when test="@maxOccurs">
                        <xsl:element name="string">
                            <xsl:text>maxOccurs</xsl:text>
                        </xsl:element>
                        <xsl:element name="int">
                            <xsl:value-of select="@maxOccurs"/>
                        </xsl:element>
                    </xsl:when>
                    <xsl:otherwise/>
                </xsl:choose>
                <xsl:choose>
                    <xsl:when test="@nillable">
                        <xsl:element name="string">
                            <xsl:text>nillable</xsl:text>
                        </xsl:element>
                        <xsl:element name="string">
                            <xsl:value-of select="@nillable"/>
                        </xsl:element>
                    </xsl:when>
                    <xsl:otherwise/>
                </xsl:choose>
            </dictionary>
        </object>
    </xsl:template>

    <xsl:template match="xsd:complexType">
        <xsl:param name="name"/>
        <object class="ComplexType" module="XMLSchema">
            <tuple/>
            <dictionary>
                <xsl:choose>
                    <xsl:when test="xsd:sequence">
                        <xsl:element name="string">
                            <xsl:text>sequence</xsl:text>
                        </xsl:element>
                        <xsl:element name="list">
                            <xsl:apply-templates select="xsd:sequence/xsd:element"/>
                        </xsl:element>
                    </xsl:when>
                    <xsl:otherwise/>
                </xsl:choose>
                <xsl:element name="string"> 
                    <xsl:value-of select="'attributes'"/>
                </xsl:element>
                <xsl:element name="list">
                    <xsl:apply-templates select="xsd:attribute | xsd:attributeGroup"/>
                </xsl:element>
            </dictionary>
        </object>
    </xsl:template>

    <xsl:template match="xsd:attribute">
        <xsl:choose>
            <xsl:when test="xsd:simpleType">
            </xsl:when>
            <xsl:when test="@type">
                <object class="Attribute" module="XMLSchema">
                    <tuple/>
                    <xsl:element name="dictionary">
                        <xsl:choose>
                            <xsl:when test="@name">
                                <xsl:element name="string">
                                    <xsl:text>name</xsl:text>
                                </xsl:element>
                                <xsl:element name="string">
                                    <xsl:value-of select="@name"/>
                                </xsl:element>
                            </xsl:when>
                            <xsl:otherwise/>
                        </xsl:choose>
                        <xsl:choose>
                            <xsl:when test="@type">
                                <xsl:element name="string">
                                    <xsl:text>type</xsl:text>
                                </xsl:element>
                                <xsl:element name="string">
                                    <xsl:value-of select="@type"/>
                                </xsl:element>
                            </xsl:when>
                            <xsl:otherwise/>
                        </xsl:choose>
                        <xsl:choose>
                            <xsl:when test="@default">
                                <xsl:element name="string">
                                    <xsl:text>default</xsl:text>
                                </xsl:element>
                                <xsl:element name="string">
                                    <xsl:value-of select="@default"/>
                                </xsl:element>
                            </xsl:when>
                            <xsl:otherwise/>
                        </xsl:choose>
                        <xsl:choose>
                            <xsl:when test="@fixed">
                                <xsl:element name="string">
                                    <xsl:text>fixed</xsl:text>
                                </xsl:element>
                                <xsl:element name="string">
                                    <xsl:value-of select="@fixed"/>
                                </xsl:element>
                            </xsl:when>
                            <xsl:otherwise/>
                        </xsl:choose>
                        <xsl:choose>
                            <xsl:when test="@use">
                                <xsl:element name="string">
                                    <xsl:text>use</xsl:text>
                                </xsl:element>
                                <xsl:element name="string">
                                    <xsl:value-of select="@use"/>
                                </xsl:element>
                            </xsl:when>
                            <xsl:otherwise/>
                        </xsl:choose>
                    </xsl:element>
                </object>
            </xsl:when>
            <xsl:otherwise/>
        </xsl:choose>
    </xsl:template>

    <xsl:template match="xsd:sequence">
        <xsl:apply-templates/>
    </xsl:template>

    <xsl:template match="xsd:group">
    </xsl:template>

    <xsl:template match="xsd:attributeGroup">
    </xsl:template>

</xsl:stylesheet>

