SUBDIRS= src 
doc_DIR=doc

#  uncomment the following if mapper requires the math library
#mapper_LDADD=-lm

EXTRA_DIST=mapper.lsm.in mapper.spec.in $(doc_DIR)/mapper.texinfo

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

#  build and install the .info pages
info_TEXINFOS = $(doc_DIR)/mapper.texinfo
mapper_TEXINFOS = $(doc_DIR)/gpl.texinfo

#  install the man pages
man_MANS=$(doc_DIR)/mapper.1
