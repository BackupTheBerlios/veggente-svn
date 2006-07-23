<?xml version="1.0" encoding="iso-8859-1" ?>
<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
    >
    <xsl:template match="/source_file">
        <html xmlns="http://www.w3.org/1999/xhtml" lang="it" xml:lang="it">
            <head>
				<title>Lifting output</title>
				<meta http-equiv="Content-type" content="text/html; charset=iso-8859-1"/>
            </head>
            <body>
                <b>Lifting of <xsl:value-of select="@name"/></b><br/>
                <b>Using <xsl:value-of select="@ontology_name"/></b><br/><br/>
                Resources: <xsl:value-of select="count(Resource)"/><br/>
                Resolution errors: <xsl:value-of select="count(//Error) - count(Error)"/><br/>
                Panic errors: <xsl:value-of select="count(Error)"/>
                <table border="1" frame="above">
                    <tr>
                        <th>XML resource</th>
                        <th>RDF resource</th>
                        <th>type</th>
                    </tr>
                    <xsl:apply-templates select="Resource"/>
                </table>
            </body>
        </html>
    </xsl:template>

    <xsl:template match="Resource">
        <xsl:choose>
            <xsl:when test="Error">
                <tr>
                    <td><xsl:value-of select="name"/></td>
                    <td><xsl:value-of select="Error"/></td>
                    <td/>
                </tr>
            </xsl:when>
            <xsl:otherwise>
                <tr>
                    <td><xsl:value-of select="name"/></td>
                    <td><xsl:value-of select="onto_res"/></td>
                    <td><xsl:value-of select="substring-after(type,'#')"/></td>
                </tr>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>

