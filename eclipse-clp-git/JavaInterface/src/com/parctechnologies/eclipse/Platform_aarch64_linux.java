// BEGIN LICENSE BLOCK
// Version: CMPL 1.1
//
// The contents of this file are subject to the Cisco-style Mozilla Public
// License Version 1.1 (the "License"); you may not use this file except
// in compliance with the License.  You may obtain a copy of the License
// at www.eclipse-clp.org/license.
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
// the License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is  The ECLiPSe Constraint Logic Programming System. 
// The Initial Developer of the Original Code is  Cisco Systems, Inc. 
// Portions created by the Initial Developer are
// Copyright (C) 2001 - 2006 Cisco Systems, Inc.  All Rights Reserved.
// 
// Contributor(s): Josh Singer, Parc Technologies
// 
// END LICENSE BLOCK

//Title:        Java/ECLiPSe interface
//Version:      $Id$
//Author:       Josh Singer
//Company:      Parc Technologies
//Description:  Encapsulated Singleton for platform-dependent code (linux on ARM)
//              architecture).
package com.parctechnologies.eclipse;

class Platform_aarch64_linux extends Platform_UNIX
{
  public boolean supportsEmbeddedEclipse()
  {
    return(true);
  }
  public boolean supportsOutOfProcessEclipse()
  {
    return(true);
  }

  private final String ECLIPSE_PLATFORM_NAME = "aarch64_linux";
  String getEclipsePlatformName()
  {
    return(ECLIPSE_PLATFORM_NAME);
  }
  private final String SHARED_LIBRARY_EXTENSION = "so";
  String getSharedLibraryExtension()
  {
    return(SHARED_LIBRARY_EXTENSION);
  }
}
