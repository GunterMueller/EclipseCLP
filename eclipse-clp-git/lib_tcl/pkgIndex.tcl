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

# Tcl package index file, version 1.1
# This file is generated by the "pkg_mkIndex" command
# and sourced either when an application starts up or
# by a "package unknown" script.  It invokes the
# "package ifneeded" command to set up package-related
# information so that packages will be loaded automatically
# in response to "package require" commands.  When this
# script is sourced, the variable $dir must contain the
# full path name of this file's directory.

package ifneeded eclipse_peer_multitask 1.0 [list source [file join $dir tkmulti.tcl]]
package ifneeded remote_eclipse 1.0 [list source [file join $dir tkec_remote.tcl]]
package ifneeded eclipse 1.0 [list tclPkgSetup $dir eclipse 1.0 {{eclipse.tcl source {ec_init ec_set_option ec_interface_type}}}]
package ifneeded eclipse_tools 1.0 [list tclPkgSetup $dir eclipse_tools 1.0 {{eclipse_tools.tcl source {ec_tools_init tkecl:get_defaults tkecl:set_tkecl_tkdefaults tkecl:tracer_popup tkecl:kill_tracer}}}]
package ifneeded tkinspect 1.0 [list tclPkgSetup $dir tkinspect 1.0 {{tkinspect.tcl source {tkinspect:CreateImage tkinspect:CurrentSelection tkinspect:Display tkinspect:DisplayImage tkinspect:DisplayKey tkinspect:DisplayPath tkinspect:Exit tkinspect:Expand_termtype tkinspect:Get_numentry tkinspect:Get_subterm_info tkinspect:Get_subterms tkinspect:Inspect_term_init tkinspect:Look_term tkinspect:Modify_name tkinspect:Move tkinspect:MoveDown tkinspect:Newselection tkinspect:Numentry tkinspect:Popnumentry tkinspect:PostSelect tkinspect:SelectCurrent tkinspect:SelectInvoc tkinspect:Setup_move tkinspect:ToggleSymbols tkinspect:Update_menulabel tkinspect:Update_optmenu tkinspect:Write_command_initial tkinspect:Write_path tkinspect:ec_resume_inspect tkinspect:RaiseWindow tkinspect:helpinfo tkinspect:inspect_command}}}]
package ifneeded tkec_icons 1.0 [list source [file join $dir tkec_icons.tcl]]





