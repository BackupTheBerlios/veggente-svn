SAXON=~/bin/saxon2
ACCOUNT=carenini@shell.berlios.de
REMOTE_DIR=/home/groups/veggente/htdocs/ns
main: xsl
xsl:
	$(SAXON) -sall ../input/MIF/rim.coremif rim2owl.xsl > rim.owl
	$(SAXON) -sall rim_datatypes.owl clean_dt.xsl > rim_datatypes_fix.owl
install:
	scp rim.owl $(ACCOUNT):$(REMOTE_DIR)/RIMOntology
	scp rim_datatypes.owl $(ACCOUNT):$(REMOTE_DIR)/RIMDatatype
	scp rim_datatypes_fix.owl $(ACCOUNT):$(REMOTE_DIR)/RIMSimpletype
