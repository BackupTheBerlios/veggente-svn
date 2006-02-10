SUBDIRS= src 
doc_DIR=doc

#  uncomment the following if elevator requires the math library
#elevator_LDADD=-lm

EXTRA_DIST=elevator.lsm.in elevator.spec.in $(doc_DIR)/elevator.texinfo

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

#  build and install the .info pages
info_TEXINFOS = $(doc_DIR)/elevator.texinfo
elevator_TEXINFOS = $(doc_DIR)/gpl.texinfo

#  install the man pages
man_MANS=$(doc_DIR)/elevator.1
