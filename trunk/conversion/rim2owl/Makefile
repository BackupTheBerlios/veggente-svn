SAXON=~/bin/saxon2
ACCOUNT=carenini@shell.berlios.de
REMOTE_DIR=/home/groups/veggente/htdocs/ns
INPUT_MIF=../input/MIF
main: xsl
xsl: rim datatype cmet
rim:
	$(SAXON) -sall $(INPUT_MIF)/rim.coremif rim2owl.xsl > rim.owl
datatype:
	$(SAXON) rim_datatypes.owl clean_dt.xsl > rim_datatypes_fix.owl
cmet:
	$(SAXON) -sall $(INPUT_MIF)/COCT*.mif cmet2owl.xsl > rim_cmet.owl
install: rim_inst dt_inst cmet_inst
cmet_inst:
	scp rim_cmet.owl $(ACCOUNT):$(REMOTE_DIR)/RIM_CMET
rim_inst:
	scp rim.owl $(ACCOUNT):$(REMOTE_DIR)/RIMOntology
dt_inst:
	scp rim_datatypes_fix.owl $(ACCOUNT):$(REMOTE_DIR)/RIMDatatype
