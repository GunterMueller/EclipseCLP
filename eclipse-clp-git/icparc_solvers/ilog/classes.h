/* BEGIN LICENSE BLOCK
 * Version: CMPL 1.1
 *
 * The contents of this file are subject to the Cisco-style Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file except
 * in compliance with the License.  You may obtain a copy of the License
 * at www.eclipse-clp.org/license.
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License. 
 * 
 * The Original Code is  The ECLiPSe Constraint Logic Programming System. 
 * The Initial Developer of the Original Code is  Cisco Systems, Inc. 
 * Portions created by the Initial Developer are
 * Copyright (C) 2006 Cisco Systems, Inc.  All Rights Reserved.
 * 
 * Contributor(s): Pascal Brisset
 * 
 * END LICENSE BLOCK */

// $Id: classes.h,v 1.1.1.1 2006/09/23 01:54:03 snovello Exp $

#ifndef CLASSES_H
#define CLASSES_H

#include "stdecil.h"

#ifndef CLASSES_CC


class EC_IlcIntVar : public IlcIntVar {
public:
  EC_IlcIntVar(IlcManager m, IlcInt min, IlcInt max, EC_word w);
  EC_IlcIntVar(IlcManager, IlcIntVar, EC_word);
  EC_IlcIntVar(IlcManager, IlcIntArray, EC_word);
  EC_ref *getEC_term() { return (EC_ref*)(getObject()); };
  void whenCondition(void (*)(EC_IlcIntVar), IlcWhenEvent event);
};
#endif
#endif
