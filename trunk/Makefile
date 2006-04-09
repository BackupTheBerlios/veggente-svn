REMOTE="carenini@shell.berlios.de"
WWW_DIR="www"
main: svn_commit rss copy_rss
svn_commit: 
	svn commit
update:
	svn co svn+ssh://carenini@svn.berlios.de/svnroot/repos/veggente/trunk/
rss: 
	svn --xml log svn+ssh://carenini@svn.berlios.de/svnroot/repos/veggente/trunk > svn_log.xml
	xsltproc $(WWW_DIR)/svnlog.xslt svn_log.xml > $(WWW_DIR)/devel.rss
copy_rss:
	scp $(WWW_DIR)/devel.rss $(REMOTE):www/
	scp $(WWW_DIR)/news.rss $(REMOTE):www/
www_update:
	scp $(WWW_DIR)/css/style.css $(REMOTE):www/css/
	scp $(WWW_DIR)/devel.rss $(REMOTE):www/
	scp $(WWW_DIR)/news.rss $(REMOTE):www/
	scp $(WWW_DIR)/foaf.rdf $(REMOTE):www/
	scp $(WWW_DIR)/*.html $(REMOTE):www/
	scp $(WWW_DIR)/img/* $(REMOTE):www/img/
