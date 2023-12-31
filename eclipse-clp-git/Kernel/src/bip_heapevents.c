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
 * Copyright (C) 1996-2006 Cisco Systems, Inc.  All Rights Reserved.
 * 
 * Contributor(s): 
 * 
 * END LICENSE BLOCK */

/*----------------------------------------------------------------------
 * System:	ECLiPSe Constraint Logic Programming System
 * Version:	$Id: bip_heapevents.c,v 1.6 2017/09/04 01:44:29 jschimpf Exp $
 *
 * Contents:	Built-ins for the heap-event-primitives
 *
 *		This file has been factored out of bip_record.c in 05/2006
 *----------------------------------------------------------------------*/

#include	"config.h"
#include        "sepia.h"
#include        "types.h"
#include        "embed.h"
#include        "mem.h"
#include        "error.h"
#include	"dict.h"
#include	"property.h"
#include	"emu_export.h"

#include        <stdio.h>	/* for sprintf() */

static dident	d_defers0_;
static pword	true_pw_;

/*----------------------------------------------------------------------
 * Prolog heap events
 *----------------------------------------------------------------------*/

/* INSTANCE TYPE DECLARATION */

/* ****** See types.h ****** */

/* METHODS */

static dident
_kind_event(void)
{
    return d_.event;
}

static void
_free_heap_event(t_heap_event *event)	/* event != NULL */
{
    /* It is possible for the reference count to drop to -1
     * when freeing an embedded self-reference. The equality
     * test ensures the event is freed once and once only.
     */
    int rem = ec_atomic_add(&event->ref_ctr, -1);
    if (rem == 0)
    {
	free_heapterm(&event->goal);
	hg_free_size(event, sizeof(t_heap_event));
#ifdef DEBUG_RECORD
	p_fprintf(current_err_, "\n_free_heap_event(0x%x)", event);
	ec_flush(current_err_);
#endif
    }
}

static t_heap_event *
_copy_heap_event(t_heap_event *event)	/* event != NULL */
{
    ec_atomic_add(&event->ref_ctr, 1);
    return event;
}

static void
_mark_heap_event(t_heap_event *event)	/* event != NULL */
{
    /*
     * Since the heap event may contain embedded handles of itself,
     * we have to avoid looping: overwrite event->goal with nil for
     * the duration of the marking, and set it back afterwards.
     * This is safe because dictionary GC is atomic.
     */
    pword pw = event->goal;
    Make_Nil(&event->goal);
    mark_dids_from_heapterm(&pw);
    event->goal = pw;

    mark_dids_from_pwords(&event->module, &event->module + 1);
}


t_ext_ptr
ec_new_heap_event(value vgoal, type tgoal, value vm, type tm, int defers)
{
    t_heap_event *event = (t_heap_event *)hg_alloc_size(sizeof(t_heap_event));
    event->ref_ctr = 1;
    event->enabled = 1;
    event->defers = defers;
    event->module.tag = tm;
    event->module.val = vm;
    event->goal.tag = tgoal;
    event->goal.val = vgoal;
    return (t_ext_ptr) event;
}


/* CLASS DESCRIPTOR (method table) */

t_ext_type heap_event_tid = {
    (void (*)(t_ext_ptr)) _free_heap_event,
    (t_ext_ptr (*)(t_ext_ptr)) _copy_heap_event,
    (void (*)(t_ext_ptr)) _mark_heap_event,
    0, /* string_size */
    0, /* to_string */
    0,	/* equal */
    (t_ext_ptr (*)(t_ext_ptr)) _copy_heap_event,
    0,	/* get */
    0,	/* set */
    _kind_event
};


/* PROLOG INTERFACE */

/*
 * event_create(+Goal, +Options, -EventHandle, +Module)
 * event_retrieve(+EventHandle, -Goal, -Module)
 * event_enable(+Handle)
 * event_disable(+Handle)
 */


static int
p_event_create4(value vevent, type tevent, value vopt, type topt, value vhandle, type thandle, value vmodule, type tmodule, ec_eng_t *ec_eng)
{
    t_heap_event *event;
    pword hevent;
    int defers = 0;
    int res = PSUCCEED;

    Check_Ref(thandle);
    Check_Goal(tevent);
    Check_List(topt);

    while (IsList(topt))
    {
	pword *pw = vopt.ptr++;
	Dereference_(pw);
	Check_Atom(pw->tag);
	if (pw->val.did == d_defers0_)
	    defers = 1;
	else
	    { Bip_Error(RANGE_ERROR); }
	Dereference_(vopt.ptr);
	topt.all = vopt.ptr->tag.all;
	vopt.ptr = vopt.ptr->val.ptr;
	Check_List(topt);
    }

    event = (t_heap_event *) ec_new_heap_event(true_pw_.val, true_pw_.tag, vmodule, tmodule, defers);
    hevent = ecl_handle(ec_eng, &heap_event_tid, (t_ext_ptr) event);

    /* Unify the handle before the heap copy in case it is embedded within 
     * the event
     */
    res = Unify_Pw(vhandle, thandle, hevent.val, hevent.tag);
    res = res == PSUCCEED ? create_heapterm(ec_eng, &event->goal, vevent, tevent) : res;

    if (res != PSUCCEED) {
	hg_free_size(event, sizeof(t_heap_event));
	Bip_Error(res);
    }

    /* The goal *may* have an embedded reference to the handle,
     * the heap copy will have incremented the reference count to two.
     * As a result we reset it back to one here to ensure we avoid liveness
     * maintained by the embedded internal reference. 
     */
    event->ref_ctr = 1;

    Succeed_;
}


static int
p_event_create(value vevent, type tevent, value vhandle, type thandle, value vmodule, type tmodule, ec_eng_t *ec_eng)
{
    pword opt;
    Make_Nil(&opt);
    return p_event_create4(vevent, tevent, opt.val, opt.tag, vhandle, thandle, vmodule, tmodule, ec_eng);
}


static int
p_event_retrieve(value vhandle, type thandle, value vgoal, type tgoal, value vmodule, type tmodule, ec_eng_t *ec_eng)
{
    t_heap_event *event;
    pword goal;

    Prepare_Requests;

    Get_Typed_Object(vhandle, thandle, &heap_event_tid, event);

    /* the goal is read-only, no locking needed */
    get_heapterm(ec_eng, &event->goal, &goal);

    /* Is the event enabled or disabled? */
    if (event->enabled) {
        Request_Unify_Pw(vgoal, tgoal, goal.val, goal.tag);
    } else {
	/* Event disabled, just return the goal as 'true' */
	Request_Unify_Atom(vgoal, tgoal, d_.true0)
    }

    Request_Unify_Pw(vmodule, tmodule, event->module.val, event->module.tag);

    Return_Unify;
}


static int
p_event_enable(value vhandle, type thandle, ec_eng_t *ec_eng)
{
    t_heap_event *event;

    Get_Typed_Object(vhandle, thandle, &heap_event_tid, event);

    /* If an event is in the event queue but has been disabled
     * then it must be removed before the event is re-enabled.
     */
    if (!event->enabled) 
    {
	purge_disabled_dynamic_events(ec_eng, event);
    }

    event->enabled = 1;

    Succeed_;
}


static int
p_event_disable(value vhandle, type thandle, ec_eng_t *ec_eng)
{
    t_heap_event *event;

    Get_Typed_Object(vhandle, thandle, &heap_event_tid, event);

    event->enabled = 0;

    Succeed_;
}


/*----------------------------------------------------------------------
 * Initialisation
 *----------------------------------------------------------------------*/

void
bip_heapevent_init(int flags)
{
    d_defers0_ = in_dict("defers", 0);
    Make_Atom(&true_pw_, d_.true0);

    if (flags & INIT_SHARED)
    {
	(void) built_in(in_dict("event_create_", 3), p_event_create, B_SAFE|U_SIMPLE);
	(void) built_in(in_dict("event_create_", 4), p_event_create4, B_SAFE|U_SIMPLE);
	(void) built_in(in_dict("event_retrieve", 3), p_event_retrieve, B_UNSAFE|U_FRESH);
	(void) built_in(in_dict("event_enable", 1), p_event_enable, B_SAFE|U_NONE);
	(void) built_in(in_dict("event_disable", 1), p_event_disable, B_SAFE|U_NONE);
    }
}

