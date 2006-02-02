<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:date="http://exslt.org/dates-and-times" xmlns:func="http://exslt.org/functions" xmlns:str="http://exslt.org/strings" extension-element-prefixes="date func str" version="1.0">

<!-- 
svnlog.xslt by Martin Pittenauer / TheCodingMonkeys            
Web: www.codingmonkeys.de, Mail: map@codingmonkeys.de                                           

iso->rfc822 date conversion code heavily inspired by Steven Engelhardt (http://www.deez.info/sengelha/)
Tested with libxslt.
-->

    <xsl:output omit-xml-declaration="no" indent="yes" encoding="UTF-8" method="xml" />       
    
    <xsl:template match="log">
      <rss version="2.0">
           <channel>
                <title>SVN: Veggente project</title> <!-- put your repository name here -->
				<link>http://veggente.berlios.de</link> <!-- put your project homepage here -->
                <description>svn commit log</description>
                <language>en-us</language>
                <copyright>2006, Alessio Carenini</copyright> <!-- put your company here -->
                <generator>svnlog.xslt</generator>
                <xsl:apply-templates select="logentry" />
           </channel>
      </rss>
    </xsl:template>
    
    <xsl:template match="logentry">
      <item>
		   <title>Revision <xsl:value-of select="@revision"/> by <xsl:value-of select="author" /></title>
		   <link>http://svn.berlios.de/wsvn/veggente/trunk/?rev=<xsl:value-of select="@revision"/>&amp;sc=1</link>
           <pubDate>
            <xsl:call-template name="rfc822">
                <xsl:with-param name="isodate" select="date" />
            </xsl:call-template>
           </pubDate>
           <!-- Accoring to RSS 2.0 specs, author has to be an rfc822 valid email address -->
           <author><xsl:value-of select="author" />-nospam@users.berlios.de</author> <!-- put your domain here -->
           <description><xsl:value-of select="msg" />
                <xsl:apply-templates select="paths/path" />
           </description>
      </item>
    </xsl:template>
         
    <xsl:template match="paths/path">
        <xsl:call-template name="svncommand"><xsl:with-param name="command" select="@action" /></xsl:call-template>
        &lt;td&gt;&amp;nbsp;&lt;a href="https://www.codingmonkeys.de/project/svn<xsl:value-of select="."/>"&gt;<xsl:value-of select="."/>&lt;/a&gt;&lt;/td&gt;&lt;/tr&gt; <!-- put link to your repository here -->
    </xsl:template>

    <xsl:template name="rfc822">
        <xsl:param name="isodate" />
        <xsl:variable name="dayOfWeek" select="date:day-abbreviation($isodate)" />
        <xsl:variable name="day" select="date:day-in-month($isodate)" />
        <xsl:variable name="monthAbbr" select="date:month-abbreviation($isodate)" />
        <xsl:variable name="year" select="date:year($isodate)" />
        <xsl:variable name="hour" select="date:hour-in-day($isodate)" />
        <xsl:variable name="minute" select="date:minute-in-hour($isodate)" />
        <xsl:variable name="second" select="round(date:second-in-minute($isodate))" />

        <xsl:value-of select="$dayOfWeek" />
        <xsl:text>, </xsl:text>
        <xsl:value-of select="$day" />
        <xsl:text> </xsl:text>
        <xsl:value-of select="$monthAbbr" />
        <xsl:text> </xsl:text>
        <xsl:value-of select="$year" />
        <xsl:choose>
            <xsl:when test="$hour and $minute and $second">
                <xsl:text> </xsl:text>
                <xsl:value-of select="str:align($hour, '00', 'right')" />
                <xsl:text>:</xsl:text>
                <xsl:value-of select="str:align($minute, '00', 'right')" />
                <xsl:text>:</xsl:text>
                <xsl:value-of select="str:align($second, '00', 'right')" />
                <xsl:text> GMT</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text> 00:00:00 GMT</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    <xsl:template name="svncommand">
        <xsl:param name="command"/>
        <xsl:choose>
           <xsl:when test="$command = 'A'">Added</xsl:when>
           <xsl:when test="$command = 'D'">Deleted</xsl:when>
           <xsl:when test="$command = 'M'">Modified</xsl:when>
        </xsl:choose>
    </xsl:template>

</xsl:stylesheet>
