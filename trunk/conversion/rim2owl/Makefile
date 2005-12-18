SAXON=~/bin/saxon2
ACCOUNT=carenini@shell.berlios.de
REMOTE_DIR=/home/groups/veggente/htdocs/ns
INPUT_MIF=../input/MIF
OUTPUT_DIR=../output
main: xsl
xsl: rim datatype cmet_container
rim:
	$(SAXON) -sall $(INPUT_MIF)/rim.coremif rim2owl.xsl > $(OUTPUT_DIR)/rim.owl
datatype:
	$(SAXON) rim_datatypes.owl clean_dt.xsl > $(OUTPUT_DIR)/rim_datatypes_fix.owl
cmet_container:
	$(SAXON) -sall $(INPUT_MIF)/cmetinfo.coremif cmet_container.xsl > $(OUTPUT_DIR)/rim_cmet.owl
cmet:
	for i in `/bin/ls $(INPUT_MIF)/*.mif`; do \
		touch  $(OUTPUT_DIR)/cmet/`basename $$i .mif`;\
		$(SAXON) -sall $$i cmet2owl.xsl > $(OUTPUT_DIR)/cmet/`basename $$i .mif`;\
	done;
install: rim_inst dt_inst cmet_inst
cmet_inst:
	scp $(OUTPUT_DIR)/rim_cmet.owl $(ACCOUNT):$(REMOTE_DIR)/RIM_CMET
rim_inst:
	scp $(OUTPUT_DIR)/rim.owl $(ACCOUNT):$(REMOTE_DIR)/RIMOntology
dt_inst:
	scp $(OUTPUT_DIR)/rim_datatypes_fix.owl $(ACCOUNT):$(REMOTE_DIR)/RIMDatatype
