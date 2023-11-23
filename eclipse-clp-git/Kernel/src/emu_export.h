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
 * Copyright (C) 1989-2006 Cisco Systems, Inc.  All Rights Reserved.
 * 
 * Contributor(s): 
 * 
 * END LICENSE BLOCK */


/*
 * SEPIA INCLUDE FILE
 *
 * VERSION	$Id: emu_export.h,v 1.24 2017/09/01 03:05:09 jschimpf Exp $
 */

/*
 * IDENTIFICATION		emu_export.h
 *
 * DESCRIPTION	
 *	
 *
 * CONTENTS:			Macros and extern declarations related to the
 *				abstract machine
 *
 */

#define SAFE_B_AREA 4	/* interrupt-safe area on control stack (in pwords) */

/*
 * macros to recognise control frames
 */

extern vmcode
		engine_exit_code_[],
		gc_fail_code_[],
		soft_cut_code_[],
		slave_fail_code_[],
		stop_fail_code_[],
		recurs_fail_code_[],
		*catch_fail_code_,
		catch_unint_fail_code_[],
		external_fail_code_[],
		exception_fail_code_[];

#define IsNestingFrame(top)\
        ((vmcode *)(top)->backtrack == recurs_fail_code_)
#define IsRecursionFrame(top)\
        ((vmcode *)(top)->backtrack == stop_fail_code_ ||\
         (vmcode *)(top)->backtrack == recurs_fail_code_ ||\
	 (vmcode *)(top)->backtrack == slave_fail_code_)
#define IsExceptionFrame(top)\
        ((vmcode *)(top)->backtrack == exception_fail_code_)
#define IsCatchFrame(top)\
        ((vmcode *)(top)->backtrack == catch_fail_code_ ||\
        (vmcode *)(top)->backtrack == catch_unint_fail_code_)
#define IsCatchEventsDeferredFrame(top)\
        ((vmcode *)(top)->backtrack == catch_unint_fail_code_)
#define IsGcFrame(top)\
        ((vmcode *)(top)->backtrack == gc_fail_code_)
#define IsUnpubParFrame(top)\
	SameCode(*((vmcode *)(top)->backtrack), Retry_seq)
#define IsPubParFrame(top)\
	SameCode(*((vmcode *)(top)->backtrack), Fail_clause)
#define IsParallelFrame(top)\
	(IsPubParFrame(top) || IsUnpubParFrame(top))

#define RETRY_ME_INLINE_SIZE 4
#define IsRetryMeInlineFrame(top)\
	SameCode(*((vmcode *)(top)->backtrack), Retry_me_inline)
#define TRUST_ME_INLINE_SIZE 3
#define IsTrustMeInlineFrame(top)\
	SameCode(*((vmcode *)(top)->backtrack), Trust_me_inline)
#define RETRY_INLINE_SIZE 4
#define IsRetryInlineFrame(top)\
	SameCode(*((vmcode *)(top)->backtrack), Retry_inline)
#define TRUST_INLINE_SIZE 4
#define IsTrustInlineFrame(top)\
	SameCode(*((vmcode *)(top)->backtrack), Trust_inline)

#define Top(pw)		((struct top_frame *)(pw))
#define Invoc(pw)	((struct invocation_frame *)(pw))
#define Exception(pw)	((struct exception_frame *)(pw))
#define Chp(pw)		((struct choice_frame *)(pw))
#define ChpPar(pw)	((struct parallel_frame *)(pw))
#define ChpDbg(pw)	((struct choice_debug *)(pw))

/* macros for accessing choicepoint fields */
#define BPrev(pw)	((Top(pw)-1)->frame.args)
#define BBp(pw)		((Top(pw)-1)->backtrack)
#define BAlt(pw)	((Top(pw)-1)->frame.chp_par->alt)

#define BTop(pw)	((Top(pw)-1))
#define BChp(pw)	((Top(pw)-1)->frame.chp)
#define BPar(pw)	((Top(pw)-1)->frame.chp_par)
#define BException(pw)	((Top(pw)-1)->frame.exception)
#define BInvoc(pw)	((Top(pw)-1)->frame.invoc)


/* find the actual size of an environment whose size is not known statically
 * (indicated by -1 in the environment size location in the code)
 */
#define DynEnvSize(e)	(((((e)-1)->tag.kernel >> 8) & 0xffff) + 1)
#define DynEnvFlags(e)	(((e)-1)->tag.kernel)
#define DynEnvVal(e)	(((e)-1)->val.wptr)
#define DynEnvDE(e)	((e)-2)
#define DynEnvDbgPri(e) ((e)-3)
#define DynEnvDbgPort(e) ((e)-4)
#define DynEnvDbgInvoc(e) ((e)-5)
#define DynEnvDbgPath(e) ((e)-6)
#define DynEnvDbgLine(e) ((e)-7)
#define DynEnvDbgFrom(e) ((e)-8)
#define DynEnvDbgTo(e)   ((e)-9)
#define DYNENVDBGSIZE   8

#define PushDynEnvHdr(NrSlots, Flags, Val) \
	    (--SP)->tag.all = ((NrSlots) << 8) | Flags | TPTR;\
	    SP->val.wptr = (uword *) (Val);


/*
 * Environment descriptors and their access macros.
 * Environment descriptors occur in call and retry/trust_inline
 * instructions. They indicate which parts of an environment are active,
 * and consist of an environment size or an activity bitmap (EAM).
 *
 * Preliminary scheme, allowing both environment sizes and bitmaps, 32-bit:
 *	<29 bit env size>000	LSB=00 indicates size field, all slots active,
 *				size in bytes, multiple of pword.
 *	<31 bit EAM bitmap>1	LSB= 1 indicates 31-bit EAM (activity bitmap)
 *	<pointer  to  EAM>10	LSB=10 indicates pointer to uword-array
 *				of 31+1 bit maps, where all but the last
 *				uword of the array have LSB=0.
 * compatible on 64-bit:
 *	<60 bit env size>0000	LSB=00 indicates size field, all slots active,
 *				size in bytes, multiple of pword.
 *	<31 bit  EAM bitmap>1	LSB= 1 indicates 31-bit EAM (activity bitmap)
 *				bits 32..63 are same as bit 31 (sign extended)
 *	<pointer  to  EAM>010	LSB=10 indicates pointer to uword-array
 *				of 31+1 bit maps, where all but the last
 *				uword of the array have LSB=0.
 *
 * Dynamic environments marked by -1 size field (true size in Y1 tag).
 *
 * Bitmaps are cut up into 31-bit chunks (bits 1 to 31 of a uword, with
 * bit 0 of the uword being used as a marker for the last chunk).
 * First chunk corresponds to Y1(bit1)..Y31(bit31), second chunk to
 * Y32(bit1)..Y62(bit31), etc.  To have portable byte code, we use 31 bit
 * chunks even on 64-bit machines, with the rest of the uword wasted.
 */

#define EAM_CHUNK_SZ	31

#define Esize(size)	((vmcode)((size) * (word)sizeof(pword)))

#define EdescIsSize(ed)	(((ed) & 3) == 0)
#define EdescSize(ed,e)	((ed) == -((word)sizeof(pword)) ? DynEnvSize(e) : (ed) / (word)sizeof(pword))

#define EdescIsEam(ed)	(!EdescIsSize(ed))
#define EdescEamPtr(ed)	((ed)&1 ? (uword*)(&(ed)) : (uword*)((ed) & ~2))
#define EamPtrNext(peam)	(!(*(peam)++ & 1))
/* (uint32_t) cast needed to get rid of sign extension on 64-bit */
#define EamPtrEam(peam)	((uint32_t)(*(uword*)(peam)) >> 1)

/* Final, simplified scheme (env size no longer supported):
 *	<31 bit EAM bitmap>1	LSB=1 indicates 31-bit EAM (activity bitmap)
 *	<pointer   to  EAM>0	LSB=0 indicates pointer to uword-array
 *				of 31+1 bit maps, where all but the last
 *				uword of the array have LSB=0.
 * Dynamic environment sizes (formerly marked by -1 size) are then
 * indicated by a pointer to a particular static address.
 *
#define EdescIsSize(ed)	((uword*)(ed) == &dyn_env_size_indicator)
#define EdescSize(ed,e)	DynEnvSize(e)
#define EdescEamPtr(ed)	((ed)&1 ? (uword*)(&(ed)) : (uword*)(ed))
*/


/*---------------------------------------------------------------------------
 * Overflow checks and garbage collection
 *---------------------------------------------------------------------------*/

extern int	control_ov(ec_eng_t*),
		local_ov(ec_eng_t*);

#ifdef IN_C_EMULATOR
#undef Check_Trail_Ov
#define Check_Trail_Ov  if (TT <= TT_LIM) \
	{ Export_B_Sp_Tg_Tt trail_ov(ec_eng); Import_None }
#undef Check_Gc
#define Check_Gc        if (TG >= TG_LIM) \
	{ Export_B_Sp_Tg_Tt global_ov(ec_eng); Import_None }
#endif

#define LOCAL_CONTROL_GAP (SAFE_B_AREA+NARGREGS+sizeof(struct invocation_frame))


/*
 * Constants for default size check, in pwords. It must hold
 * GC_MAX_HEAD == GC_MAX_FACT + GC_MAX_PUT + GC_MAX_PUTS
 */
#define GC_MAX_HEAD		200	/* head up to first put's	*/
#define GC_MAX_FACT		 80	/* the whole unit clause	*/
#define GC_MAX_PUT		120	/* put's in the body		*/
#define GC_MAX_PUTS		 50	/* puts' before Exit		*/


/*
 * The gap left between TT and TG. It specifies how much can be pushed
 * on the two stacks before an overflow check is really needed.
 * It should be greater or equal to the maximum of
 * MAXARITY, GC_MAX_HEAD and the size of the largest trail frame.
 */
#define GLOBAL_TRAIL_GAP	NARGREGS


/*
 * The initial trail gap is slightly larger. On overflow, it gets reduced
 * to GLOBAL_TRAIL_GAP and a GC triggered in the hope that the gc will
 * make trail expansion unnecessary.
 */
#define	TRAIL_GAP		(GLOBAL_TRAIL_GAP + 128)


/*
 * If after a GC and stack trimming there are less than TG_MIN_SEG
 * pwords available, we give up with stack overflow.
 */

#define TG_MIN_SEG		1024		/* in pwords */


/*---------------------------------------------------------------------------
 * Trailing and Untrailing
 *---------------------------------------------------------------------------*/

/*
 * the two least significant bits identify the trail frame:
 */
#define TRAIL_ADDRESS	0x0
#define TRAIL_TAG	0x1
#define TRAIL_MULT	0x2
#define TRAIL_EXT	0x3

/*
 * an extended trail is further specified by the Etype Field:
 */
#define TrailedEtype(x)          (((word)(x) >> 4) & 0x0f)
#define TrailedEtypeField(x)     ((word)(x) << 4)
#define TrailedEsize(x)          (((word)(x) >> 8) & 0xffffff)
#define TrailedEsizeField(x)     (((word)(x) & 0xffffff) << 8)

#define TRAIL_UNDO		0x0
#define TRAIL_UNDO_STAMPED	0x1


#define TRAIL_UNDO_FLAGS 0
#define TRAIL_UNDO_ADDRESS 1
#define TRAIL_UNDO_FUNCT 2
#define TRAIL_UNDO_SIMPLE_HEADER_SIZE 3

#define TRAIL_UNDO_STAMP_ADDRESS 3
#define TRAIL_UNDO_OLDSTAMP 4
#define TRAIL_UNDO_STAMPED_HEADER_SIZE 5


/*
 * how to get the tag from a tag trail frame
 * the argument is the first word of the trail frame
 * x is casted to preserve the sign bit in the shift,
 * 0x9fffffff is casted because ansi C treats it as unsigned.
 */
#define TrailedTag(x)           (((word)(x) >> 2) & (word)(~(MARK|LINK)))

/*
 * the following macros are used in value trails
 * the argument is the first word of the trail frame
 */
#define TrailedOffset(x)        ((word)(x)>>8)
#define TrailedNumber(x)        ((word)(x)>>4 & 0xf)
#define TrailedType(x)          ((word)(x) & 0xc)

#define TRAILED_TYPE_MASK 0xc

/* consistency check with ifdefs in sepia.h */
#if ((TRAILED_PWORD|TRAILED_REF|TRAILED_WORD32|TRAILED_COMP) & ~TRAILED_TYPE_MASK)
Trailed type macros not defined correctly!!!
#endif


/*
 * Extract the adress of the trailed location from an arbitrary trail frame.
 * tr is a pointer to the first word of the frame
 */
#define TrailedLocation(tr)     (((word) *(tr) & 3) ? *((tr)+1) : *(tr))

/*
 * Skip a trail frame
 * tr is a pointer to the first word of the trail frame
 * end is set to the beginning of the next trail frame
 */
#define End_Of_Frame(tr, end) \
    switch(((word) *(tr) & 3)) {\
    default:\
    case TRAIL_ADDRESS:\
        end = (tr)+1; break;\
    case TRAIL_TAG:\
        end = (tr)+2; break;\
    case TRAIL_MULT:\
        end = (tr) + TrailedNumber((word)*(tr)) + 3; break;\
    case TRAIL_EXT:\
        end = (tr) + TrailedEsize((word)*(tr)); break;\
    }


/*
 * simple address trail
 * the trailed pword is restored to a self reference with TREF tag
 */

#define Trail_(pw) \
	*--TT = (pword *) (pw);\
	Check_Trail_Ov

/*
 * Tag trail for trailing the binding of a non-standard variable
 * On untrailing, the trailed pword is restored to a self reference
 * with the old tag (note that the GC bits are always restored to 00 !).
 * (this macro would be simpler if we were sure never to trail a TREF tag!)
 */

#define Trail_Tag(pw) \
	*--TT = (pword *) (pw);\
	*--TT = (pword *) ( ((pw)->tag.kernel & SIGN_BIT)\
				| ((pw)->tag.kernel << 2) | TRAIL_TAG);\
	Check_Trail_Ov

/*
 * different value trails
 * The type information is needed by the garbage collector.
 * The addresses in the trail must point to proper pwords,
 * at least if they point into the global stack.
 */

#define Trail_Pointer(pw) \
	*--TT = ((pword *) (pw))->val.ptr;\
	*--TT = (pword *) (pw);\
	*--TT = (pword *) (TRAILED_REF | TRAIL_MULT);\
	Check_Trail_Ov

#define Trail_Comp(pw) \
	*--TT = ((pword *) (pw))->val.ptr;\
	*--TT = (pword *) (pw);\
	*--TT = (pword *) (TRAILED_COMP | TRAIL_MULT);\
	Check_Trail_Ov

/*
 * trail a word at a fixed offset from a pword.
 * The type information is needed by the garbage collector.
 */

#define Trail_Word(pw, offset, type) \
	*--TT = (pword *) (*((uword *) (pw) + (offset)));\
	*--TT = (pword *) (pw);\
	*--TT = (pword *) ((offset) << 8 | (type) | TRAIL_MULT);\
	Check_Trail_Ov

#define Trail_Pword(pw) \
	*--TT = (pword *) ((pword *) (pw))->tag.all;\
	*--TT = ((pword *) (pw))->val.ptr;\
	*--TT = (pword *) (pw);\
	*--TT = (pword *) ((1 << 4) | TRAILED_PWORD | TRAIL_MULT);\
	Check_Trail_Ov


/* trail multiple words at an offset from pw */

#define Trail_Pwords(pw, offset, n) { \
	word _i = (offset) + (n); \
	do { \
	    TT -= 2; \
	    * (pword *) TT = (pw)[--_i]; \
	} while (_i > offset); \
	*--TT = (pword *) (pw); \
	*--TT = (pword *) (word)((2*(offset) << 8) | (2*(n)-1 << 4) | TRAILED_PWORD | TRAIL_MULT); \
	Check_Trail_Ov \
    }

/*
 * general trail for multiple words
 * nwords is the number of trailed 32 bit words (max 16).
 * offset is the offset of the first trailed word relative to pw
 * (in units of 32 bit words), type must be one of
 * TRAILED_PWORD, TRAILED_WORD32, TRAILED_REF, TRAILED_COMP
 * specifying what kind of data has been trailed (for the GC)
 */

#define Trail_Frame(pw, offset, nwords, type) \
	{\
	    uword *ptr = (uword *)(pw) + (offset) + (nwords);\
	    while(ptr > (uword *)(pw) + (offset))\
		*--TT = (pword *) (*--ptr);\
	    *--TT = ((pword *) (ptr));\
	    *--TT = (pword *) (((offset) << 8) | ((nwords) - 1 << 4)\
			| (type) | TRAIL_MULT);\
	    Check_Trail_Ov\
	}

/*
 * This trail will cause the specified function to be called on
 * backtracking, usually to undo a side effect.
 * The function will be called with a single argument which is
 * a pointer to the prolog word pw.
 */

#define Trail_Undo(pw, function) \
	TT = (pword **) ((void (**)(pword *)) TT - 1);\
	* (void (**)(pword *)) TT = (void (*)(pword *)) (function);\
	*--TT = (pword *) (pw);\
	*--TT = (pword *) ( TrailedEsizeField(TRAIL_UNDO_SIMPLE_HEADER_SIZE)\
		| TrailedEtypeField(TRAIL_UNDO) | TRAIL_EXT);\
	Check_Trail_Ov


/*
 * conditional trailing macros
 */

#define Trail_If_Needed(pw) \
	if((pword *)(pw) < GB || (pword *)(pw) >= EB) {\
	    Trail_(pw)\
	}

/* the following works only with local stack locations */

#define Trail_If_Needed_Eb(pw) \
	if((pword *)(pw) >= EB) {\
	    Trail_(pw)\
	}

/* the following works only with global stack locations */

#define Trail_If_Needed_Gb(pw) \
	if((pword *) (pw) < GB) {\
	    Trail_(pw)\
	}

#define Trail_Tag_If_Needed_Gb(pw) \
	if((pword *) (pw) < GB) {\
	    Trail_Tag(pw)\
	}

/* Check a pointer for pointing into deterministic part of the global stack.
 * Use this macro when you are not sure pw really points into the global
 * stack (e.g. ground structures in the heap) !!!
 */
#define NewLocation(pw) ((pw) >= GB && (pw) <= TG)

/* Check whether a pword can be recognised as being "new", i.e. younger
 * than the most recent choicepoint. This is the case only for items that
 * have been pushed onto the global stack since.
 */
#define NewValue(v, t)	(ISPointer((t).kernel) && NewLocation((v).ptr))


#define Trail_Word_If_Needed_Gb(pw, offset, type) \
	if ((pword *) (pw) < GB) {\
	     Trail_Word(pw, offset, type)\
	}

#define Trail_Pointer_If_Needed_Gb(pw) \
	if ((pword *) (pw) < GB) {\
	     Trail_Pointer(pw)\
	}

#define Trail_Pword_If_Needed_Gb(pw) \
	if ((pword *) (pw) < GB) {\
	     Trail_Pword(pw)\
	}


/*
 * With the assembler emulator we must be careful not to untrail something
 * above the local stack top, since this may corrupt the C stack.
 */

#ifdef AS_EMU

extern pword	*spmax_;

#define Ignore_If_Above_Sp(pw) \
	if((pw) < SP && (pw) >= spmax_) continue;

#else /* AS_EMU */

#define Ignore_If_Above_Sp(pw)

#endif /* AS_EMU */


/*
 * The Untrailing Routine
 *
 * top		Where to stop untrailing (previous TT value)
 * ctr		an auxiliary variable of type word
 * pw		an auxiliary pointer of type pword *
 */

#define Untrail_(ttptr, top, ctr, pw) \
      while(ttptr < top) {\
	  switch((((word) *ttptr) & 3)) {\
	  case TRAIL_ADDRESS:\
		pw = *ttptr++;\
		Ignore_If_Above_Sp(pw);\
		pw->val.ptr = pw;\
		pw->tag.kernel = TREF;\
		break;\
	  case TRAIL_TAG:\
		pw = *(ttptr+1);\
		pw->val.ptr = pw;\
		pw->tag.kernel = TrailedTag(*(ttptr));\
		ttptr += 2;\
		break;\
	  case TRAIL_MULT:\
		ctr = (word) *ttptr++;\
		pw = (pword *)((uword *) *(ttptr++) + TrailedOffset(ctr));\
		ctr = TrailedNumber(ctr);\
		do {\
		    pw->val.ptr = *ttptr++;\
		    pw = (pword *) ((uword *) pw + 1);\
		} while (ctr--);\
		break;\
	  case TRAIL_EXT:\
		Untrail_Export;\
		untrail_ext(ec_eng, ttptr, UNDO_FAIL);\
		Untrail_Import;\
		ttptr += TrailedEsize(*ttptr);\
		break;\
	  }\
      }


#ifdef IN_C_EMULATOR

#define Untrail_Export	Export_B_Sp_Tg_Tt_Eb_Gb
#define Untrail_Import	Import_Tg_Tt

#define Untrail_Variables(top, ctr, pw) \
	Untrail_(TT, top, ctr, pw)

#else /* IN_C_EMULATOR */

#define Untrail_Export
#define Untrail_Import

#define Untrail_Variables(top) {\
	word n; pword *pw1;\
	Untrail_(TT, top, n, pw1);\
	}

#endif /* IN_C_EMULATOR */

/*---------------------------------------------------------------------------
 * Mechanism to flag asynchronous events by simulating a stack overflow
 *
 * A shadow register TG_SLS always holds the correct value of TG_SL.
 * TG_SL itself can be set to 0 (thus faking a stack overflow) in order
 * to trigger synchronous engine events.
 * Whenever TG_SL or TG_LIM is changed, make sure that TG_SL =< TG_LIM !!
 * Use only the macros below to manipulate TG_SL, TG_SLS and TG_LIM!
 *---------------------------------------------------------------------------*/

#define FakedOverflow	(ec_atomic_load(&TG_SL) == NULL)

#define Fake_Overflow				\
	ec_atomic_store(&TG_SL, NULL)

/* The following must only be called when we are about to handle
 * FakedOverflow conditions.
 */
#define Reset_Faked_Overflow			\
	ec_atomic_store(&TG_SL, TG_SLS)


/* Update TG_SL and TG_SLS if there is no FakedOverflow condition.
 * Otherwise only update TG_SLS.
 */

#define Set_Tg_Soft_Lim(new) {			\
	ec_compare_and_swap(&TG_SL, TG_SLS, new);	\
	TG_SLS = new;				\
    }

#define Save_Tg_Soft_Lim(saved)			\
	(saved) = TG_SLS;

#define Restore_Tg_Soft_Lim(saved)		\
	Set_Tg_Soft_Lim(TG_LIM < (saved) ? TG_LIM : (saved))

#define Set_Tg_Lim(newlim) {			\
	if ((TG_LIM = (newlim)) < TG_SLS) {	\
	    Set_Tg_Soft_Lim(TG_LIM)		\
	}					\
    }


#define Adjust_GcTg_and_TgSl(TG) {		\
    	if (TG < GCTG) {			\
	    GCTG = TG;				\
	    Restore_Tg_Soft_Lim(TG + TG_SEG);	\
	}					\
    }


#define Compute_Gcb(gcb) {			\
	pword *_gcb = B.args;			\
	while (BChp(_gcb)->tg >= GCTG  &&	\
	    !(IsRecursionFrame(BTop(_gcb)) ||	\
		IsExceptionFrame(BTop(_gcb))))	\
	{					\
	    _gcb = BPrev(_gcb);			\
	}					\
	gcb = _gcb;				\
    }

#define EventPending		(TG >= ec_atomic_load(&TG_SL))

#define GlobalOverflow		(TG >= TG_SLS)	/* a real stack overflow */


/* Macros to check for exit- or throw-requests, and service them */

#ifdef IN_C_EMULATOR
#define Poll_Interrupts() {			\
	int _event_flags = ec_atomic_load(&EVENT_FLAGS); \
	if (_event_flags & URGENT_EVENT_POSTED) goto _do_requested_throw_;		\
	if (_event_flags & EXIT_REQUEST) goto _do_requested_exit_;		\
    }
#endif

#define Return_On_Request() { \
	int _event_flags; \
	if (FakedOverflow && ((_event_flags = ec_atomic_load(&EVENT_FLAGS)) \
				    & (URGENT_EVENT_POSTED|EXIT_REQUEST))) \
	    return ecl_do_requested_action(ec_eng, _event_flags, 0); \
    }

#define Longjmp_On_Request() { \
	int _event_flags; \
	if (FakedOverflow && ((_event_flags = ec_atomic_load(&EVENT_FLAGS)) \
				    & (URGENT_EVENT_POSTED|EXIT_REQUEST))) \
	    ecl_do_requested_action(ec_eng, _event_flags, 1); \
    }


/*---------------------------------------------------------------------------
 * General purpose macros
 *---------------------------------------------------------------------------*/

/* Add a positive offset to a pointer. If this overflows the address
 * range, set it to max instead. */
#define Safe_Add_To_Pointer(old, pos_offset, max, new) { \
	(new) = (old) + (pos_offset); \
	if ((new) < (old)) (new) = (max); \
    }
 	
#define Safe_Sub_From_Pointer(old, pos_offset, min, new) { \
	(new) = (old) - (pos_offset); \
	if ((new) > (old)) (new) = (min); \
    }


/*---------------------------------------------------------------------------
 * Binding macros to be used in the built-ins to speed up the unification
 *---------------------------------------------------------------------------*/

#define IsLocal(p)	(SP <= (p))

#ifndef IN_C_EMULATOR

#undef Bind_
#define Bind_(pw,v,t) 			\
	Trail_If_Needed(pw)		\
	(pw)->tag.all = (uword) (t);	\
	(pw)->val.all = (uword) (v);

/*
 * Return_Bind_Var(value, type, (uword), word)
 * 	Bind a free (maybe mutable) variable to a term which is known
 *	not to be a reference or a mutable object and then return
 *	from the built-in. This macro can be used instead of Return_Unify().
 */
#define Return_Bind_Var(vval, vtag, term, termtag)	\
    if (IsVar(vtag)) {					\
	Bind_((vval).ptr, term, termtag);		\
	Succeed_;					\
    } else {						\
	pword	aux_pw;					\
	aux_pw.val.all = (uword) (term);		\
	aux_pw.tag.kernel = (termtag);			\
	return bind_c(ec_eng, (vval).ptr, &aux_pw, &MU);	\
    }
	
#define Request_Bind_Var(vval, vtag, term, termtag)	\
    if (uNiFy_result != PFAIL) {			\
	if (IsVar(vtag)) {				\
	    Bind_((vval).ptr, term, termtag);		\
	} else {					\
	    pword	aux_pw;				\
	    aux_pw.val.all = (uword) (term);		\
	    aux_pw.tag.kernel = (termtag);		\
	    uNiFy_result = bind_c(ec_eng, (vval).ptr, &aux_pw, &MU);\
	}						\
    }

/*
 * Bind_Var(value, type, (uword), word)
 * 	Bind a free (maybe mutable) variable to a term which is known
 *	not to be a reference or a mutable object. This macro can be
 *	used instead of Request_Unify().
 */
#define Bind_Var(vval, vtag, term, termtag)	\
    if (IsVar(vtag)) {				\
	Bind_((vval).ptr, term, termtag);	\
    } else {					\
	pword	aux_pw;				\
	aux_pw.val.all = (uword) (term);	\
	aux_pw.tag.kernel = (termtag);		\
	(void) bind_c(ec_eng, (vval).ptr, &aux_pw, &MU);\
    }

#endif /* IN_C_EMULATOR */

#ifdef IN_C_EMULATOR

#define EmuStringStart(pw) \
	(       SameTypeC((pw)->tag, TBUFFER)\
		?       ((pw) + 1)\
		:       ((pw) + 1)->val.ptr\
	)

/* when count is negative, the strings are equal	*/
/* CAUTION: pw1/pw2 are expanded several times!		*/
/* This code raises alignment warnings, but it's ok.	*/

#define Compare_Strings(pw1, pw2, count)		\
	if ((count = pw1->val.nint) == pw2->val.nint) {	\
	    pw1 = EmuStringStart(pw1);			\
	    pw2 = EmuStringStart(pw2);			\
	    while (count--) {				\
		if (*(char*)(pw1) != *(char*)(pw2))	\
			break;				\
		pw1 = (pword *) ((char*)(pw1) + 1);	\
		pw2 = (pword *) ((char*)(pw2) + 1);	\
	    }						\
	}

#else /* IN_C_EMULATOR */

/* when count is negative, the strings are equal	*/
/* CAUTION: v1/v2 are expanded several times!		*/

#define Compare_Strings(v1, v2, count)			\
	if ((count = (v1).ptr->val.nint) == (v2).ptr->val.nint) {	\
	    register char *s1 = StringStart(v1);	\
	    register char *s2 = StringStart(v2);	\
	    while (count--)				\
		if (*s1++ != *s2++)			\
			break;				\
    }
	
#endif /* IN_C_EMULATOR */

/*
 * Bind and return a numeric result, make appropriate type error if 
 * the result argument is not a variable or the same numeric type.
 */

#ifdef ARITH_OUTPUT_TYPE_ERROR
#define Return_Numeric(v, t, result) \
    if (IsRef(t)) { \
        Return_Bind_Var(v, t, result.val.all, result.tag.all); \
    } else if (SameType(t, result.tag)) { \
        Succeed_If( \
            IsSimple(t) ? SimpleEq(t.kernel, v, result.val) \
            : tag_desc[TagType(t)].equal(result.val.ptr, v.ptr)); \
    } else if (tag_desc[TagType(t)].super == tag_desc[TagType(result.tag)].super) { \
        Fail_; \
    } else { Bip_Error(TYPE_ERROR); }
#else
#define Return_Numeric(v, t, result) \
    if (IsRef(t)) { \
        Return_Bind_Var(v, t, result.val.all, result.tag.all); \
    } else if (SameType(t, result.tag)) { \
        Succeed_If( \
            IsSimple(t) ? SimpleEq(t.kernel, v, result.val) \
            : tag_desc[TagType(t)].equal(result.val.ptr, v.ptr)); \
    } else { \
        Fail_; \
    }
#endif


/*---------------------------------------------------------------------------
 * Acquire objects for engine
 *---------------------------------------------------------------------------*/

/*
 * Get a reference and a lock on the stream
 * (with auto-cleanup on return from the external)
 * mode=0:	any descriptor, even closed
 * mode=SRDWR:	require open descriptor
 * mode=SREAD:	require read-descriptor
 * mode=SWRITE:	require write-descriptor
 */

#define Get_Locked_Stream(v, t, mode, nst) {			\
	int err_or_copied;					\
	nst = get_stream_id(v, t, mode, 1, ec_eng, &err_or_copied);	\
	if (nst == NO_STREAM) { Bip_Error(err_or_copied) }	\
    }


/*---------------------------------------------------------------------------
 * Coroutining / Metaterms
 *---------------------------------------------------------------------------*/

#define MetaTerm(pw)			((pw) + 1)
#define MetaDelayTerm(pw)		MetaTerm(pw)

/*
 * Maximum overhead size of an attribute in its canonical I/O format,
 * i.e. ( name1:Attr1 , name2:Attr , ... , nameN:AttrN )
 * N*3 for every :/2 structure plus (N-1)*3 for every ,/2
 */
#define ATTR_IO_TERM_SIZE (6 * MetaArity - 3)


#define Push_var_delay(vptr, tdummy) {			\
	register pword *_pw = TG;			\
	Push_List_Frame();				\
	if (IsLocal(vptr)) {	/* assume IsRef */	\
	    Make_Var(_pw);	/* globalise */		\
	    Make_Ref(vptr, _pw);			\
	} else {					\
	    Make_Ref(_pw, vptr);			\
	}						\
	if (SV) {					\
	    Make_List(&_pw[1], SV);			\
	} else						\
	    Make_Nil(&_pw[1]);				\
	SV = _pw;					\
    }

#define Push_var_delay_unif(v, t) Push_var_delay(v, t)


/*
 * Suspension structure:
 *
 *	|-----------------|
 *	|                 |
 *	|- - - MODULE  - -|
 *	|                 |
 *	|-----------------|
 *	|                 |
 *	|- - -  GOAL - - -|
 *	|                 |
 *	|-----------------|
 *	| RP/PRIO  WS TREF|	<= these are mutable fields
 *	|- - - STATE - - -|
 *	|    timestamp    |
 *	|-----------------|
 *	|      INVOC      |     <= CAUTION: no tag!
 *	|- - - - - - - - -|
 *	|       PRI       |
 *	|-----------------|       |---------|
 *	|0--       DD TDE |       |  TSUSP  |
 *	|- - - - - - - - -|       |- - - - -|
 *	|       LD        |    /-------     |
 *	|-----------------|<--/   |---------|
 *
 * When the suspension is SuspDead, then the suspension may be partially
 * garbage collected, i.e. goal and module may no longer be present and
 * it may be removed from the LD list.
 *
 *	|-----------------|
 *	|      INVOC      |
 *	|- - - - - - - - -|
 *	|       PRI       |
 *	|-----------------|       |---------|
 *	|0--       1D TDE |       |  TSUSP  |
 *	|- - - - - - - - -|       |- - - - -|
 *	|       NULL      |    /-------     |
 *	|-----------------|<--/   |---------|
 *
 *
 * Suspension states (non-demon):
 *   ________         _________                             ____
 *  |   00   |       |   11    |                           |    |
 *  |Sleeping|--sch->|Scheduled|--------uns---exe--------->|Dead|<--kill
 *  |________|       |_________|                           |____|
 *
 * Suspension states (demon):
 *   ________         _________         ___________         ____
 *  |   00   |--sch->|   11    |--uns->|    10     |       |    |
 *  |Sleeping|       |Scheduled|       |Unscheduled|       |Dead|<--kill
 *  |________|<-exe--|_________|<-sch--|___________|       |____|
 *               |                            |
 *               \----------------------------/
 *
 * An unscheduled suspension is one that had been scheduled, but some other
 * code made its actual execution redundant and called unschedule_suspension/1.
 * The difference between 'unscheduled' and 'sleeping' is that unscheduled
 * suspensions are still in the WL lists. From there it can either be
 * rescheduled cheaply, or go to dead/sleeping state at the time it gets
 * taken out of the WL lists.
 *
 * Priorities:
 * We store both priority and run_priority in the suspension.
 * The run-priority is always equal or higher than the schedule-priority.
 */


/* In the SUSP_FLAGS tag: */
#define SUSP_FLAG_DEMON		0x00000100
#define SUSP_FLAG_DEAD		0x00000200

/* In the SUSP_STATE tag: */
#define SUSP_FLAG_PRIO		0x00F00000
#define SUSP_FLAG_RUNPRIO	0x0F000000
#define SUSP_STATE_SCHED	0x00000100	/* scheduled */
#define SUSP_STATE_INWL		0x00000200	/* in woken lists */

#define SUSP_PRIO_SHIFT		20
#define SUSP_RUNPRIO_SHIFT	24
#define SUSP_MAX_PRIO		PRIORITY_MAX

#define SUSP_LD		0	/* offsets in suspensions */
#define SUSP_FLAGS	0
#define SUSP_PRI	1
#define SUSP_INVOC	1
#define SUSP_HEADER_SIZE 2
#define SUSP_STATE	2
#define SUSP_GOAL	3
#define SUSP_MODULE	4
#define SUSP_SIZE	5

/* field access macros */
#define SuspDemon(p)		((p)[SUSP_FLAGS].tag.kernel & SUSP_FLAG_DEMON)
#define SuspDead(p)		((p)[SUSP_FLAGS].tag.kernel & SUSP_FLAG_DEAD)
#define SuspScheduled(p)	((p)[SUSP_STATE].tag.kernel & SUSP_STATE_SCHED)
#define SuspInWL(p)		((p)[SUSP_STATE].tag.kernel & SUSP_STATE_INWL)
#define SuspPrio(p)		(((unsigned) ((p)[SUSP_STATE].tag.kernel) & SUSP_FLAG_PRIO)>>SUSP_PRIO_SHIFT)
#define SuspRunPrio(p)		(((unsigned) ((p)[SUSP_STATE].tag.kernel) & SUSP_FLAG_RUNPRIO)>>SUSP_RUNPRIO_SHIFT)
#define SuspStamp(p)		((p)[SUSP_STATE].val.ptr)
#define SuspPrevious(p)		(((pword *) p)[SUSP_LD].val.ptr)
#define SuspProc(p)		((pri*)(((pword *) p)[SUSP_PRI].val.wptr))
#define SuspDebugInvoc(p)	(((pword *) p)[SUSP_INVOC].tag.kernel)
#define SuspModule(p)		(((pword *) p)[SUSP_MODULE].val.did)

#define SuspTagDead(t)		((t) & SUSP_FLAG_DEAD)

/* field update macros */
#define Set_Susp_Scheduled(p)	Set_Susp_State(p, (SUSP_STATE_SCHED|SUSP_STATE_INWL))
#define Set_Susp_Delayed(p)	Reset_Susp_State(p, (SUSP_STATE_SCHED|SUSP_STATE_INWL))
#define Set_Susp_Rescheduled(p)	Set_Susp_State(p, SUSP_STATE_SCHED)
#define Set_Susp_Unscheduled(p)	Reset_Susp_State(p, SUSP_STATE_SCHED)
#define Set_Susp_Dead(p)	Set_Susp_Flag(p, SUSP_FLAG_DEAD)
#define Set_Susp_Dead_Untrailed(p)	Set_Susp_Flag_Untrailed(p, SUSP_FLAG_DEAD)
#define Set_Susp_DebugInvoc(p,i)	p[SUSP_INVOC].tag.kernel = (i);

#define Init_Susp_Header(p,proc) \
	(p)[SUSP_LD].val.ptr = (pword *)LD;\
	Update_LD(p);\
	(p)[SUSP_FLAGS].tag.kernel = TDE|(PriFlags(proc) & PROC_DEMON ? SUSP_FLAG_DEMON : 0);\
	(p)[SUSP_PRI].val.wptr = (uword *)(proc);\
	(p)[SUSP_INVOC].tag.kernel = 0;
#define Init_Susp_Dead(p) \
	(p)[SUSP_LD].val.ptr = (pword *)0;\
	(p)[SUSP_FLAGS].tag.kernel = TDE|SUSP_FLAG_DEAD;

/*
 * In order to be able to safely use global stack addresses as time stamps,
 * we push a "witness" word with every choicepoint. Their addresses are
 * used as the time stamps. GB will always point to such a witness.
 * A stamp looks like a [] (a ref to a TNIL of the proper age).
 * Make_Stamp() and OldStamp() are in sepia.h
 * A stamp older than any other is at the first word of the stack!
 */

#define Push_Witness	TG++->tag.kernel = TNIL;

#define OlderStamp(p,b) \
    OlderStampThanGlobalAddress(p,BChp(b)->tg)

#define OlderStampThanGlobalAddress(p,tg) \
    ((p)->val.ptr < tg)

#define Update_Stamp(p) \
    Trail_Pointer(p);\
    (p)->val.ptr = GB;



#define Trail_State(p) \
    if ((p)->val.ptr < GB /* implies p < GB */) {\
        Trail_Pword(p);\
	(p)->val.ptr = BChp(B.args)->tg;\
    }

#define Init_Susp_State(p, prio, runprio) { \
    (p)[SUSP_STATE].val.ptr = BChp(B.args)->tg;\
    (p)[SUSP_STATE].tag.kernel = TREF | \
    	((prio) << SUSP_PRIO_SHIFT) | \
	(((prio) < (runprio) ? prio : runprio) << SUSP_RUNPRIO_SHIFT); \
}

#define Set_Susp_State(p, f) \
    Trail_State(&(p)[SUSP_STATE]);\
    (p)[SUSP_STATE].tag.kernel |= (f)

#define Reset_Susp_State(p, f) \
    Trail_State(&(p)[SUSP_STATE]);\
    (p)[SUSP_STATE].tag.kernel &= ~(f)

#define Set_Susp_Prio(p, prio) { \
    int _runprio = SuspRunPrio(p); \
    Trail_State(&(p)[SUSP_STATE]);\
    (p)[SUSP_STATE].tag.kernel = \
    	((p)[SUSP_STATE].tag.kernel & ~(SUSP_FLAG_PRIO|SUSP_FLAG_RUNPRIO)) | \
	((prio) << SUSP_PRIO_SHIFT) | \
	(((prio) < _runprio ? prio : _runprio) << SUSP_RUNPRIO_SHIFT); \
}


#define Set_Susp_Flag_Untrailed(p, f) \
    (p)[SUSP_FLAGS].tag.kernel |= (f)
#define Set_Susp_Flag(p, f) \
    if ((p) < GB) {\
        Trail_Word(p, 1, TRAILED_WORD32);\
    }\
    Set_Susp_Flag_Untrailed(p, f)

#define Reset_Susp_Flag_Untrailed(p, f) \
    (p)[SUSP_FLAGS].tag.kernel &= ~(f)
#define Reset_Susp_Flag(p, f) \
    if ((p) < GB) {\
        Trail_Word(p, 1, TRAILED_WORD32);\
    }\
    Reset_Susp_Flag_Untrailed(p, f)


#define Update_LD(suspension)			\
	LD = (suspension);

#define Reset_DE		DE = (pword *) 0
#define Kill_DE			{ if (DE) { Set_Susp_Dead(DE); Reset_DE; }}


/*
 * Woken goals structure
 *
 *	      . . .
 *	|                 |
 *	|-----------------|
 *	|                 |
 *	|- List for #1 - -|
 *	|                 |
 *	|-----------------|
 *	|    TSUSP	  |
 *	|- - - - - - - - -|
 *	|    previous LD  |
 *	|-----------------|
 *	|    TINT	  |
 *	|- - - - - - - - -|
 *	|    previous WP  |
 *	|-----------------|
 *	|    TCOMP	  |
 *	|- - - - - - - - -|
 *	|    previous WL  |
 *	|-----------------|
 *	|    TDICT	  |
 *	|- - - - - - - - -|
 *	|woken/SUSP_MAX_PR|
 *	|-----------------|<-- WL
 *
 */

#define WL_PREVIOUS		1
#define WL_PREVIOUS_WP		2
#define WL_PREVIOUS_LD		3
#define WL_FIRST		4
#define WL_ARITY		(WL_FIRST + PRIORITY_MAX - 1)

#define LD_END			WL[WL_PREVIOUS_LD].val.ptr

#define Init_WP(prio) {\
	Make_Stamp(&ec_eng->wp_stamp);\
	WP = (prio);\
    }
    
#define Set_WP(prio) {\
	if (WP != (prio)) {\
	    if (OldStamp(&WP_STAMP)) {\
		Update_Stamp(&WP_STAMP)\
		Trail_Word(&WP, 0, TRAILED_WORD32)\
	    }\
	    WP = (prio);\
	}\
    }
    

#define WLPrevious(wl)		((wl) + WL_PREVIOUS)
#define WLPreviousWP(wl)	((wl) + WL_PREVIOUS_WP)
#define WLPreviousLD(wl)	((wl) + WL_PREVIOUS_LD)
#define WLFirst(wl)		((wl) + WL_FIRST)
#define WLArity(maxprio)	((maxprio) + WL_FIRST - 1)
#define WLMaxPrio(wl)		(DidArity(wl->val.did) - WL_FIRST + 1)

#define Update_MU(vptr) {			\
	register pword *_pw = TG;		\
	TG += 2;				\
	Check_Gc;				\
	_pw[0].val.ptr = vptr;			\
	_pw[0].tag.kernel = TLIST;		\
	if (MU) {				\
	    _pw[1].val.ptr = MU;			\
	    _pw[1].tag.kernel = TLIST;		\
	} else {				\
	    _pw[1].tag.kernel = TNIL;		\
	    Fake_Overflow;			\
	}					\
	MU = _pw;				\
    }

/*---------------------------------------------------------------------------
 * Occur Check
 *---------------------------------------------------------------------------*/

#define OccurCheckEnabled()	(EclGblFlags & OCCUR_CHECK)

#ifdef OC

#ifdef OC_STAT
extern int	occur_check_read_ = 0, occur_check_write_ = 0;
#define OC_Read_Inc			occur_check_read_++;
#define OC_Write_Inc			occur_check_write_++;
#else
#define OC_Read_Inc
#define OC_Write_Inc
#endif /* OC_STAT */

#define Occur_Check_Boundary(p)		OCB = (p);
#define Constructed_Structure(pw)	TCS = (pw);
#define Occur_Check_Read(var, v, t, fail_action)			\
	if (var->val.ptr < OCB && IsCompound(t)) {			\
	    OC_Read_Inc							\
	    if (ec_occurs(var->val, var->tag, v, t))			\
		fail_action;						\
	}
#define Occur_Check_Write(var, fail_action)				\
	if (OCB) {							\
	    register pword	*p = var;				\
	    Occur_Check_Boundary(0);					\
	    Dereference_(p);						\
	    if (IsCompound(p->tag) && TCS) {				\
		OC_Write_Inc						\
		if (occurs_compound(TCS, p)) {				\
		    fail_action;					\
		}							\
	    }								\
	}

#else /* OC */

#define Occur_Check_Boundary(p)
#define Constructed_Structure(pw)
#define Occur_Check_Read(var, v, t, fail_action)
#define Occur_Check_Write(var, fail_action)

#endif /* OC */

/*---------------------------------------------------------------------------
 * Oracle Recording
 *---------------------------------------------------------------------------*/

#define ORC_ALT		1
#define ORC_NTRY	2
#define ORC_NEXT	3
#define ORC_ARITY	3
#define ORC_SIZE	(ORC_ARITY+1)

#define O_SHALLOW	0x00000100
#define O_PAR_ORACLE	0x00000200
#define O_CHK_ORACLE	0x00000400

#define ChpOracle(b)		(Chp(b)->tg - ORC_SIZE)
#define BOracle(b)		(BChp(b)->tg - ORC_SIZE)

#define OPrev(po)		((po)[ORC_NEXT].val.ptr)
#define OAlt(po)		((po)[ORC_ALT].val.nint)
#define OCount(po)		((po)[ORC_NTRY].val.nint)
#define OParallel(po)		((po)->tag.kernel & O_PAR_ORACLE)

#define O_Set_Flag(po,fl)	(po)->tag.kernel |= (fl)
#define O_Clr_Flag(po,fl)	(po)->tag.kernel &= ~(fl)
#define OFlagged(po,fl)		((po)->tag.kernel & (fl))

#define O_Set_Alt(po, alt)	(po)[ORC_ALT].val.nint = (alt);
#define O_Next_Alt(po)		(po)[ORC_ALT].val.nint++;

#define O_Reset_Try_Count(po)	(po)[ORC_NTRY].val.nint = 0;
#define O_Count_Try(po)		(po)[ORC_NTRY].val.nint++;

#define O_Push(n, flags) {					\
	pword *_p = TG;						\
	Push_Struct_Frame(d_.arg);				\
	O_Set_Flag(_p, flags);					\
	_p[ORC_NEXT].val.ptr = TO;				\
	_p[ORC_NEXT].tag.kernel = TO? TCOMP: TNIL;		\
	Make_Integer(_p+ORC_ALT, n);				\
	Make_Integer(_p+ORC_NTRY, 0);				\
	TO = _p;						\
}


#ifdef NEW_ORACLE

#define Record_Alternative(n, flags) {				\
	if (TO) {			/* we are recording */	\
	    if (OFlagged(TO,O_SHALLOW)) {			\
		if (TO > BOracle(B.args)) {			\
		    /* don't oracle shallow cuts */		\
		    TO = TO[ORC_NEXT].val.ptr;	/* pop */	\
		    O_Count_Try(TO);				\
		} else {					\
		    O_Clr_Flag(TO,O_SHALLOW);	/* bury */	\
		}						\
	    }							\
	    O_Push(n, flags);					\
	}							\
}

#define Record_Next_Alternative {				\
	if (TO) {						\
	    TO = BOracle(B.args);				\
	    O_Next_Alt(TO);					\
	    O_Reset_Try_Count(TO);				\
	}							\
}

#define Update_Recorded_Alternative(n) {			\
	if (TO) {						\
	    TO = BOracle(B.args);				\
	    O_Set_Alt(TO, n);					\
	    O_Reset_Try_Count(TO);				\
	}							\
}

#else /* NEW_ORACLE */

#define Record_Alternative(n, flags)
#define Record_Next_Alternative
#define Update_Recorded_Alternative(n)

#endif /* NEW_ORACLE */


/*---------------------------------------------------------------------------
 * Oracle Following
 *---------------------------------------------------------------------------*/

#define NODESIZE	sizeof(st_handle_t)
#define STOPSIZE	1
#define CountSize(i)	(1 + (i)/128)
#define AltSize(i)	((i)<16 ? 1 : 4)

#define ALT_FLAG	1
#define CREATE_FLAG	2
#define PAR_FLAG	4
#define CHK_FLAG	8
#define ALT_SHIFT	4
#define CNT_SHIFT	1

#define Write_Stop(p)	*--(p) = 0;

#define Write_Count(p,n) {				\
	uword _i = n;				        \
	while (_i > 127)				\
	    { *--(p) = 127<<CNT_SHIFT; _i -= 127; }	\
	*--(p) = _i<<CNT_SHIFT;				\
    }

#define FoCount(fo, n)	((n) >> CNT_SHIFT)

/* CAUTION: this scheme cannot handle n==0 */
#define Write_Alt(p, n, fl) {				\
	word _i = (n) < 16 ? (n) : 0;			\
	*--(p) = (_i<<ALT_SHIFT)|(fl)|ALT_FLAG;		\
	if (_i == 0) {					\
	    *--(p) = (n) >> 24;				\
	    *--(p) = (n) >> 16;				\
	    *--(p) = (n) >> 8;				\
	    *--(p) = (n);				\
	}						\
    }

#define FoAlt(fo, n) 					\
	( (n) >> ALT_SHIFT != 0				\
	? (n) >> ALT_SHIFT				\
	: ( (n) = *(fo)++,				\
	    (n) = (n) << 8 | (*(fo)++) & 0xff,		\
	    (n) = (n) << 8 | (*(fo)++) & 0xff,		\
	    (n) = (n) << 8 | (*(fo)++) & 0xff)		\
	)

#define Write_Node(p,node) _write_node(p,node)

#define FoHeader(fo)	(*(fo)++)
#define FoEnd(fo)	FoIsStop(*(fo))
#define FoIsStop(i)	((i) == 0)
#define FoIsCount(i)	(!FoIsAlt(i))
#define FoIsCreate(i)	((i) & CREATE_FLAG)
#define FoIsPar(i)	((i) & PAR_FLAG)
#define FoIsAlt(i)	((i) & ALT_FLAG)
#define FoIsChk(i)	((i) & CHK_FLAG)

#define Fo_Node(fo,dest) fo = read_node(fo, dest)

extern char *read_node();


/*---------------------------------------------------------------------------
 * Global references used in C
 *---------------------------------------------------------------------------*/

#ifdef DFID
#define DfidDepth	(GLOBVAR[1].val.ptr)
#define MaxDepth	(GLOBVAR[2].val.ptr->val.nint)
#define DepthLimit	(GLOBVAR[3].val.ptr->val.nint)
#define DepthOV		(GLOBVAR[4].val.ptr->val.nint)
#endif


/*---------------------------------------------------------------------------
 * Get DID for a type
 *---------------------------------------------------------------------------*/

#define TransfDid(t)	transf_did((word) t)
extern dident transf_did(word);


/*---------------------------------------------------------------------------
 * Tracer
 *---------------------------------------------------------------------------*/

/* Trace frame access  - must correspond to definition in tracer.pl */
#define TF_HEADER	0
#define TF_INVOC	1
#define TF_GOAL		2
#define TF_LEVEL	3
#define TF_CHP_STAMP	4
#define TF_ANCESTOR	5
#define TF_PROC		6
#define TF_PRIO 	7
#define TF_PATH		8
#define TF_LINE		9
#define TF_FROM	       10
#define TF_TO	       11
#define TF_MODULE      12
#define TF_ARITY       12

#define DInvoc(td)	(td)[TF_INVOC].val.nint
#define DGoal(td)	(td)[TF_GOAL]
#define DLevel(td)	(td)[TF_LEVEL].val.nint
#define DAncestor(td)	(td)[TF_ANCESTOR].val.ptr
#define DProc(td)	(td)[TF_PROC].val.priptr
#define DPath(td)	(td)[TF_PATH].val.did
#define DLine(td)	(td)[TF_LINE].val.nint
#define DFrom(td)	(td)[TF_FROM].val.nint
#define DTo(td)		(td)[TF_TO].val.nint

#define Push_Dbg_Frame(pw, tinvoc, vgoal, tgoal, depth, prio, proc, filedid, line, from, to, mod) { \
	pw = TG; \
	Push_Struct_Frame(d_.trace_frame); \
	if (PriFlags(proc) & DEBUG_SK) pw[TF_HEADER].tag.kernel |= TF_SKIPPED; \
	if (!(PriFlags(proc) & DEBUG_DB) && (PriFlags(proc) & DEBUG_TRMETA) ) pw[TF_HEADER].tag.kernel |= TF_TRMETA; \
	Make_Integer(&pw[TF_INVOC], tinvoc); \
	pw[TF_GOAL].val.all = vgoal.all; \
	pw[TF_GOAL].tag.all = tgoal.all; \
	Make_Integer(&pw[TF_LEVEL], (word) (depth)); \
	Make_Stamp(&pw[TF_CHP_STAMP]); \
	pw[TF_ANCESTOR] = TAGGED_TD; \
	pw[TF_PROC].val.priptr = proc; \
	pw[TF_PROC].tag.kernel = TPTR; \
        Make_Integer(&pw[TF_PRIO], (word) (prio)); \
	Make_Atom(&pw[TF_PATH], filedid); \
        Make_Integer(&pw[TF_LINE], (word) (line)); \
        Make_Integer(&pw[TF_FROM], (word) (from)); \
        Make_Integer(&pw[TF_TO], (word) (to)); \
	pw[TF_MODULE].val.did = mod; \
	pw[TF_MODULE].tag.kernel = ModuleTag(mod); \
	Make_Struct(&TAGGED_TD, pw); \
    }

#define Make_Dbg_Frame(pw, tinvoc, vgoal, tgoal, depth, prio, proc, filedid, line, from, to, mod) { \
	pw = TG; \
	Push_Struct_Frame(d_.trace_frame); \
	if (PriFlags(proc) & DEBUG_SK) pw[TF_HEADER].tag.kernel |= TF_SKIPPED; \
	if (!(PriFlags(proc) & DEBUG_DB) && (PriFlags(proc) & DEBUG_TRMETA) ) pw[TF_HEADER].tag.kernel |= TF_TRMETA; \
	Make_Integer(&pw[TF_INVOC], tinvoc); \
	pw[TF_GOAL].val.all = vgoal.all; \
	pw[TF_GOAL].tag.all = tgoal.all; \
	Make_Integer(&pw[TF_LEVEL], (word) (depth)); \
	Make_Stamp(&pw[TF_CHP_STAMP]); \
	Make_Var(&pw[TF_ANCESTOR]); \
	pw[TF_PROC].val.priptr = proc; \
	pw[TF_PROC].tag.kernel = TPTR; \
        Make_Integer(&pw[TF_PRIO], (word) (prio)); \
	Make_Atom(&pw[TF_PATH], filedid); \
        Make_Integer(&pw[TF_LINE], (word) (line)); \
        Make_Integer(&pw[TF_FROM], (word) (from)); \
        Make_Integer(&pw[TF_TO], (word) (to)); \
	pw[TF_MODULE].val.did = mod; \
	pw[TF_MODULE].tag.kernel = ModuleTag(mod); \
    }

#define Make_Partial_Dbg_Frame(pw, tinvoc, goal, prio, proc, filedid, line, from, to, mod) { \
	pw = TG; \
	Push_Struct_Frame(d_.trace_frame); \
	if (PriFlags(proc) & DEBUG_SK) pw[TF_HEADER].tag.kernel |= TF_SKIPPED; \
	if (!(PriFlags(proc) & DEBUG_DB) && (PriFlags(proc) & DEBUG_TRMETA) ) pw[TF_HEADER].tag.kernel |= TF_TRMETA; \
	Make_Integer(&pw[TF_INVOC], tinvoc); \
	pw[TF_GOAL] = (goal); \
	Make_Var(&pw[TF_LEVEL]); \
	Make_Stamp(&pw[TF_CHP_STAMP]); \
	Make_Var(&pw[TF_ANCESTOR]); \
	pw[TF_PROC].val.priptr = proc; \
	pw[TF_PROC].tag.kernel = TPTR; \
        Make_Integer(&pw[TF_PRIO], (word) (prio)); \
	Make_Atom(&pw[TF_PATH], filedid); \
        Make_Integer(&pw[TF_LINE], (word) (line)); \
        Make_Integer(&pw[TF_FROM], (word) (from)); \
        Make_Integer(&pw[TF_TO], (word) (to)); \
	pw[TF_MODULE].val.did = mod; \
	pw[TF_MODULE].tag.kernel = ModuleTag(mod); \
    }

#define Pop_Dbg_Frame() { \
	if (TD < GB) { Trail_Pword(&TAGGED_TD); } \
	TAGGED_TD = TD[TF_ANCESTOR]; \
    }

/*
 * OfInterest is true if:
 * - the predicate's DEBUG_TR|DEBUG_SP flags are the same as the tracer's
 *   TR_TRACING|TR_LEAPING flags, i.e. in creep mode all traceable preds
 *   match, in leap mode only traceable ones with spy points
 * *or*
 *  tracer is in leap mode and we are at a breakpoint
 * 
 * - depth is in selected range
 * - invoc is in selected range
 */
#define OfInterest(flags, invoc, depth, brkpt) \
	( (!((((flags) & TRACEMODE) ^ TRACEMODE) & (TR_TRACING|TR_LEAPING)) \
        || ((brkpt) && (TRACEMODE & TR_LEAPING))) \
	&& JMINLEVEL <= (depth) && (depth) <= JMAXLEVEL \
	&& JMININVOC <= (invoc) && (invoc) <= JMAXINVOC )

/*
 * Init the tracer state. The TR_STARTED flag is used to trigger raising
 * of the DEBUG_INIT_EVENT, and is then reset (see raise_init_event/0).
 */
#define TracerInit \
	NINVOC = RLEVEL = FDROP = JMININVOC = 0; \
	JMINLEVEL = 0; JMAXLEVEL = MAX_DEPTH; JMAXINVOC = MAX_INVOC; \
	PORTFILTER = ANY_NOTIFIES; \
	TRACEMODE = TR_TRACING|TR_STARTED;

/* Flag in debug-event save frame */
#define WAS_CALL	(SIGN_BIT >> 3)
#define WAS_NONDET	(SIGN_BIT >> 4)

/* Tracer constants */
#define MAX_INVOC	MAX_S_WORD
#define MAX_DEPTH	MAX_S_WORD
#define MAX_FAILTRACE	1024

/* Trace frame flags */
#define TF_SKIPPED	0x0100	/* it is a skipped procedure's frame	*/
#define TF_INTRACER	0x0200	/* we are currently inside tracer code	*/
#define TF_NOGOAL	0x0400	/* frame's goal/module field is invalid	*/
#define TF_REDO		0x0800	/* we are tracing a REDO (retry/trust)	*/
#define TF_BREAK	0x1000	/* this frame's CALL had a breakpoint	*/
#define TF_SYSTRACE	0x2000	/* abstract instruction trace disabled	*/
#define TF_SIMPLE	0x4000	/* it is a simple goal's trace frame	*/
#define TF_TRMETA	0x8000	/* trace metacalled subgoals		*/

#define TfFlags(td)		(td)[TF_HEADER].tag.kernel
#define Set_Tf_Flag(td,flag)	{ TfFlags(td) |= (flag); }
#define Clr_Tf_Flag(td,flag)	{ TfFlags(td) &= ~(flag); }
#define Flip_Tf_Flag(td,flag)	{ TfFlags(td) ^= (flag); }

#define Unskipped(td)	((TfFlags(td) & (TF_SKIPPED|TF_INTRACER)) == 0)
#define Tracing		(TD && Unskipped(TD))
#define TracingWakes(invoc)	(!(TfFlags(TD) & (TF_INTRACER)) && (!(TfFlags(TD) & TF_SKIPPED) || (invoc)))
#define TracingMetacalls(port)	(Unskipped(TD) && (TfFlags(TD) & TF_TRMETA))


/*---------------------------------------------------------------------------
 * Engines
 *---------------------------------------------------------------------------*/

#define ENG_MIN_LOCAL	64	/* kbytes */
#define ENG_MIN_GLOBAL	256	/* kbytes */

/* Resume types */
#define RESUME_CONT	0
#define RESUME_SIMPLE	1

/* Option argument for ecl_resume_goal() */
#define GOAL_CALL	0
#define GOAL_CUT	1
#define GOAL_NOTNOT	2
#define GOAL_CUTFAIL	3
#define GOAL_CATCH	4	/* bit-significant */

#define YIELD_ARITY	4	/* valid arguments when engine yielded */

#define PAUSE_GENERAL		0
#define PAUSE_CONDITION_WAIT	1
#define PAUSE_SELECT		2
#define PAUSE_SLEEP		3

#define EngIsDead(eng) 			(!((eng)->tg))	/* no stacks */
#define EngIsFree(eng)			(!(eng)->owner_thread)
#define EngIsOurs(eng)			((eng)->owner_thread == ec_thread_self())
#define EngIsPaused(eng)		((eng)->paused)
#define EngPauseArity(eng)		((eng)->paused>>3)
#define EngPauseKind(eng)		((eng)->paused & 0x3)
#define PauseType(arity,kind)		(((arity)<<3)|0x4|kind)

#define EngPrintId(eng)	((word)eng/0x1000)
#define EngLogMsg(eng,msg,...) {\
    if ((eng)->vm_flags & ENG_VERBOSE) {\
 	ec_printff(log_output_, "Engine %x: " msg "\n", EngPrintId(eng), __VA_ARGS__);\
    }\
}

/*---------------------------------------------------------------------------
 * Cleanup mechanism
 * We have a stack, implemented as a doubly linked list of elements.
 * The elements contain a function pointer f and an object pointer p.
 * Require_Cleanup(f,p) pushes an element onto the stack.
 * Do_Cleanup() calls the cleanup functions f(p) for all elements in
 * reverse order, with the object pointer as argument.
 * There is always one unused list element, pointed to by ec_eng->cleanup.
 * New elements are allocated when needed, but only deallocated finally.
 * ->down points down the stack, to used elements
 * ->up points up to (further) currently unused elements
 *---------------------------------------------------------------------------*/

#define Init_Cleanup()\
	ec_eng->cleanup_bot = ec_eng->cleanup = (action_list_t*) hp_alloc_size(sizeof(action_list_t));\
	ec_eng->cleanup->down = NULL;\
	ec_eng->cleanup->up = NULL;

#define NoCleanup(eng)\
	((eng)->cleanup == (eng)->cleanup_bot)

#define Require_Cleanup(fun,ptr) {\
	action_list_t *pa = ec_eng->cleanup;\
	pa->action = (void(*)(void*))(fun);\
	pa->thing = (void*)(ptr);\
	pa = pa->up;\
	if (!pa) {\
	    pa = (action_list_t*) hp_alloc_size(sizeof(action_list_t));\
	    pa->down = ec_eng->cleanup;\
	    pa->up = NULL;\
	    ec_eng->cleanup->up = pa;\
	}\
	ec_eng->cleanup = pa;\
    }

#define Do_Cleanup() {\
	action_list_t *pa = ec_eng->cleanup;\
	/* cleanup until current bottom */\
	while (pa != ec_eng->cleanup_bot) {\
	    pa = pa->down;\
	    pa->action(pa->thing);\
	}\
	ec_eng->cleanup = pa;\
    }

#define Fini_Cleanup() {\
	action_list_t *pa = ec_eng->cleanup;\
	/* cleanup any remaining entries */\
	while (pa->down) {\
	    pa = pa->down;\
	    pa->action(pa->thing);\
	}\
	/* free the whole list */\
	while(pa) {\
	    action_list_t *ptmp = pa;\
	    pa = pa->up;\
	    hp_free_size(ptmp, sizeof(action_list_t));\
	}\
	ec_eng->cleanup_bot = ec_eng->cleanup = NULL;\
    }


/* common special cases */
#define Require_Unlock(plock)\
	Require_Cleanup(ec_cleanup_unlock, plock)

#define Require_Release(ext_type_desc, pobject)\
	Require_Cleanup((ext_type_desc).free, pobject)


/*---------------------------------------------------------------------------
 * Arithmetic comparisons, for arith_compare()
 *---------------------------------------------------------------------------*/

#define BILt	1
#define BIGt	2
#define BILe	3
#define BIGe	4
#define BIEq	5
#define BINe	6
#define BILeGe	7	/* =< or >=, needed for sorting */


/*---------------------------------------------------------------------------
 * Prototypes
 *---------------------------------------------------------------------------*/

Extern	void	ec_exit(int);
Extern	void	re_fake_overflow(ec_eng_t*);
Extern	int	sub_emulc_opt(ec_eng_t*, value, type, value, type, int);
Extern	int	ecl_subgoal(ec_eng_t*, pword, pword, int);
Extern	int	boot_emulc(ec_eng_t*, value, type, value, type);
Extern	int	slave_emulc(ec_eng_t*);
Extern	int	resume_emulc(ec_eng_t*);
Extern	int	eclipse_main(ec_eng_t *ec_eng, int startup);
Extern	int	emu_init(ec_eng_t*, ec_eng_t*);
Extern	void	ec_emu_fini(ec_eng_t*);
Extern	void	destroy_parser_env(ec_eng_t*);

Extern	DLLEXP	int ecl_engine_init(ec_eng_t *parent_eng, ec_eng_t *new_eng);
Extern	int	ecl_engines_init(t_eclipse_options*, ec_eng_t **);
Extern	int	ecl_init_aux(t_eclipse_options *, ec_eng_t *, int);
Extern	void	ecl_engine_exit(ec_eng_t*, int);
Extern	int	ecl_pause_engine(ec_eng_t *ec_eng, int arity, int kind, void*, void*);
Extern	void	ecl_interrupt_pause(ec_eng_t *ec_eng);
Extern	void	ecl_unpause_engine(ec_eng_t *ec_eng);
Extern	int	ecl_housekeeping(ec_eng_t*, word valid_args);
Extern	int	ecl_do_requested_action(ec_eng_t*,int event_flags,int jump);
Extern	int	ecl_copy_resume(ec_eng_t *from_eng, ec_eng_t *ec_eng, const pword term, const pword module);
Extern	int	ecl_copy_resume_async(ec_eng_t *from_eng, ec_eng_t *ec_eng, const pword term, const pword module);
Extern  int     ecl_engine_clone(ec_eng_t* from_eng, ec_eng_t* to_eng, int arity);

Extern	void	delayed_exit(ec_eng_t*);
Extern	int	next_posted_item(ec_eng_t*, pword*, int);
Extern	int	next_urgent_event(ec_eng_t*);
Extern	void	ec_fini_dynamic_event_queue(ec_eng_t*);

Extern	int	deep_suspend(ec_eng_t*, value, type, int, pword*, int);
Extern	DLLEXP	pword *	add_attribute(ec_eng_t*, word, pword*, word, int);
Extern	DLLEXP	int	insert_suspension(ec_eng_t*, pword*, int, pword*, int);

#define	notify_constrained(p) ecl_notify_constrained(ec_eng,p)
Extern	DLLEXP	int	ecl_notify_constrained(ec_eng_t*,pword*);

Extern	pword *	first_woken(ec_eng_t*, int);
Extern	pword *	wl_init(ec_eng_t*);
Extern	DLLEXP	int 	bind_c(ec_eng_t*, pword*, pword*, pword**);
Extern	int 	meta_bind(ec_eng_t*, pword*, value, type);

#define ec_schedule_susps(p) ecl_schedule_susps(ec_eng,p)
Extern	DLLEXP	int 	ecl_schedule_susps(ec_eng_t*,pword*);

#define ec_double_to_int_or_bignum(d,p) ecl_double_to_int_or_bignum(ec_eng,d,p)
Extern	DLLEXP	int ecl_double_to_int_or_bignum(ec_eng_t*, double, pword *);

#define ec_keysort(l,vk,tk,r,d,n,e) ecl_keysort(ec_eng,l,vk,tk,r,d,n,e)
Extern	pword *	ecl_keysort(ec_eng_t*, value, value, type, int, int, int, int *);

Extern	pword *	ec_nonground(value, type);

Extern	void	untrail_ext(ec_eng_t*,pword**,int);
Extern	void	do_cut_action(ec_eng_t*);
Extern	void	ecl_longjmp_throw(ec_eng_t *ec_eng, value v_tag, type t_tag);
Extern	int	ecl_return_throw(ec_eng_t *ec_eng, value v_tag, type t_tag);

Extern	dident	meta_name(int);
Extern	DLLEXP	int	meta_index(dident);
Extern	int	p_schedule_woken(value, type, ec_eng_t*);
Extern	DLLEXP	int	p_schedule_postponed(value, type, ec_eng_t*);
Extern	int	ec_compare_terms(value, type, value, type);
Extern	int	trim_global_trail(ec_eng_t*,uword);
Extern	int	trim_control_local(ec_eng_t*);
Extern	void	mark_dids_from_pwords(pword *from, register pword *to);
Extern	int	ec_occurs(value vs, type ts, value vterm, type tterm);
Extern	void	ec_init_dynamic_event_queue(ec_eng_t*);
Extern	void	trim_dynamic_event_queue(ec_eng_t*);
Extern	void	purge_disabled_dynamic_events(ec_eng_t*, t_heap_event *event);
Extern	void	mark_dids_dynamic_event_queue(ec_eng_t*);
Extern	DLLEXP	int p_merge_suspension_lists(value, type, value, type, value, type, value, type, ec_eng_t*);
Extern	DLLEXP	int p_set_suspension_priority(value, type, value, type, ec_eng_t*);

#define ec_enter_suspension(t,s) ecl_enter_suspension(ec_eng,t,s)
Extern	DLLEXP	int ecl_enter_suspension(ec_eng_t*,pword *, pword *);

/* from handlers.c */
Extern	int	ec_sigio;
Extern	int	ec_signalnum(value vsig, type tsig);
Extern	int	ec_thread_reinstall_handlers(void*);
Extern	void	ec_send_signal(int);
Extern	void	ec_signal_dict_gc(void);

/* from bip_arith.c */
Extern	DLLEXP	int unary_arith_op(value,type,value,type,ec_eng_t*,int,int);
Extern	int	binary_arith_op(value,type,value,type,value,type,ec_eng_t*,int);
Extern	int	un_arith_op(value,type,pword *,ec_eng_t*,int,int);
Extern	int	bin_arith_op(value,type,value,type,pword *,ec_eng_t*,int);
Extern	int	arith_compare(ec_eng_t*, value v1, type t1, value v2, type t2, int *res);

/* from bip_tconv.c */
Extern	pword * ec_chase_arg(value vn, type tn, value vt, type tt, int *perr);
Extern	uword	ec_term_hash(value vterm, type tterm, uword maxdepth, int *pres);

/* from bigrat.c */
Extern	void	bigrat_init(void);
Extern	int	ec_array_to_big(ec_eng_t *ec_eng, const void *p, int count, int order, int size, int endian, unsigned nails, pword *result);
Extern	int	ec_big_to_chunks(ec_eng_t *ec_eng, pword *pw1, uword chunksize, pword *result);

/* from bip_array.c */
Extern	int	make_kernel_array(ec_eng_t *ec_eng, dident adid, int length, dident atype, dident avisib);
Extern	pword *	get_kernel_array(dident adid);
Extern	uword *	get_elt_address(value v, type t, uword *kind, dident mod_did, type mod_tag, int *perr);
Extern	word	get_first_elt(pword *p, pword *q, uword *kind, uword *size, dident vmod_did, type mod_tag);
Extern	void	ec_free_array(pword *prop_value);

/* from init.c */
Extern	int	eclipse_global_init(int init_flags);
Extern	int	eclipse_boot(ec_eng_t*, char *initfile);

/* from gc_stacks.c */
Extern	int	collect_stacks(ec_eng_t *ec_eng, word arity, word gc_forced);
Extern	void	mark_dids_from_stacks(ec_eng_t *ec_eng, word arity);

/* from bip_delay.c */
Extern	int	ecl_prune_suspensions(ec_eng_t *, pword *);
Extern	int	ec_init_postponed(ec_eng_t*);

/* from handle.c */
Extern	void	handle_copy_anchor(pword*,pword*,ec_eng_t*);

/* from engines.c */
Extern	t_ext_type engine_tid;
Extern	int	ecl_free_engine(ec_eng_t *ec_eng, int locked);
Extern	ec_eng_t * ecl_copy_engine(ec_eng_t *ec_eng);
Extern	ec_eng_t * ecl_resurrect_engine(ec_eng_t *ec_eng);
Extern	void	ec_embed_fini(void);

/* from term_copy.c */
Extern	int	ec_copy_term_across(ec_eng_t *from_eng, ec_eng_t *ec_eng, value v, type t, pword *dest, int with_attributes);
Extern	int	unreference_embedded_handle(t_ext_ptr handle, pword *root);
Extern	void	get_heapterm(ec_eng_t*,pword*, pword*);
Extern	int	create_heapterm(ec_eng_t*, pword*, value, type);
Extern	int	create_heapterm_simple(pword*, pword);
Extern	int	create_heapterm_for_handle(pword*, t_ext_type*, t_ext_ptr);
Extern	void	free_heapterm(pword*);
Extern	void	move_heapterm(pword*, pword*);
Extern	void	make_heapterm_persistent(pword*);
Extern	void	mark_dids_from_heapterm(pword*);
Extern	void	set_string(pword*, char*);
Extern	void	set_string_n(pword*, char*, int);

/* from bip_load.c */
Extern	void	bip_load_fini(void);

/* from bip_random.c */
Extern	void	ec_random_init(ec_eng_t *ec_eng);

/* from bip_module.c */
Extern	int	ec_create_module(dident module_did);

/* from bip_mps.c */
Extern int	mps_present(void);

/* from bip_heapevents.c */
Extern	t_ext_type heap_event_tid;
Extern	t_ext_ptr ec_new_heap_event(value vgoal, type tgoal, value vm, type tm, int defers);

/* from bip_record.c */
Extern t_ext_type heap_rec_header_tid;
Extern t_ext_ptr ec_record_create(void);
Extern int	 ec_record_append(t_ext_ptr, t_ext_type*, t_ext_ptr);

/* from bip_shelf.c */
Extern t_ext_type heap_array_tid;
Extern	int ecl_shelf_create(value v, type t, pword *pshelf, ec_eng_t *ec_eng);

/* from bip_store.c */
Extern t_ext_type heap_htable_tid;

/* from emu.c */
Extern func_ptr ec_emulate(ec_eng_t*);
Extern func_ptr ec_emulate_profile(ec_eng_t*);

/* from emu_par.c, dummy_par.c */
Extern int	par_present(void);
Extern int	cut_public(void);
Extern int	io_rpc(stream_id, int);
Extern void	halt_system(/*unspecified*/);

/* from printam.c */
Extern	vmcode * print_am(vmcode *, vmcode **, vmcode *, int *, int);
Extern	void	print_instr(vmcode *, int);
Extern	int	get_int_opcode(vmcode *code);

/* from procedure.c */
Extern	int	ec_load_eco_from_stream(ec_eng_t *ec_eng, stream_id nst, int options, pword *module);

/* from intervals.c */
Extern	void	ec_intervals_init(void);
