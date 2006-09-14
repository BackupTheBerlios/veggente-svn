SAXON=~/bin/saxon2
ACCOUNT=carenini@shell.berlios.de
VEGGENTE=http://veggente.berlios.de/ns
REMOTE_DIR=/home/groups/veggente/htdocs/ns
INPUT_MIF=../input/MIF
OUTPUT_DIR=../output
CMET_INFO_DIR=$(PWD)/../input/MIF
EXPLODE_DIR=../output/explode
CWM_RULES=../cwm-rules

main: xsl

xsl: rim datatype cmet_container

rim:
	$(SAXON) -sall $(INPUT_MIF)/rim.coremif rim2owl.xsl > $(OUTPUT_DIR)/rim.owl

datatype:
	$(SAXON) rim_datatypes.owl clean_dt.xsl > $(OUTPUT_DIR)/rim_datatypes_fix.owl

cmet_container:
	$(SAXON) -sall $(INPUT_MIF)/cmetinfo.coremif cmet_container.xsl > $(OUTPUT_DIR)/rim_cmet.owl

cmet:
	for i in `/bin/ls -1 $(INPUT_MIF)/*_*.mif`; do \
		echo "Processing $$i"; \
		touch  $(OUTPUT_DIR)/cmet/`basename $$i .mif`;\
		$(SAXON) -sall $$i cmet2owl.xsl > $(OUTPUT_DIR)/cmet/`basename $$i .mif`;\
	done;

cmet_test:
	$(SAXON) -sall $(INPUT_MIF)/PORT_MT020001UV01.mif cmet2owl.xsl mif_input_path=$(CMET_INFO_DIR)>$(OUTPUT_DIR)/cmet/PORT_MT020001UV01.owl
	$(SAXON) -sall $(INPUT_MIF)/PORT_HD020001UV01.mif cmet2owl.xsl mif_input_path=$(CMET_INFO_DIR)>$(OUTPUT_DIR)/cmet/PORT_HD020001UV01.owl

install: rim_inst dt_inst cmet_inst

cmet_testinst:
	scp $(OUTPUT_DIR)/cmet/PORT_MT020001UV01.owl $(ACCOUNT):$(REMOTE_DIR)/cmet/PORT_MT020001UV01
	scp $(OUTPUT_DIR)/cmet/PORT_HD020001UV01.owl $(ACCOUNT):$(REMOTE_DIR)/cmet/PORT_HD020001UV01

cmet_inst:
	scp $(OUTPUT_DIR)/rim_cmet.owl $(ACCOUNT):$(REMOTE_DIR)/RIM_CMET
	scp $(OUTPUT_DIR)/cmet/* $(ACCOUNT):$(REMOTE_DIR)/cmet/

rim_inst:
	scp $(OUTPUT_DIR)/rim.owl $(ACCOUNT):$(REMOTE_DIR)/RIMOntology

dt_inst:
	scp $(OUTPUT_DIR)/rim_datatypes_fix.owl $(ACCOUNT):$(REMOTE_DIR)/RIMDatatype

debug:
	for i in `/bin/ls -1 $(OUTPUT_DIR)/cmet`; do \
		echo "Processing $$i"; \
		$(SAXON) -sall $(OUTPUT_DIR)/cmet/$$i debug.xsl > $(OUTPUT_DIR)/debug/$$i.out;\
	done;
deps: main_deps cmet_deps

main_deps:
	for i in $(OUTPUT_DIR)/*.owl; do \
		$(SAXON) $(OUTPUT_DIR)/$$i doc_dep.xsl > $(OUTPUT_DIR)/`basename $(OUTPUT_DIR)/$$i .owl`-tmp.dep; \
	done; \
	for i in $(OUTPUT_DIR)/*-tmp.dep; do \
		$(SAXON) $(OUTPUT_DIR)/$$i unique_dep_find.xsl > $(OUTPUT_DIR)/`basename $(OUTPUT_DIR)/$$i -tmp.dep`.dep; \
	done; \
	rm -f $(OUTPUT_DIR)/*-tmp.dep
	mv $(OUTPUT_DIR)/*.dep $(OUTPUT_DIR)/deps/

cmet_deps:
	for i in $(OUTPUT_DIR)/cmet/*; do \
		$(SAXON) $$i doc_dep.xsl > $$i-tmp.dep; \
	done; \
	for i in $(OUTPUT_DIR)/cmet/*-tmp.dep; do \
		$(SAXON) $$i unique_dep_find.xsl > $(OUTPUT_DIR)/cmet/`basename $(OUTPUT_DIR)/cmet/$$i -tmp.dep`.dep; \
	done; \
	rm -f $(OUTPUT_DIR)/cmet/*-tmp.dep
	mv $(OUTPUT_DIR)/*.dep $(OUTPUT_DIR)/deps/

explode_deps: explode_main explode_cmet

explode_main:
	 cwm --rdf $(VEGGENTE)/RIMOntology `cat $(OUTPUT_DIR)/rim.dep` --n3 $(CWM_RULES)/owl.n3 --filter=$(CWM_RULES)/owl-lite.n3 --think --ugly --purge --rdf >$(EXPLODE_DIR)/rim.tmp && cwm --rdf $(VEGGENTE)/RIMOntology $(EXPLODE_DIR)/rim.tmp > $(EXPLODE_DIR)/RIMOntology
	 cwm --rdf $(VEGGENTE)/RIMDatatype --n3 $(CWM_RULES)/owl.n3 --filter=$(CWM_RULES)/owl-lite.n3 --think --ugly --purge --rdf >$(EXPLODE_DIR)/rim_dt.tmp && cwm --rdf $(VEGGENTE)/RIMDatatype $(EXPLODE_DIR)/rim_dt.tmp > $(EXPLODE_DIR)/RIMDatatype
	 rm -f $(EXPLODE_DIR)/*.tmp 

explode_cmet:
	for i in $(OUTPUT_DIR)/cmet/*; do \
		 DOCNAME=`basename $$i`; \
		 echo "Processing $$DOCNAME"; \
	   	 cwm --rdf $(VEGGENTE)/cmet/$$DOCNAME `cat $(OUTPUT_DIR)/deps/$$DOCNAME.dep`  --n3 $(CWM_RULES)/owl.n3 --filter=$(CWM_RULES)/owl-lite.n3 --think --ugly --purge --rdf >$(EXPLODE_DIR)/$$DOCNAME.tmp && \
	  	 cwm --rdf $(VEGGENTE)/cmet/$$DOCNAME $(EXPLODE_DIR)/$$DOCNAME.tmp `cat $(OUTPUT_DIR)/deps/$$DOCNAME.dep`> $(EXPLODE_DIR)/$$DOCNAME; \
	done; \
 	rm -f $(EXPLODE_DIR)/*.tmp 

test_explode:
	DOCNAME=PORT_MT020001UV01; \
	echo "Applying rules on $$DOCNAME"; \
	time cwm --rdf $(VEGGENTE)/cmet/$$DOCNAME `cat $(OUTPUT_DIR)/deps/$$DOCNAME.dep`  --n3 $(CWM_RULES)/owl.n3 --filter=$(CWM_RULES)/owl_complete.n3 --purge --ugly --rdf >$(EXPLODE_DIR)/$$DOCNAME.tmp && \
	echo "Merging results into original data..." && \
	time cwm --rdf $(VEGGENTE)/cmet/$$DOCNAME $(EXPLODE_DIR)/$$DOCNAME.tmp `cat $(OUTPUT_DIR)/deps/$$DOCNAME.dep` --bySubject > $(EXPLODE_DIR)/$$DOCNAME;

svn_commit:
	cd ../../; svn commit
