# BEGIN LICENSE BLOCK
# Version: CMPL 1.1
#
# The contents of this file are subject to the Cisco-style Mozilla Public
# License Version 1.1 (the "License"); you may not use this file except
# in compliance with the License.  You may obtain a copy of the License
# at www.eclipse-clp.org/license.
# 
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
# the License for the specific language governing rights and limitations
# under the License. 
# 
# The Original Code is  The ECLiPSe Constraint Logic Programming System. 
# The Initial Developer of the Original Code is  Cisco Systems, Inc. 
# Portions created by the Initial Developer are
# Copyright (C) 2006 Cisco Systems, Inc.  All Rights Reserved.
# 
# Contributor(s): 
# 
# END LICENSE BLOCK

DEST = ../../doc_internal
MODE = a+rX,ug+w


SUBDIRS = kernel project

.PHONY: $(SUBDIRS)

install: kernel
	@mkdir -p $(DEST)
	cp -pr *.html *.txt *.png $(DEST)
	@chmod -R $(MODE) $(DEST)

$(SUBDIRS):
	$(MAKE) -C $@ install


clean:
	@for dir in $(SUBDIRS); do \
	    $(MAKE) -C $$dir clean; \
	done

