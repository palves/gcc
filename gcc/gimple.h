/* Gimple IR definitions.

   Copyright (C) 2007-2014 Free Software Foundation, Inc.
   Contributed by Aldy Hernandez <aldyh@redhat.com>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef GCC_GIMPLE_H
#define GCC_GIMPLE_H

typedef gimple gimple_seq_node;

/* For each block, the PHI nodes that need to be rewritten are stored into
   these vectors.  */
typedef vec<gimple> gimple_vec;

enum gimple_code {
#define DEFGSCODE(SYM, STRING, STRUCT)	SYM,
#include "gimple.def"
#undef DEFGSCODE
    LAST_AND_UNUSED_GIMPLE_CODE
};

extern const char *const gimple_code_name[];
extern const unsigned char gimple_rhs_class_table[];

/* Error out if a gimple tuple is addressed incorrectly.  */
#if defined ENABLE_GIMPLE_CHECKING
#define gcc_gimple_checking_assert(EXPR) gcc_assert (EXPR)
extern void gimple_check_failed (const_gimple, const char *, int,          \
                                 const char *, enum gimple_code,           \
				 enum tree_code) ATTRIBUTE_NORETURN;

#define GIMPLE_CHECK(GS, CODE)						\
  do {									\
    const_gimple __gs = (GS);						\
    if (gimple_code (__gs) != (CODE))					\
      gimple_check_failed (__gs, __FILE__, __LINE__, __FUNCTION__,	\
	  		   (CODE), ERROR_MARK);				\
  } while (0)
#else  /* not ENABLE_GIMPLE_CHECKING  */
#define gcc_gimple_checking_assert(EXPR) ((void)(0 && (EXPR)))
#define GIMPLE_CHECK(GS, CODE)			(void)0
#endif

/* Class of GIMPLE expressions suitable for the RHS of assignments.  See
   get_gimple_rhs_class.  */
enum gimple_rhs_class
{
  GIMPLE_INVALID_RHS,	/* The expression cannot be used on the RHS.  */
  GIMPLE_TERNARY_RHS,	/* The expression is a ternary operation.  */
  GIMPLE_BINARY_RHS,	/* The expression is a binary operation.  */
  GIMPLE_UNARY_RHS,	/* The expression is a unary operation.  */
  GIMPLE_SINGLE_RHS	/* The expression is a single object (an SSA
			   name, a _DECL, a _REF, etc.  */
};

/* Specific flags for individual GIMPLE statements.  These flags are
   always stored in gimple_statement_base.subcode and they may only be
   defined for statement codes that do not use subcodes.

   Values for the masks can overlap as long as the overlapping values
   are never used in the same statement class.

   The maximum mask value that can be defined is 1 << 15 (i.e., each
   statement code can hold up to 16 bitflags).

   Keep this list sorted.  */
enum gf_mask {
    GF_ASM_INPUT		= 1 << 0,
    GF_ASM_VOLATILE		= 1 << 1,
    GF_CALL_FROM_THUNK		= 1 << 0,
    GF_CALL_RETURN_SLOT_OPT	= 1 << 1,
    GF_CALL_TAILCALL		= 1 << 2,
    GF_CALL_VA_ARG_PACK		= 1 << 3,
    GF_CALL_NOTHROW		= 1 << 4,
    GF_CALL_ALLOCA_FOR_VAR	= 1 << 5,
    GF_CALL_INTERNAL		= 1 << 6,
    GF_OMP_PARALLEL_COMBINED	= 1 << 0,
    GF_OMP_FOR_KIND_MASK	= (1 << 2) - 1,
    GF_OMP_FOR_KIND_FOR		= 0,
    GF_OMP_FOR_KIND_DISTRIBUTE	= 1,
    /* Flag for SIMD variants of OMP_FOR kinds.  */
    GF_OMP_FOR_SIMD		= 1 << 1,
    GF_OMP_FOR_KIND_SIMD	= GF_OMP_FOR_SIMD | 0,
    GF_OMP_FOR_KIND_CILKSIMD	= GF_OMP_FOR_SIMD | 1,
    GF_OMP_FOR_COMBINED		= 1 << 2,
    GF_OMP_FOR_COMBINED_INTO	= 1 << 3,
    GF_OMP_TARGET_KIND_MASK	= (1 << 2) - 1,
    GF_OMP_TARGET_KIND_REGION	= 0,
    GF_OMP_TARGET_KIND_DATA	= 1,
    GF_OMP_TARGET_KIND_UPDATE	= 2,

    /* True on an GIMPLE_OMP_RETURN statement if the return does not require
       a thread synchronization via some sort of barrier.  The exact barrier
       that would otherwise be emitted is dependent on the OMP statement with
       which this return is associated.  */
    GF_OMP_RETURN_NOWAIT	= 1 << 0,

    GF_OMP_SECTION_LAST		= 1 << 0,
    GF_OMP_ATOMIC_NEED_VALUE	= 1 << 0,
    GF_OMP_ATOMIC_SEQ_CST	= 1 << 1,
    GF_PREDICT_TAKEN		= 1 << 15
};

/* Currently, there are only two types of gimple debug stmt.  Others are
   envisioned, for example, to enable the generation of is_stmt notes
   in line number information, to mark sequence points, etc.  This
   subcode is to be used to tell them apart.  */
enum gimple_debug_subcode {
  GIMPLE_DEBUG_BIND = 0,
  GIMPLE_DEBUG_SOURCE_BIND = 1
};

/* Masks for selecting a pass local flag (PLF) to work on.  These
   masks are used by gimple_set_plf and gimple_plf.  */
enum plf_mask {
    GF_PLF_1	= 1 << 0,
    GF_PLF_2	= 1 << 1
};

/* Data structure definitions for GIMPLE tuples.  NOTE: word markers
   are for 64 bit hosts.  */

struct GTY((desc ("gimple_statement_structure (&%h)"), tag ("GSS_BASE"),
	    chain_next ("%h.next"), variable_size))
  gimple_statement_base
{
  /* [ WORD 1 ]
     Main identifying code for a tuple.  */
  ENUM_BITFIELD(gimple_code) code : 8;

  /* Nonzero if a warning should not be emitted on this tuple.  */
  unsigned int no_warning	: 1;

  /* Nonzero if this tuple has been visited.  Passes are responsible
     for clearing this bit before using it.  */
  unsigned int visited		: 1;

  /* Nonzero if this tuple represents a non-temporal move.  */
  unsigned int nontemporal_move	: 1;

  /* Pass local flags.  These flags are free for any pass to use as
     they see fit.  Passes should not assume that these flags contain
     any useful value when the pass starts.  Any initial state that
     the pass requires should be set on entry to the pass.  See
     gimple_set_plf and gimple_plf for usage.  */
  unsigned int plf		: 2;

  /* Nonzero if this statement has been modified and needs to have its
     operands rescanned.  */
  unsigned modified 		: 1;

  /* Nonzero if this statement contains volatile operands.  */
  unsigned has_volatile_ops 	: 1;

  /* Padding to get subcode to 16 bit alignment.  */
  unsigned pad			: 1;

  /* The SUBCODE field can be used for tuple-specific flags for tuples
     that do not require subcodes.  Note that SUBCODE should be at
     least as wide as tree codes, as several tuples store tree codes
     in there.  */
  unsigned int subcode		: 16;

  /* UID of this statement.  This is used by passes that want to
     assign IDs to statements.  It must be assigned and used by each
     pass.  By default it should be assumed to contain garbage.  */
  unsigned uid;

  /* [ WORD 2 ]
     Locus information for debug info.  */
  location_t location;

  /* Number of operands in this tuple.  */
  unsigned num_ops;

  /* [ WORD 3 ]
     Basic block holding this statement.  */
  basic_block bb;

  /* [ WORD 4-5 ]
     Linked lists of gimple statements.  The next pointers form
     a NULL terminated list, the prev pointers are a cyclic list.
     A gimple statement is hence also a double-ended list of
     statements, with the pointer itself being the first element,
     and the prev pointer being the last.  */
  gimple next;
  gimple GTY((skip)) prev;
};


/* Base structure for tuples with operands.  */

/* This gimple subclass has no tag value.  */
struct GTY(())
  gimple_statement_with_ops_base : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7 ]
     SSA operand vectors.  NOTE: It should be possible to
     amalgamate these vectors with the operand vector OP.  However,
     the SSA operand vectors are organized differently and contain
     more information (like immediate use chaining).  */
  struct use_optype_d GTY((skip (""))) *use_ops;
};


/* Statements that take register operands.  */

struct GTY((tag("GSS_WITH_OPS")))
  gimple_statement_with_ops : public gimple_statement_with_ops_base
{
  /* [ WORD 1-7 ] : base class */

  /* [ WORD 8 ]
     Operand vector.  NOTE!  This must always be the last field
     of this structure.  In particular, this means that this
     structure cannot be embedded inside another one.  */
  tree GTY((length ("%h.num_ops"))) op[1];
};


/* Base for statements that take both memory and register operands.  */

struct GTY((tag("GSS_WITH_MEM_OPS_BASE")))
  gimple_statement_with_memory_ops_base : public gimple_statement_with_ops_base
{
  /* [ WORD 1-7 ] : base class */

  /* [ WORD 8-9 ]
     Virtual operands for this statement.  The GC will pick them
     up via the ssa_names array.  */
  tree GTY((skip (""))) vdef;
  tree GTY((skip (""))) vuse;
};


/* Statements that take both memory and register operands.  */

struct GTY((tag("GSS_WITH_MEM_OPS")))
  gimple_statement_with_memory_ops :
    public gimple_statement_with_memory_ops_base
{
  /* [ WORD 1-9 ] : base class */

  /* [ WORD 10 ]
     Operand vector.  NOTE!  This must always be the last field
     of this structure.  In particular, this means that this
     structure cannot be embedded inside another one.  */
  tree GTY((length ("%h.num_ops"))) op[1];
};


/* Call statements that take both memory and register operands.  */

struct GTY((tag("GSS_CALL")))
  gimple_statement_call : public gimple_statement_with_memory_ops_base
{
  /* [ WORD 1-9 ] : base class */

  /* [ WORD 10-13 ]  */
  struct pt_solution call_used;
  struct pt_solution call_clobbered;

  /* [ WORD 14 ]  */
  union GTY ((desc ("%1.subcode & GF_CALL_INTERNAL"))) {
    tree GTY ((tag ("0"))) fntype;
    enum internal_fn GTY ((tag ("GF_CALL_INTERNAL"))) internal_fn;
  } u;

  /* [ WORD 15 ]
     Operand vector.  NOTE!  This must always be the last field
     of this structure.  In particular, this means that this
     structure cannot be embedded inside another one.  */
  tree GTY((length ("%h.num_ops"))) op[1];
};


/* OpenMP statements (#pragma omp).  */

struct GTY((tag("GSS_OMP")))
  gimple_statement_omp : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7 ]  */
  gimple_seq body;
};


/* GIMPLE_BIND */

struct GTY((tag("GSS_BIND")))
  gimple_statement_bind : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7 ]
     Variables declared in this scope.  */
  tree vars;

  /* [ WORD 8 ]
     This is different than the BLOCK field in gimple_statement_base,
     which is analogous to TREE_BLOCK (i.e., the lexical block holding
     this statement).  This field is the equivalent of BIND_EXPR_BLOCK
     in tree land (i.e., the lexical scope defined by this bind).  See
     gimple-low.c.  */
  tree block;

  /* [ WORD 9 ]  */
  gimple_seq body;
};


/* GIMPLE_CATCH */

struct GTY((tag("GSS_CATCH")))
  gimple_statement_catch : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7 ]  */
  tree types;

  /* [ WORD 8 ]  */
  gimple_seq handler;
};


/* GIMPLE_EH_FILTER */

struct GTY((tag("GSS_EH_FILTER")))
  gimple_statement_eh_filter : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7 ]
     Filter types.  */
  tree types;

  /* [ WORD 8 ]
     Failure actions.  */
  gimple_seq failure;
};

/* GIMPLE_EH_ELSE */

struct GTY((tag("GSS_EH_ELSE")))
  gimple_statement_eh_else : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7,8 ] */
  gimple_seq n_body, e_body;
};

/* GIMPLE_EH_MUST_NOT_THROW */

struct GTY((tag("GSS_EH_MNT")))
  gimple_statement_eh_mnt : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7 ] Abort function decl.  */
  tree fndecl;
};

/* GIMPLE_PHI */

struct GTY((tag("GSS_PHI")))
  gimple_statement_phi : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7 ]  */
  unsigned capacity;
  unsigned nargs;

  /* [ WORD 8 ]  */
  tree result;

  /* [ WORD 9 ]  */
  struct phi_arg_d GTY ((length ("%h.nargs"))) args[1];
};


/* GIMPLE_RESX, GIMPLE_EH_DISPATCH */

struct GTY((tag("GSS_EH_CTRL")))
  gimple_statement_eh_ctrl : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7 ]
     Exception region number.  */
  int region;
};

struct GTY((tag("GSS_EH_CTRL")))
  gimple_statement_resx : public gimple_statement_eh_ctrl
{
  /* No extra fields; adds invariant:
       stmt->code == GIMPLE_RESX.  */
};

struct GTY((tag("GSS_EH_CTRL")))
  gimple_statement_eh_dispatch : public gimple_statement_eh_ctrl
{
  /* No extra fields; adds invariant:
       stmt->code == GIMPLE_EH_DISPATH.  */
};


/* GIMPLE_TRY */

struct GTY((tag("GSS_TRY")))
  gimple_statement_try : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7 ]
     Expression to evaluate.  */
  gimple_seq eval;

  /* [ WORD 8 ]
     Cleanup expression.  */
  gimple_seq cleanup;
};

/* Kind of GIMPLE_TRY statements.  */
enum gimple_try_flags
{
  /* A try/catch.  */
  GIMPLE_TRY_CATCH = 1 << 0,

  /* A try/finally.  */
  GIMPLE_TRY_FINALLY = 1 << 1,
  GIMPLE_TRY_KIND = GIMPLE_TRY_CATCH | GIMPLE_TRY_FINALLY,

  /* Analogous to TRY_CATCH_IS_CLEANUP.  */
  GIMPLE_TRY_CATCH_IS_CLEANUP = 1 << 2
};

/* GIMPLE_WITH_CLEANUP_EXPR */

struct GTY((tag("GSS_WCE")))
  gimple_statement_wce : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* Subcode: CLEANUP_EH_ONLY.  True if the cleanup should only be
	      executed if an exception is thrown, not on normal exit of its
	      scope.  This flag is analogous to the CLEANUP_EH_ONLY flag
	      in TARGET_EXPRs.  */

  /* [ WORD 7 ]
     Cleanup expression.  */
  gimple_seq cleanup;
};


/* GIMPLE_ASM  */

struct GTY((tag("GSS_ASM")))
  gimple_statement_asm : public gimple_statement_with_memory_ops_base
{
  /* [ WORD 1-9 ] : base class */

  /* [ WORD 10 ]
     __asm__ statement.  */
  const char *string;

  /* [ WORD 11 ]
       Number of inputs, outputs, clobbers, labels.  */
  unsigned char ni;
  unsigned char no;
  unsigned char nc;
  unsigned char nl;

  /* [ WORD 12 ]
     Operand vector.  NOTE!  This must always be the last field
     of this structure.  In particular, this means that this
     structure cannot be embedded inside another one.  */
  tree GTY((length ("%h.num_ops"))) op[1];
};

/* GIMPLE_OMP_CRITICAL */

struct GTY((tag("GSS_OMP_CRITICAL")))
  gimple_statement_omp_critical : public gimple_statement_omp
{
  /* [ WORD 1-7 ] : base class */

  /* [ WORD 8 ]
     Critical section name.  */
  tree name;
};


struct GTY(()) gimple_omp_for_iter {
  /* Condition code.  */
  enum tree_code cond;

  /* Index variable.  */
  tree index;

  /* Initial value.  */
  tree initial;

  /* Final value.  */
  tree final;

  /* Increment.  */
  tree incr;
};

/* GIMPLE_OMP_FOR */

struct GTY((tag("GSS_OMP_FOR")))
  gimple_statement_omp_for : public gimple_statement_omp
{
  /* [ WORD 1-7 ] : base class */

  /* [ WORD 8 ]  */
  tree clauses;

  /* [ WORD 9 ]
     Number of elements in iter array.  */
  size_t collapse;

  /* [ WORD 10 ]  */
  struct gimple_omp_for_iter * GTY((length ("%h.collapse"))) iter;

  /* [ WORD 11 ]
     Pre-body evaluated before the loop body begins.  */
  gimple_seq pre_body;
};


/* GIMPLE_OMP_PARALLEL, GIMPLE_OMP_TARGET */
struct GTY((tag("GSS_OMP_PARALLEL_LAYOUT")))
  gimple_statement_omp_parallel_layout : public gimple_statement_omp
{
  /* [ WORD 1-7 ] : base class */

  /* [ WORD 8 ]
     Clauses.  */
  tree clauses;

  /* [ WORD 9 ]
     Child function holding the body of the parallel region.  */
  tree child_fn;

  /* [ WORD 10 ]
     Shared data argument.  */
  tree data_arg;
};

/* GIMPLE_OMP_PARALLEL or GIMPLE_TASK */
struct GTY((tag("GSS_OMP_PARALLEL_LAYOUT")))
  gimple_statement_omp_taskreg : public gimple_statement_omp_parallel_layout
{
    /* No extra fields; adds invariant:
         stmt->code == GIMPLE_OMP_PARALLEL
	 || stmt->code == GIMPLE_OMP_TASK.  */
};


/* GIMPLE_OMP_PARALLEL */
struct GTY((tag("GSS_OMP_PARALLEL_LAYOUT")))
  gimple_statement_omp_parallel : public gimple_statement_omp_taskreg
{
    /* No extra fields; adds invariant:
         stmt->code == GIMPLE_OMP_PARALLEL.  */
};

struct GTY((tag("GSS_OMP_PARALLEL_LAYOUT")))
  gimple_statement_omp_target : public gimple_statement_omp_parallel_layout
{
    /* No extra fields; adds invariant:
         stmt->code == GIMPLE_OMP_TARGET.  */
};

/* GIMPLE_OMP_TASK */

struct GTY((tag("GSS_OMP_TASK")))
  gimple_statement_omp_task : public gimple_statement_omp_taskreg
{
  /* [ WORD 1-10 ] : base class */

  /* [ WORD 11 ]
     Child function holding firstprivate initialization if needed.  */
  tree copy_fn;

  /* [ WORD 12-13 ]
     Size and alignment in bytes of the argument data block.  */
  tree arg_size;
  tree arg_align;
};


/* GIMPLE_OMP_SECTION */
/* Uses struct gimple_statement_omp.  */


/* GIMPLE_OMP_SECTIONS */

struct GTY((tag("GSS_OMP_SECTIONS")))
  gimple_statement_omp_sections : public gimple_statement_omp
{
  /* [ WORD 1-7 ] : base class */

  /* [ WORD 8 ]  */
  tree clauses;

  /* [ WORD 9 ]
     The control variable used for deciding which of the sections to
     execute.  */
  tree control;
};

/* GIMPLE_OMP_CONTINUE.

   Note: This does not inherit from gimple_statement_omp, because we
         do not need the body field.  */

struct GTY((tag("GSS_OMP_CONTINUE")))
  gimple_statement_omp_continue : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7 ]  */
  tree control_def;

  /* [ WORD 8 ]  */
  tree control_use;
};

/* GIMPLE_OMP_SINGLE, GIMPLE_OMP_TEAMS */

struct GTY((tag("GSS_OMP_SINGLE_LAYOUT")))
  gimple_statement_omp_single_layout : public gimple_statement_omp
{
  /* [ WORD 1-7 ] : base class */

  /* [ WORD 7 ]  */
  tree clauses;
};

struct GTY((tag("GSS_OMP_SINGLE_LAYOUT")))
  gimple_statement_omp_single : public gimple_statement_omp_single_layout
{
    /* No extra fields; adds invariant:
         stmt->code == GIMPLE_OMP_SINGLE.  */
};

struct GTY((tag("GSS_OMP_SINGLE_LAYOUT")))
  gimple_statement_omp_teams : public gimple_statement_omp_single_layout
{
    /* No extra fields; adds invariant:
         stmt->code == GIMPLE_OMP_TEAMS.  */
};


/* GIMPLE_OMP_ATOMIC_LOAD.
   Note: This is based on gimple_statement_base, not g_s_omp, because g_s_omp
   contains a sequence, which we don't need here.  */

struct GTY((tag("GSS_OMP_ATOMIC_LOAD")))
  gimple_statement_omp_atomic_load : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7-8 ]  */
  tree rhs, lhs;
};

/* GIMPLE_OMP_ATOMIC_STORE.
   See note on GIMPLE_OMP_ATOMIC_LOAD.  */

struct GTY((tag("GSS_OMP_ATOMIC_STORE_LAYOUT")))
  gimple_statement_omp_atomic_store_layout : public gimple_statement_base
{
  /* [ WORD 1-6 ] : base class */

  /* [ WORD 7 ]  */
  tree val;
};

struct GTY((tag("GSS_OMP_ATOMIC_STORE_LAYOUT")))
  gimple_statement_omp_atomic_store :
    public gimple_statement_omp_atomic_store_layout
{
    /* No extra fields; adds invariant:
         stmt->code == GIMPLE_OMP_ATOMIC_STORE.  */
};

struct GTY((tag("GSS_OMP_ATOMIC_STORE_LAYOUT")))
  gimple_statement_omp_return :
    public gimple_statement_omp_atomic_store_layout
{
    /* No extra fields; adds invariant:
         stmt->code == GIMPLE_OMP_RETURN.  */
};

/* GIMPLE_TRANSACTION.  */

/* Bits to be stored in the GIMPLE_TRANSACTION subcode.  */

/* The __transaction_atomic was declared [[outer]] or it is
   __transaction_relaxed.  */
#define GTMA_IS_OUTER			(1u << 0)
#define GTMA_IS_RELAXED			(1u << 1)
#define GTMA_DECLARATION_MASK		(GTMA_IS_OUTER | GTMA_IS_RELAXED)

/* The transaction is seen to not have an abort.  */
#define GTMA_HAVE_ABORT			(1u << 2)
/* The transaction is seen to have loads or stores.  */
#define GTMA_HAVE_LOAD			(1u << 3)
#define GTMA_HAVE_STORE			(1u << 4)
/* The transaction MAY enter serial irrevocable mode in its dynamic scope.  */
#define GTMA_MAY_ENTER_IRREVOCABLE	(1u << 5)
/* The transaction WILL enter serial irrevocable mode.
   An irrevocable block post-dominates the entire transaction, such
   that all invocations of the transaction will go serial-irrevocable.
   In such case, we don't bother instrumenting the transaction, and
   tell the runtime that it should begin the transaction in
   serial-irrevocable mode.  */
#define GTMA_DOES_GO_IRREVOCABLE	(1u << 6)
/* The transaction contains no instrumentation code whatsover, most
   likely because it is guaranteed to go irrevocable upon entry.  */
#define GTMA_HAS_NO_INSTRUMENTATION	(1u << 7)

struct GTY((tag("GSS_TRANSACTION")))
  gimple_statement_transaction : public gimple_statement_with_memory_ops_base
{
  /* [ WORD 1-9 ] : base class */

  /* [ WORD 10 ] */
  gimple_seq body;

  /* [ WORD 11 ] */
  tree label;
};

#define DEFGSSTRUCT(SYM, STRUCT, HAS_TREE_OP)	SYM,
enum gimple_statement_structure_enum {
#include "gsstruct.def"
    LAST_GSS_ENUM
};
#undef DEFGSSTRUCT

template <>
template <>
inline bool
is_a_helper <gimple_statement_asm *>::test (gimple gs)
{
  return gs->code == GIMPLE_ASM;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_bind *>::test (gimple gs)
{
  return gs->code == GIMPLE_BIND;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_call *>::test (gimple gs)
{
  return gs->code == GIMPLE_CALL;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_catch *>::test (gimple gs)
{
  return gs->code == GIMPLE_CATCH;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_resx *>::test (gimple gs)
{
  return gs->code == GIMPLE_RESX;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_eh_dispatch *>::test (gimple gs)
{
  return gs->code == GIMPLE_EH_DISPATCH;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_eh_else *>::test (gimple gs)
{
  return gs->code == GIMPLE_EH_ELSE;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_eh_filter *>::test (gimple gs)
{
  return gs->code == GIMPLE_EH_FILTER;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_eh_mnt *>::test (gimple gs)
{
  return gs->code == GIMPLE_EH_MUST_NOT_THROW;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_atomic_load *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_ATOMIC_LOAD;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_atomic_store *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_ATOMIC_STORE;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_return *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_RETURN;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_continue *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_CONTINUE;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_critical *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_CRITICAL;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_for *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_FOR;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_taskreg *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_PARALLEL || gs->code == GIMPLE_OMP_TASK;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_parallel *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_PARALLEL;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_target *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_TARGET;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_sections *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_SECTIONS;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_single *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_SINGLE;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_teams *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_TEAMS;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_omp_task *>::test (gimple gs)
{
  return gs->code == GIMPLE_OMP_TASK;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_phi *>::test (gimple gs)
{
  return gs->code == GIMPLE_PHI;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_transaction *>::test (gimple gs)
{
  return gs->code == GIMPLE_TRANSACTION;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_try *>::test (gimple gs)
{
  return gs->code == GIMPLE_TRY;
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_wce *>::test (gimple gs)
{
  return gs->code == GIMPLE_WITH_CLEANUP_EXPR;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_asm *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_ASM;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_bind *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_BIND;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_call *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_CALL;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_catch *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_CATCH;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_resx *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_RESX;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_eh_dispatch *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_EH_DISPATCH;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_eh_filter *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_EH_FILTER;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_atomic_load *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_ATOMIC_LOAD;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_atomic_store *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_ATOMIC_STORE;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_return *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_RETURN;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_continue *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_CONTINUE;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_critical *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_CRITICAL;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_for *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_FOR;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_taskreg *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_PARALLEL || gs->code == GIMPLE_OMP_TASK;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_parallel *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_PARALLEL;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_target *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_TARGET;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_sections *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_SECTIONS;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_single *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_SINGLE;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_teams *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_TEAMS;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_omp_task *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_OMP_TASK;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_phi *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_PHI;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_transaction *>::test (const_gimple gs)
{
  return gs->code == GIMPLE_TRANSACTION;
}

/* Offset in bytes to the location of the operand vector.
   Zero if there is no operand vector for this tuple structure.  */
extern size_t const gimple_ops_offset_[];

/* Map GIMPLE codes to GSS codes.  */
extern enum gimple_statement_structure_enum const gss_for_code_[];

/* This variable holds the currently expanded gimple statement for purposes
   of comminucating the profile info to the builtin expanders.  */
extern gimple currently_expanding_gimple_stmt;

#define gimple_alloc(c, n) gimple_alloc_stat (c, n MEM_STAT_INFO)
gimple gimple_alloc_stat (enum gimple_code, unsigned MEM_STAT_DECL);
gimple gimple_build_return (tree);
void gimple_call_reset_alias_info (gimple);
gimple gimple_build_call_vec (tree, vec<tree> );
gimple gimple_build_call (tree, unsigned, ...);
gimple gimple_build_call_valist (tree, unsigned, va_list);
gimple gimple_build_call_internal (enum internal_fn, unsigned, ...);
gimple gimple_build_call_internal_vec (enum internal_fn, vec<tree> );
gimple gimple_build_call_from_tree (tree);
gimple gimple_build_assign_stat (tree, tree MEM_STAT_DECL);
#define gimple_build_assign(l,r) gimple_build_assign_stat (l, r MEM_STAT_INFO)
gimple gimple_build_assign_with_ops (enum tree_code, tree,
				     tree, tree, tree CXX_MEM_STAT_INFO);
gimple gimple_build_assign_with_ops (enum tree_code, tree,
				     tree, tree CXX_MEM_STAT_INFO);
gimple gimple_build_cond (enum tree_code, tree, tree, tree, tree);
gimple gimple_build_cond_from_tree (tree, tree, tree);
void gimple_cond_set_condition_from_tree (gimple, tree);
gimple gimple_build_label (tree label);
gimple gimple_build_goto (tree dest);
gimple gimple_build_nop (void);
gimple gimple_build_bind (tree, gimple_seq, tree);
gimple gimple_build_asm_vec (const char *, vec<tree, va_gc> *,
			     vec<tree, va_gc> *, vec<tree, va_gc> *,
			     vec<tree, va_gc> *);
gimple gimple_build_catch (tree, gimple_seq);
gimple gimple_build_eh_filter (tree, gimple_seq);
gimple gimple_build_eh_must_not_throw (tree);
gimple gimple_build_eh_else (gimple_seq, gimple_seq);
gimple_statement_try *gimple_build_try (gimple_seq, gimple_seq,
					enum gimple_try_flags);
gimple gimple_build_wce (gimple_seq);
gimple gimple_build_resx (int);
gimple gimple_build_switch_nlabels (unsigned, tree, tree);
gimple gimple_build_switch (tree, tree, vec<tree> );
gimple gimple_build_eh_dispatch (int);
gimple gimple_build_debug_bind_stat (tree, tree, gimple MEM_STAT_DECL);
#define gimple_build_debug_bind(var,val,stmt)			\
  gimple_build_debug_bind_stat ((var), (val), (stmt) MEM_STAT_INFO)
gimple gimple_build_debug_source_bind_stat (tree, tree, gimple MEM_STAT_DECL);
#define gimple_build_debug_source_bind(var,val,stmt)			\
  gimple_build_debug_source_bind_stat ((var), (val), (stmt) MEM_STAT_INFO)
gimple gimple_build_omp_critical (gimple_seq, tree);
gimple gimple_build_omp_for (gimple_seq, int, tree, size_t, gimple_seq);
gimple gimple_build_omp_parallel (gimple_seq, tree, tree, tree);
gimple gimple_build_omp_task (gimple_seq, tree, tree, tree, tree, tree, tree);
gimple gimple_build_omp_section (gimple_seq);
gimple gimple_build_omp_master (gimple_seq);
gimple gimple_build_omp_taskgroup (gimple_seq);
gimple gimple_build_omp_continue (tree, tree);
gimple gimple_build_omp_ordered (gimple_seq);
gimple gimple_build_omp_return (bool);
gimple gimple_build_omp_sections (gimple_seq, tree);
gimple gimple_build_omp_sections_switch (void);
gimple gimple_build_omp_single (gimple_seq, tree);
gimple gimple_build_omp_target (gimple_seq, int, tree);
gimple gimple_build_omp_teams (gimple_seq, tree);
gimple gimple_build_omp_atomic_load (tree, tree);
gimple gimple_build_omp_atomic_store (tree);
gimple gimple_build_transaction (gimple_seq, tree);
gimple gimple_build_predict (enum br_predictor, enum prediction);
extern void gimple_seq_add_stmt (gimple_seq *, gimple);
extern void gimple_seq_add_stmt_without_update (gimple_seq *, gimple);
void gimple_seq_add_seq (gimple_seq *, gimple_seq);
extern void annotate_all_with_location_after (gimple_seq, gimple_stmt_iterator,
					      location_t);
extern void annotate_all_with_location (gimple_seq, location_t);
bool empty_body_p (gimple_seq);
gimple_seq gimple_seq_copy (gimple_seq);
bool gimple_call_same_target_p (const_gimple, const_gimple);
int gimple_call_flags (const_gimple);
int gimple_call_arg_flags (const_gimple, unsigned);
int gimple_call_return_flags (const_gimple);
bool gimple_assign_copy_p (gimple);
bool gimple_assign_ssa_name_copy_p (gimple);
bool gimple_assign_unary_nop_p (gimple);
void gimple_set_bb (gimple, basic_block);
void gimple_assign_set_rhs_from_tree (gimple_stmt_iterator *, tree);
void gimple_assign_set_rhs_with_ops_1 (gimple_stmt_iterator *, enum tree_code,
				       tree, tree, tree);
tree gimple_get_lhs (const_gimple);
void gimple_set_lhs (gimple, tree);
gimple gimple_copy (gimple);
bool gimple_has_side_effects (const_gimple);
bool gimple_could_trap_p_1 (gimple, bool, bool);
bool gimple_could_trap_p (gimple);
bool gimple_assign_rhs_could_trap_p (gimple);
extern void dump_gimple_statistics (void);
unsigned get_gimple_rhs_num_ops (enum tree_code);
extern tree canonicalize_cond_expr_cond (tree);
gimple gimple_call_copy_skip_args (gimple, bitmap);
extern bool gimple_compare_field_offset (tree, tree);
extern tree gimple_unsigned_type (tree);
extern tree gimple_signed_type (tree);
extern alias_set_type gimple_get_alias_set (tree);
extern bool gimple_ior_addresses_taken (bitmap, gimple);
extern bool gimple_builtin_call_types_compatible_p (const_gimple, tree);
extern bool gimple_call_builtin_p (const_gimple);
extern bool gimple_call_builtin_p (const_gimple, enum built_in_class);
extern bool gimple_call_builtin_p (const_gimple, enum built_in_function);
extern bool gimple_asm_clobbers_memory_p (const_gimple);
extern void dump_decl_set (FILE *, bitmap);
extern bool nonfreeing_call_p (gimple);
extern bool infer_nonnull_range (gimple, tree, bool, bool);
extern void sort_case_labels (vec<tree> );
extern void preprocess_case_label_vec_for_gimple (vec<tree> , tree, tree *);
extern void gimple_seq_set_location (gimple_seq , location_t);

/* Formal (expression) temporary table handling: multiple occurrences of
   the same scalar expression are evaluated into the same temporary.  */

typedef struct gimple_temp_hash_elt
{
  tree val;   /* Key */
  tree temp;  /* Value */
} elt_t;

/* Get the number of the next statement uid to be allocated.  */
static inline unsigned int
gimple_stmt_max_uid (struct function *fn)
{
  return fn->last_stmt_uid;
}

/* Set the number of the next statement uid to be allocated.  */
static inline void
set_gimple_stmt_max_uid (struct function *fn, unsigned int maxid)
{
  fn->last_stmt_uid = maxid;
}

/* Set the number of the next statement uid to be allocated.  */
static inline unsigned int
inc_gimple_stmt_max_uid (struct function *fn)
{
  return fn->last_stmt_uid++;
}

/* Return the first node in GIMPLE sequence S.  */

static inline gimple_seq_node
gimple_seq_first (gimple_seq s)
{
  return s;
}


/* Return the first statement in GIMPLE sequence S.  */

static inline gimple
gimple_seq_first_stmt (gimple_seq s)
{
  gimple_seq_node n = gimple_seq_first (s);
  return n;
}


/* Return the last node in GIMPLE sequence S.  */

static inline gimple_seq_node
gimple_seq_last (gimple_seq s)
{
  return s ? s->prev : NULL;
}


/* Return the last statement in GIMPLE sequence S.  */

static inline gimple
gimple_seq_last_stmt (gimple_seq s)
{
  gimple_seq_node n = gimple_seq_last (s);
  return n;
}


/* Set the last node in GIMPLE sequence *PS to LAST.  */

static inline void
gimple_seq_set_last (gimple_seq *ps, gimple_seq_node last)
{
  (*ps)->prev = last;
}


/* Set the first node in GIMPLE sequence *PS to FIRST.  */

static inline void
gimple_seq_set_first (gimple_seq *ps, gimple_seq_node first)
{
  *ps = first;
}


/* Return true if GIMPLE sequence S is empty.  */

static inline bool
gimple_seq_empty_p (gimple_seq s)
{
  return s == NULL;
}

/* Allocate a new sequence and initialize its first element with STMT.  */

static inline gimple_seq
gimple_seq_alloc_with_stmt (gimple stmt)
{
  gimple_seq seq = NULL;
  gimple_seq_add_stmt (&seq, stmt);
  return seq;
}


/* Returns the sequence of statements in BB.  */

static inline gimple_seq
bb_seq (const_basic_block bb)
{
  return (!(bb->flags & BB_RTL)) ? bb->il.gimple.seq : NULL;
}

static inline gimple_seq *
bb_seq_addr (basic_block bb)
{
  return (!(bb->flags & BB_RTL)) ? &bb->il.gimple.seq : NULL;
}

/* Sets the sequence of statements in BB to SEQ.  */

static inline void
set_bb_seq (basic_block bb, gimple_seq seq)
{
  gcc_checking_assert (!(bb->flags & BB_RTL));
  bb->il.gimple.seq = seq;
}


/* Return the code for GIMPLE statement G.  */

static inline enum gimple_code
gimple_code (const_gimple g)
{
  return g->code;
}


/* Return the GSS code used by a GIMPLE code.  */

static inline enum gimple_statement_structure_enum
gss_for_code (enum gimple_code code)
{
  gcc_gimple_checking_assert ((unsigned int)code < LAST_AND_UNUSED_GIMPLE_CODE);
  return gss_for_code_[code];
}


/* Return which GSS code is used by GS.  */

static inline enum gimple_statement_structure_enum
gimple_statement_structure (gimple gs)
{
  return gss_for_code (gimple_code (gs));
}


/* Return true if statement G has sub-statements.  This is only true for
   High GIMPLE statements.  */

static inline bool
gimple_has_substatements (gimple g)
{
  switch (gimple_code (g))
    {
    case GIMPLE_BIND:
    case GIMPLE_CATCH:
    case GIMPLE_EH_FILTER:
    case GIMPLE_EH_ELSE:
    case GIMPLE_TRY:
    case GIMPLE_OMP_FOR:
    case GIMPLE_OMP_MASTER:
    case GIMPLE_OMP_TASKGROUP:
    case GIMPLE_OMP_ORDERED:
    case GIMPLE_OMP_SECTION:
    case GIMPLE_OMP_PARALLEL:
    case GIMPLE_OMP_TASK:
    case GIMPLE_OMP_SECTIONS:
    case GIMPLE_OMP_SINGLE:
    case GIMPLE_OMP_TARGET:
    case GIMPLE_OMP_TEAMS:
    case GIMPLE_OMP_CRITICAL:
    case GIMPLE_WITH_CLEANUP_EXPR:
    case GIMPLE_TRANSACTION:
      return true;

    default:
      return false;
    }
}


/* Return the basic block holding statement G.  */

static inline basic_block
gimple_bb (const_gimple g)
{
  return g->bb;
}


/* Return the lexical scope block holding statement G.  */

static inline tree
gimple_block (const_gimple g)
{
  return LOCATION_BLOCK (g->location);
}


/* Set BLOCK to be the lexical scope block holding statement G.  */

static inline void
gimple_set_block (gimple g, tree block)
{
  if (block)
    g->location =
	COMBINE_LOCATION_DATA (line_table, g->location, block);
  else
    g->location = LOCATION_LOCUS (g->location);
}


/* Return location information for statement G.  */

static inline location_t
gimple_location (const_gimple g)
{
  return g->location;
}

/* Return pointer to location information for statement G.  */

static inline const location_t *
gimple_location_ptr (const_gimple g)
{
  return &g->location;
}


/* Set location information for statement G.  */

static inline void
gimple_set_location (gimple g, location_t location)
{
  g->location = location;
}


/* Return true if G contains location information.  */

static inline bool
gimple_has_location (const_gimple g)
{
  return LOCATION_LOCUS (gimple_location (g)) != UNKNOWN_LOCATION;
}


/* Return the file name of the location of STMT.  */

static inline const char *
gimple_filename (const_gimple stmt)
{
  return LOCATION_FILE (gimple_location (stmt));
}


/* Return the line number of the location of STMT.  */

static inline int
gimple_lineno (const_gimple stmt)
{
  return LOCATION_LINE (gimple_location (stmt));
}


/* Determine whether SEQ is a singleton. */

static inline bool
gimple_seq_singleton_p (gimple_seq seq)
{
  return ((gimple_seq_first (seq) != NULL)
	  && (gimple_seq_first (seq) == gimple_seq_last (seq)));
}

/* Return true if no warnings should be emitted for statement STMT.  */

static inline bool
gimple_no_warning_p (const_gimple stmt)
{
  return stmt->no_warning;
}

/* Set the no_warning flag of STMT to NO_WARNING.  */

static inline void
gimple_set_no_warning (gimple stmt, bool no_warning)
{
  stmt->no_warning = (unsigned) no_warning;
}

/* Set the visited status on statement STMT to VISITED_P.  */

static inline void
gimple_set_visited (gimple stmt, bool visited_p)
{
  stmt->visited = (unsigned) visited_p;
}


/* Return the visited status for statement STMT.  */

static inline bool
gimple_visited_p (gimple stmt)
{
  return stmt->visited;
}


/* Set pass local flag PLF on statement STMT to VAL_P.  */

static inline void
gimple_set_plf (gimple stmt, enum plf_mask plf, bool val_p)
{
  if (val_p)
    stmt->plf |= (unsigned int) plf;
  else
    stmt->plf &= ~((unsigned int) plf);
}


/* Return the value of pass local flag PLF on statement STMT.  */

static inline unsigned int
gimple_plf (gimple stmt, enum plf_mask plf)
{
  return stmt->plf & ((unsigned int) plf);
}


/* Set the UID of statement.  */

static inline void
gimple_set_uid (gimple g, unsigned uid)
{
  g->uid = uid;
}


/* Return the UID of statement.  */

static inline unsigned
gimple_uid (const_gimple g)
{
  return g->uid;
}


/* Make statement G a singleton sequence.  */

static inline void
gimple_init_singleton (gimple g)
{
  g->next = NULL;
  g->prev = g;
}


/* Return true if GIMPLE statement G has register or memory operands.  */

static inline bool
gimple_has_ops (const_gimple g)
{
  return gimple_code (g) >= GIMPLE_COND && gimple_code (g) <= GIMPLE_RETURN;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_with_ops *>::test (const_gimple gs)
{
  return gimple_has_ops (gs);
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_with_ops *>::test (gimple gs)
{
  return gimple_has_ops (gs);
}

/* Return true if GIMPLE statement G has memory operands.  */

static inline bool
gimple_has_mem_ops (const_gimple g)
{
  return gimple_code (g) >= GIMPLE_ASSIGN && gimple_code (g) <= GIMPLE_RETURN;
}

template <>
template <>
inline bool
is_a_helper <const gimple_statement_with_memory_ops *>::test (const_gimple gs)
{
  return gimple_has_mem_ops (gs);
}

template <>
template <>
inline bool
is_a_helper <gimple_statement_with_memory_ops *>::test (gimple gs)
{
  return gimple_has_mem_ops (gs);
}

/* Return the set of USE operands for statement G.  */

static inline struct use_optype_d *
gimple_use_ops (const_gimple g)
{
  const gimple_statement_with_ops *ops_stmt =
    dyn_cast <const gimple_statement_with_ops *> (g);
  if (!ops_stmt)
    return NULL;
  return ops_stmt->use_ops;
}


/* Set USE to be the set of USE operands for statement G.  */

static inline void
gimple_set_use_ops (gimple g, struct use_optype_d *use)
{
  gimple_statement_with_ops *ops_stmt =
    as_a <gimple_statement_with_ops *> (g);
  ops_stmt->use_ops = use;
}


/* Return the single VUSE operand of the statement G.  */

static inline tree
gimple_vuse (const_gimple g)
{
  const gimple_statement_with_memory_ops *mem_ops_stmt =
     dyn_cast <const gimple_statement_with_memory_ops *> (g);
  if (!mem_ops_stmt)
    return NULL_TREE;
  return mem_ops_stmt->vuse;
}

/* Return the single VDEF operand of the statement G.  */

static inline tree
gimple_vdef (const_gimple g)
{
  const gimple_statement_with_memory_ops *mem_ops_stmt =
     dyn_cast <const gimple_statement_with_memory_ops *> (g);
  if (!mem_ops_stmt)
    return NULL_TREE;
  return mem_ops_stmt->vdef;
}

/* Return the single VUSE operand of the statement G.  */

static inline tree *
gimple_vuse_ptr (gimple g)
{
  gimple_statement_with_memory_ops *mem_ops_stmt =
     dyn_cast <gimple_statement_with_memory_ops *> (g);
  if (!mem_ops_stmt)
    return NULL;
  return &mem_ops_stmt->vuse;
}

/* Return the single VDEF operand of the statement G.  */

static inline tree *
gimple_vdef_ptr (gimple g)
{
  gimple_statement_with_memory_ops *mem_ops_stmt =
     dyn_cast <gimple_statement_with_memory_ops *> (g);
  if (!mem_ops_stmt)
    return NULL;
  return &mem_ops_stmt->vdef;
}

/* Set the single VUSE operand of the statement G.  */

static inline void
gimple_set_vuse (gimple g, tree vuse)
{
  gimple_statement_with_memory_ops *mem_ops_stmt =
    as_a <gimple_statement_with_memory_ops *> (g);
  mem_ops_stmt->vuse = vuse;
}

/* Set the single VDEF operand of the statement G.  */

static inline void
gimple_set_vdef (gimple g, tree vdef)
{
  gimple_statement_with_memory_ops *mem_ops_stmt =
    as_a <gimple_statement_with_memory_ops *> (g);
  mem_ops_stmt->vdef = vdef;
}


/* Return true if statement G has operands and the modified field has
   been set.  */

static inline bool
gimple_modified_p (const_gimple g)
{
  return (gimple_has_ops (g)) ? (bool) g->modified : false;
}


/* Set the MODIFIED flag to MODIFIEDP, iff the gimple statement G has
   a MODIFIED field.  */

static inline void
gimple_set_modified (gimple s, bool modifiedp)
{
  if (gimple_has_ops (s))
    s->modified = (unsigned) modifiedp;
}


/* Return the tree code for the expression computed by STMT.  This is
   only valid for GIMPLE_COND, GIMPLE_CALL and GIMPLE_ASSIGN.  For
   GIMPLE_CALL, return CALL_EXPR as the expression code for
   consistency.  This is useful when the caller needs to deal with the
   three kinds of computation that GIMPLE supports.  */

static inline enum tree_code
gimple_expr_code (const_gimple stmt)
{
  enum gimple_code code = gimple_code (stmt);
  if (code == GIMPLE_ASSIGN || code == GIMPLE_COND)
    return (enum tree_code) stmt->subcode;
  else
    {
      gcc_gimple_checking_assert (code == GIMPLE_CALL);
      return CALL_EXPR;
    }
}


/* Return true if statement STMT contains volatile operands.  */

static inline bool
gimple_has_volatile_ops (const_gimple stmt)
{
  if (gimple_has_mem_ops (stmt))
    return stmt->has_volatile_ops;
  else
    return false;
}


/* Set the HAS_VOLATILE_OPS flag to VOLATILEP.  */

static inline void
gimple_set_has_volatile_ops (gimple stmt, bool volatilep)
{
  if (gimple_has_mem_ops (stmt))
    stmt->has_volatile_ops = (unsigned) volatilep;
}

/* Return true if STMT is in a transaction.  */

static inline bool
gimple_in_transaction (gimple stmt)
{
  return bb_in_transaction (gimple_bb (stmt));
}

/* Return true if statement STMT may access memory.  */

static inline bool
gimple_references_memory_p (gimple stmt)
{
  return gimple_has_mem_ops (stmt) && gimple_vuse (stmt);
}


/* Return the subcode for OMP statement S.  */

static inline unsigned
gimple_omp_subcode (const_gimple s)
{
  gcc_gimple_checking_assert (gimple_code (s) >= GIMPLE_OMP_ATOMIC_LOAD
	      && gimple_code (s) <= GIMPLE_OMP_TEAMS);
  return s->subcode;
}

/* Set the subcode for OMP statement S to SUBCODE.  */

static inline void
gimple_omp_set_subcode (gimple s, unsigned int subcode)
{
  /* We only have 16 bits for the subcode.  Assert that we are not
     overflowing it.  */
  gcc_gimple_checking_assert (subcode < (1 << 16));
  s->subcode = subcode;
}

/* Set the nowait flag on OMP_RETURN statement S.  */

static inline void
gimple_omp_return_set_nowait (gimple s)
{
  GIMPLE_CHECK (s, GIMPLE_OMP_RETURN);
  s->subcode |= GF_OMP_RETURN_NOWAIT;
}


/* Return true if OMP return statement G has the GF_OMP_RETURN_NOWAIT
   flag set.  */

static inline bool
gimple_omp_return_nowait_p (const_gimple g)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_RETURN);
  return (gimple_omp_subcode (g) & GF_OMP_RETURN_NOWAIT) != 0;
}


/* Set the LHS of OMP return.  */

static inline void
gimple_omp_return_set_lhs (gimple g, tree lhs)
{
  gimple_statement_omp_return *omp_return_stmt =
    as_a <gimple_statement_omp_return *> (g);
  omp_return_stmt->val = lhs;
}


/* Get the LHS of OMP return.  */

static inline tree
gimple_omp_return_lhs (const_gimple g)
{
  const gimple_statement_omp_return *omp_return_stmt =
    as_a <const gimple_statement_omp_return *> (g);
  return omp_return_stmt->val;
}


/* Return a pointer to the LHS of OMP return.  */

static inline tree *
gimple_omp_return_lhs_ptr (gimple g)
{
  gimple_statement_omp_return *omp_return_stmt =
    as_a <gimple_statement_omp_return *> (g);
  return &omp_return_stmt->val;
}


/* Return true if OMP section statement G has the GF_OMP_SECTION_LAST
   flag set.  */

static inline bool
gimple_omp_section_last_p (const_gimple g)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_SECTION);
  return (gimple_omp_subcode (g) & GF_OMP_SECTION_LAST) != 0;
}


/* Set the GF_OMP_SECTION_LAST flag on G.  */

static inline void
gimple_omp_section_set_last (gimple g)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_SECTION);
  g->subcode |= GF_OMP_SECTION_LAST;
}


/* Return true if OMP parallel statement G has the
   GF_OMP_PARALLEL_COMBINED flag set.  */

static inline bool
gimple_omp_parallel_combined_p (const_gimple g)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_PARALLEL);
  return (gimple_omp_subcode (g) & GF_OMP_PARALLEL_COMBINED) != 0;
}


/* Set the GF_OMP_PARALLEL_COMBINED field in G depending on the boolean
   value of COMBINED_P.  */

static inline void
gimple_omp_parallel_set_combined_p (gimple g, bool combined_p)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_PARALLEL);
  if (combined_p)
    g->subcode |= GF_OMP_PARALLEL_COMBINED;
  else
    g->subcode &= ~GF_OMP_PARALLEL_COMBINED;
}


/* Return true if OMP atomic load/store statement G has the
   GF_OMP_ATOMIC_NEED_VALUE flag set.  */

static inline bool
gimple_omp_atomic_need_value_p (const_gimple g)
{
  if (gimple_code (g) != GIMPLE_OMP_ATOMIC_LOAD)
    GIMPLE_CHECK (g, GIMPLE_OMP_ATOMIC_STORE);
  return (gimple_omp_subcode (g) & GF_OMP_ATOMIC_NEED_VALUE) != 0;
}


/* Set the GF_OMP_ATOMIC_NEED_VALUE flag on G.  */

static inline void
gimple_omp_atomic_set_need_value (gimple g)
{
  if (gimple_code (g) != GIMPLE_OMP_ATOMIC_LOAD)
    GIMPLE_CHECK (g, GIMPLE_OMP_ATOMIC_STORE);
  g->subcode |= GF_OMP_ATOMIC_NEED_VALUE;
}


/* Return true if OMP atomic load/store statement G has the
   GF_OMP_ATOMIC_SEQ_CST flag set.  */

static inline bool
gimple_omp_atomic_seq_cst_p (const_gimple g)
{
  if (gimple_code (g) != GIMPLE_OMP_ATOMIC_LOAD)
    GIMPLE_CHECK (g, GIMPLE_OMP_ATOMIC_STORE);
  return (gimple_omp_subcode (g) & GF_OMP_ATOMIC_SEQ_CST) != 0;
}


/* Set the GF_OMP_ATOMIC_SEQ_CST flag on G.  */

static inline void
gimple_omp_atomic_set_seq_cst (gimple g)
{
  if (gimple_code (g) != GIMPLE_OMP_ATOMIC_LOAD)
    GIMPLE_CHECK (g, GIMPLE_OMP_ATOMIC_STORE);
  g->subcode |= GF_OMP_ATOMIC_SEQ_CST;
}


/* Return the number of operands for statement GS.  */

static inline unsigned
gimple_num_ops (const_gimple gs)
{
  return gs->num_ops;
}


/* Set the number of operands for statement GS.  */

static inline void
gimple_set_num_ops (gimple gs, unsigned num_ops)
{
  gs->num_ops = num_ops;
}


/* Return the array of operands for statement GS.  */

static inline tree *
gimple_ops (gimple gs)
{
  size_t off;

  /* All the tuples have their operand vector at the very bottom
     of the structure.  Note that those structures that do not
     have an operand vector have a zero offset.  */
  off = gimple_ops_offset_[gimple_statement_structure (gs)];
  gcc_gimple_checking_assert (off != 0);

  return (tree *) ((char *) gs + off);
}


/* Return operand I for statement GS.  */

static inline tree
gimple_op (const_gimple gs, unsigned i)
{
  if (gimple_has_ops (gs))
    {
      gcc_gimple_checking_assert (i < gimple_num_ops (gs));
      return gimple_ops (CONST_CAST_GIMPLE (gs))[i];
    }
  else
    return NULL_TREE;
}

/* Return a pointer to operand I for statement GS.  */

static inline tree *
gimple_op_ptr (const_gimple gs, unsigned i)
{
  if (gimple_has_ops (gs))
    {
      gcc_gimple_checking_assert (i < gimple_num_ops (gs));
      return gimple_ops (CONST_CAST_GIMPLE (gs)) + i;
    }
  else
    return NULL;
}

/* Set operand I of statement GS to OP.  */

static inline void
gimple_set_op (gimple gs, unsigned i, tree op)
{
  gcc_gimple_checking_assert (gimple_has_ops (gs) && i < gimple_num_ops (gs));

  /* Note.  It may be tempting to assert that OP matches
     is_gimple_operand, but that would be wrong.  Different tuples
     accept slightly different sets of tree operands.  Each caller
     should perform its own validation.  */
  gimple_ops (gs)[i] = op;
}

/* Return true if GS is a GIMPLE_ASSIGN.  */

static inline bool
is_gimple_assign (const_gimple gs)
{
  return gimple_code (gs) == GIMPLE_ASSIGN;
}

/* Determine if expression CODE is one of the valid expressions that can
   be used on the RHS of GIMPLE assignments.  */

static inline enum gimple_rhs_class
get_gimple_rhs_class (enum tree_code code)
{
  return (enum gimple_rhs_class) gimple_rhs_class_table[(int) code];
}

/* Return the LHS of assignment statement GS.  */

static inline tree
gimple_assign_lhs (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);
  return gimple_op (gs, 0);
}


/* Return a pointer to the LHS of assignment statement GS.  */

static inline tree *
gimple_assign_lhs_ptr (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);
  return gimple_op_ptr (gs, 0);
}


/* Set LHS to be the LHS operand of assignment statement GS.  */

static inline void
gimple_assign_set_lhs (gimple gs, tree lhs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);
  gimple_set_op (gs, 0, lhs);

  if (lhs && TREE_CODE (lhs) == SSA_NAME)
    SSA_NAME_DEF_STMT (lhs) = gs;
}


/* Return the first operand on the RHS of assignment statement GS.  */

static inline tree
gimple_assign_rhs1 (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);
  return gimple_op (gs, 1);
}


/* Return a pointer to the first operand on the RHS of assignment
   statement GS.  */

static inline tree *
gimple_assign_rhs1_ptr (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);
  return gimple_op_ptr (gs, 1);
}

/* Set RHS to be the first operand on the RHS of assignment statement GS.  */

static inline void
gimple_assign_set_rhs1 (gimple gs, tree rhs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);

  gimple_set_op (gs, 1, rhs);
}


/* Return the second operand on the RHS of assignment statement GS.
   If GS does not have two operands, NULL is returned instead.  */

static inline tree
gimple_assign_rhs2 (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);

  if (gimple_num_ops (gs) >= 3)
    return gimple_op (gs, 2);
  else
    return NULL_TREE;
}


/* Return a pointer to the second operand on the RHS of assignment
   statement GS.  */

static inline tree *
gimple_assign_rhs2_ptr (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);
  return gimple_op_ptr (gs, 2);
}


/* Set RHS to be the second operand on the RHS of assignment statement GS.  */

static inline void
gimple_assign_set_rhs2 (gimple gs, tree rhs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);

  gimple_set_op (gs, 2, rhs);
}

/* Return the third operand on the RHS of assignment statement GS.
   If GS does not have two operands, NULL is returned instead.  */

static inline tree
gimple_assign_rhs3 (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);

  if (gimple_num_ops (gs) >= 4)
    return gimple_op (gs, 3);
  else
    return NULL_TREE;
}

/* Return a pointer to the third operand on the RHS of assignment
   statement GS.  */

static inline tree *
gimple_assign_rhs3_ptr (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);
  return gimple_op_ptr (gs, 3);
}


/* Set RHS to be the third operand on the RHS of assignment statement GS.  */

static inline void
gimple_assign_set_rhs3 (gimple gs, tree rhs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);

  gimple_set_op (gs, 3, rhs);
}

/* A wrapper around gimple_assign_set_rhs_with_ops_1, for callers which expect
   to see only a maximum of two operands.  */

static inline void
gimple_assign_set_rhs_with_ops (gimple_stmt_iterator *gsi, enum tree_code code,
				tree op1, tree op2)
{
  gimple_assign_set_rhs_with_ops_1 (gsi, code, op1, op2, NULL);
}

/* Returns true if GS is a nontemporal move.  */

static inline bool
gimple_assign_nontemporal_move_p (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);
  return gs->nontemporal_move;
}

/* Sets nontemporal move flag of GS to NONTEMPORAL.  */

static inline void
gimple_assign_set_nontemporal_move (gimple gs, bool nontemporal)
{
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);
  gs->nontemporal_move = nontemporal;
}


/* Return the code of the expression computed on the rhs of assignment
   statement GS.  In case that the RHS is a single object, returns the
   tree code of the object.  */

static inline enum tree_code
gimple_assign_rhs_code (const_gimple gs)
{
  enum tree_code code;
  GIMPLE_CHECK (gs, GIMPLE_ASSIGN);

  code = (enum tree_code) gs->subcode;
  /* While we initially set subcode to the TREE_CODE of the rhs for
     GIMPLE_SINGLE_RHS assigns we do not update that subcode to stay
     in sync when we rewrite stmts into SSA form or do SSA propagations.  */
  if (get_gimple_rhs_class (code) == GIMPLE_SINGLE_RHS)
    code = TREE_CODE (gimple_assign_rhs1 (gs));

  return code;
}


/* Set CODE to be the code for the expression computed on the RHS of
   assignment S.  */

static inline void
gimple_assign_set_rhs_code (gimple s, enum tree_code code)
{
  GIMPLE_CHECK (s, GIMPLE_ASSIGN);
  s->subcode = code;
}


/* Return the gimple rhs class of the code of the expression computed on
   the rhs of assignment statement GS.
   This will never return GIMPLE_INVALID_RHS.  */

static inline enum gimple_rhs_class
gimple_assign_rhs_class (const_gimple gs)
{
  return get_gimple_rhs_class (gimple_assign_rhs_code (gs));
}

/* Return true if GS is an assignment with a singleton RHS, i.e.,
   there is no operator associated with the assignment itself.
   Unlike gimple_assign_copy_p, this predicate returns true for
   any RHS operand, including those that perform an operation
   and do not have the semantics of a copy, such as COND_EXPR.  */

static inline bool
gimple_assign_single_p (const_gimple gs)
{
  return (is_gimple_assign (gs)
          && gimple_assign_rhs_class (gs) == GIMPLE_SINGLE_RHS);
}

/* Return true if GS performs a store to its lhs.  */

static inline bool
gimple_store_p (const_gimple gs)
{
  tree lhs = gimple_get_lhs (gs);
  return lhs && !is_gimple_reg (lhs);
}

/* Return true if GS is an assignment that loads from its rhs1.  */

static inline bool
gimple_assign_load_p (const_gimple gs)
{
  tree rhs;
  if (!gimple_assign_single_p (gs))
    return false;
  rhs = gimple_assign_rhs1 (gs);
  if (TREE_CODE (rhs) == WITH_SIZE_EXPR)
    return true;
  rhs = get_base_address (rhs);
  return (DECL_P (rhs)
	  || TREE_CODE (rhs) == MEM_REF || TREE_CODE (rhs) == TARGET_MEM_REF);
}


/* Return true if S is a type-cast assignment.  */

static inline bool
gimple_assign_cast_p (const_gimple s)
{
  if (is_gimple_assign (s))
    {
      enum tree_code sc = gimple_assign_rhs_code (s);
      return CONVERT_EXPR_CODE_P (sc)
	     || sc == VIEW_CONVERT_EXPR
	     || sc == FIX_TRUNC_EXPR;
    }

  return false;
}

/* Return true if S is a clobber statement.  */

static inline bool
gimple_clobber_p (const_gimple s)
{
  return gimple_assign_single_p (s)
         && TREE_CLOBBER_P (gimple_assign_rhs1 (s));
}

/* Return true if GS is a GIMPLE_CALL.  */

static inline bool
is_gimple_call (const_gimple gs)
{
  return gimple_code (gs) == GIMPLE_CALL;
}

/* Return the LHS of call statement GS.  */

static inline tree
gimple_call_lhs (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  return gimple_op (gs, 0);
}


/* Return a pointer to the LHS of call statement GS.  */

static inline tree *
gimple_call_lhs_ptr (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  return gimple_op_ptr (gs, 0);
}


/* Set LHS to be the LHS operand of call statement GS.  */

static inline void
gimple_call_set_lhs (gimple gs, tree lhs)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  gimple_set_op (gs, 0, lhs);
  if (lhs && TREE_CODE (lhs) == SSA_NAME)
    SSA_NAME_DEF_STMT (lhs) = gs;
}


/* Return true if call GS calls an internal-only function, as enumerated
   by internal_fn.  */

static inline bool
gimple_call_internal_p (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  return (gs->subcode & GF_CALL_INTERNAL) != 0;
}


/* Return the target of internal call GS.  */

static inline enum internal_fn
gimple_call_internal_fn (const_gimple gs)
{
  gcc_gimple_checking_assert (gimple_call_internal_p (gs));
  return static_cast <const gimple_statement_call *> (gs)->u.internal_fn;
}


/* Return the function type of the function called by GS.  */

static inline tree
gimple_call_fntype (const_gimple gs)
{
  const gimple_statement_call *call_stmt =
    as_a <const gimple_statement_call *> (gs);
  if (gimple_call_internal_p (gs))
    return NULL_TREE;
  return call_stmt->u.fntype;
}

/* Set the type of the function called by GS to FNTYPE.  */

static inline void
gimple_call_set_fntype (gimple gs, tree fntype)
{
  gimple_statement_call *call_stmt = as_a <gimple_statement_call *> (gs);
  gcc_gimple_checking_assert (!gimple_call_internal_p (gs));
  call_stmt->u.fntype = fntype;
}


/* Return the tree node representing the function called by call
   statement GS.  */

static inline tree
gimple_call_fn (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  return gimple_op (gs, 1);
}

/* Return a pointer to the tree node representing the function called by call
   statement GS.  */

static inline tree *
gimple_call_fn_ptr (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  return gimple_op_ptr (gs, 1);
}


/* Set FN to be the function called by call statement GS.  */

static inline void
gimple_call_set_fn (gimple gs, tree fn)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  gcc_gimple_checking_assert (!gimple_call_internal_p (gs));
  gimple_set_op (gs, 1, fn);
}


/* Set FNDECL to be the function called by call statement GS.  */

static inline void
gimple_call_set_fndecl (gimple gs, tree decl)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  gcc_gimple_checking_assert (!gimple_call_internal_p (gs));
  gimple_set_op (gs, 1, build_fold_addr_expr_loc (gimple_location (gs), decl));
}


/* Set internal function FN to be the function called by call statement GS.  */

static inline void
gimple_call_set_internal_fn (gimple gs, enum internal_fn fn)
{
  gimple_statement_call *call_stmt = as_a <gimple_statement_call *> (gs);
  gcc_gimple_checking_assert (gimple_call_internal_p (gs));
  call_stmt->u.internal_fn = fn;
}


/* If a given GIMPLE_CALL's callee is a FUNCTION_DECL, return it.
   Otherwise return NULL.  This function is analogous to
   get_callee_fndecl in tree land.  */

static inline tree
gimple_call_fndecl (const_gimple gs)
{
  return gimple_call_addr_fndecl (gimple_call_fn (gs));
}


/* Return the type returned by call statement GS.  */

static inline tree
gimple_call_return_type (const_gimple gs)
{
  tree type = gimple_call_fntype (gs);

  if (type == NULL_TREE)
    return TREE_TYPE (gimple_call_lhs (gs));

  /* The type returned by a function is the type of its
     function type.  */
  return TREE_TYPE (type);
}


/* Return the static chain for call statement GS.  */

static inline tree
gimple_call_chain (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  return gimple_op (gs, 2);
}


/* Return a pointer to the static chain for call statement GS.  */

static inline tree *
gimple_call_chain_ptr (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  return gimple_op_ptr (gs, 2);
}

/* Set CHAIN to be the static chain for call statement GS.  */

static inline void
gimple_call_set_chain (gimple gs, tree chain)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);

  gimple_set_op (gs, 2, chain);
}


/* Return the number of arguments used by call statement GS.  */

static inline unsigned
gimple_call_num_args (const_gimple gs)
{
  unsigned num_ops;
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  num_ops = gimple_num_ops (gs);
  return num_ops - 3;
}


/* Return the argument at position INDEX for call statement GS.  */

static inline tree
gimple_call_arg (const_gimple gs, unsigned index)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  return gimple_op (gs, index + 3);
}


/* Return a pointer to the argument at position INDEX for call
   statement GS.  */

static inline tree *
gimple_call_arg_ptr (const_gimple gs, unsigned index)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  return gimple_op_ptr (gs, index + 3);
}


/* Set ARG to be the argument at position INDEX for call statement GS.  */

static inline void
gimple_call_set_arg (gimple gs, unsigned index, tree arg)
{
  GIMPLE_CHECK (gs, GIMPLE_CALL);
  gimple_set_op (gs, index + 3, arg);
}


/* If TAIL_P is true, mark call statement S as being a tail call
   (i.e., a call just before the exit of a function).  These calls are
   candidate for tail call optimization.  */

static inline void
gimple_call_set_tail (gimple s, bool tail_p)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  if (tail_p)
    s->subcode |= GF_CALL_TAILCALL;
  else
    s->subcode &= ~GF_CALL_TAILCALL;
}


/* Return true if GIMPLE_CALL S is marked as a tail call.  */

static inline bool
gimple_call_tail_p (gimple s)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  return (s->subcode & GF_CALL_TAILCALL) != 0;
}


/* If RETURN_SLOT_OPT_P is true mark GIMPLE_CALL S as valid for return
   slot optimization.  This transformation uses the target of the call
   expansion as the return slot for calls that return in memory.  */

static inline void
gimple_call_set_return_slot_opt (gimple s, bool return_slot_opt_p)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  if (return_slot_opt_p)
    s->subcode |= GF_CALL_RETURN_SLOT_OPT;
  else
    s->subcode &= ~GF_CALL_RETURN_SLOT_OPT;
}


/* Return true if S is marked for return slot optimization.  */

static inline bool
gimple_call_return_slot_opt_p (gimple s)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  return (s->subcode & GF_CALL_RETURN_SLOT_OPT) != 0;
}


/* If FROM_THUNK_P is true, mark GIMPLE_CALL S as being the jump from a
   thunk to the thunked-to function.  */

static inline void
gimple_call_set_from_thunk (gimple s, bool from_thunk_p)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  if (from_thunk_p)
    s->subcode |= GF_CALL_FROM_THUNK;
  else
    s->subcode &= ~GF_CALL_FROM_THUNK;
}


/* Return true if GIMPLE_CALL S is a jump from a thunk.  */

static inline bool
gimple_call_from_thunk_p (gimple s)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  return (s->subcode & GF_CALL_FROM_THUNK) != 0;
}


/* If PASS_ARG_PACK_P is true, GIMPLE_CALL S is a stdarg call that needs the
   argument pack in its argument list.  */

static inline void
gimple_call_set_va_arg_pack (gimple s, bool pass_arg_pack_p)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  if (pass_arg_pack_p)
    s->subcode |= GF_CALL_VA_ARG_PACK;
  else
    s->subcode &= ~GF_CALL_VA_ARG_PACK;
}


/* Return true if GIMPLE_CALL S is a stdarg call that needs the
   argument pack in its argument list.  */

static inline bool
gimple_call_va_arg_pack_p (gimple s)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  return (s->subcode & GF_CALL_VA_ARG_PACK) != 0;
}


/* Return true if S is a noreturn call.  */

static inline bool
gimple_call_noreturn_p (gimple s)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  return (gimple_call_flags (s) & ECF_NORETURN) != 0;
}


/* If NOTHROW_P is true, GIMPLE_CALL S is a call that is known to not throw
   even if the called function can throw in other cases.  */

static inline void
gimple_call_set_nothrow (gimple s, bool nothrow_p)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  if (nothrow_p)
    s->subcode |= GF_CALL_NOTHROW;
  else
    s->subcode &= ~GF_CALL_NOTHROW;
}

/* Return true if S is a nothrow call.  */

static inline bool
gimple_call_nothrow_p (gimple s)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  return (gimple_call_flags (s) & ECF_NOTHROW) != 0;
}

/* If FOR_VAR is true, GIMPLE_CALL S is a call to builtin_alloca that
   is known to be emitted for VLA objects.  Those are wrapped by
   stack_save/stack_restore calls and hence can't lead to unbounded
   stack growth even when they occur in loops.  */

static inline void
gimple_call_set_alloca_for_var (gimple s, bool for_var)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  if (for_var)
    s->subcode |= GF_CALL_ALLOCA_FOR_VAR;
  else
    s->subcode &= ~GF_CALL_ALLOCA_FOR_VAR;
}

/* Return true of S is a call to builtin_alloca emitted for VLA objects.  */

static inline bool
gimple_call_alloca_for_var_p (gimple s)
{
  GIMPLE_CHECK (s, GIMPLE_CALL);
  return (s->subcode & GF_CALL_ALLOCA_FOR_VAR) != 0;
}

/* Copy all the GF_CALL_* flags from ORIG_CALL to DEST_CALL.  */

static inline void
gimple_call_copy_flags (gimple dest_call, gimple orig_call)
{
  GIMPLE_CHECK (dest_call, GIMPLE_CALL);
  GIMPLE_CHECK (orig_call, GIMPLE_CALL);
  dest_call->subcode = orig_call->subcode;
}


/* Return a pointer to the points-to solution for the set of call-used
   variables of the call CALL.  */

static inline struct pt_solution *
gimple_call_use_set (gimple call)
{
  gimple_statement_call *call_stmt = as_a <gimple_statement_call *> (call);
  return &call_stmt->call_used;
}


/* Return a pointer to the points-to solution for the set of call-used
   variables of the call CALL.  */

static inline struct pt_solution *
gimple_call_clobber_set (gimple call)
{
  gimple_statement_call *call_stmt = as_a <gimple_statement_call *> (call);
  return &call_stmt->call_clobbered;
}


/* Returns true if this is a GIMPLE_ASSIGN or a GIMPLE_CALL with a
   non-NULL lhs.  */

static inline bool
gimple_has_lhs (gimple stmt)
{
  return (is_gimple_assign (stmt)
	  || (is_gimple_call (stmt)
	      && gimple_call_lhs (stmt) != NULL_TREE));
}


/* Return the code of the predicate computed by conditional statement GS.  */

static inline enum tree_code
gimple_cond_code (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_COND);
  return (enum tree_code) gs->subcode;
}


/* Set CODE to be the predicate code for the conditional statement GS.  */

static inline void
gimple_cond_set_code (gimple gs, enum tree_code code)
{
  GIMPLE_CHECK (gs, GIMPLE_COND);
  gs->subcode = code;
}


/* Return the LHS of the predicate computed by conditional statement GS.  */

static inline tree
gimple_cond_lhs (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_COND);
  return gimple_op (gs, 0);
}

/* Return the pointer to the LHS of the predicate computed by conditional
   statement GS.  */

static inline tree *
gimple_cond_lhs_ptr (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_COND);
  return gimple_op_ptr (gs, 0);
}

/* Set LHS to be the LHS operand of the predicate computed by
   conditional statement GS.  */

static inline void
gimple_cond_set_lhs (gimple gs, tree lhs)
{
  GIMPLE_CHECK (gs, GIMPLE_COND);
  gimple_set_op (gs, 0, lhs);
}


/* Return the RHS operand of the predicate computed by conditional GS.  */

static inline tree
gimple_cond_rhs (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_COND);
  return gimple_op (gs, 1);
}

/* Return the pointer to the RHS operand of the predicate computed by
   conditional GS.  */

static inline tree *
gimple_cond_rhs_ptr (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_COND);
  return gimple_op_ptr (gs, 1);
}


/* Set RHS to be the RHS operand of the predicate computed by
   conditional statement GS.  */

static inline void
gimple_cond_set_rhs (gimple gs, tree rhs)
{
  GIMPLE_CHECK (gs, GIMPLE_COND);
  gimple_set_op (gs, 1, rhs);
}


/* Return the label used by conditional statement GS when its
   predicate evaluates to true.  */

static inline tree
gimple_cond_true_label (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_COND);
  return gimple_op (gs, 2);
}


/* Set LABEL to be the label used by conditional statement GS when its
   predicate evaluates to true.  */

static inline void
gimple_cond_set_true_label (gimple gs, tree label)
{
  GIMPLE_CHECK (gs, GIMPLE_COND);
  gimple_set_op (gs, 2, label);
}


/* Set LABEL to be the label used by conditional statement GS when its
   predicate evaluates to false.  */

static inline void
gimple_cond_set_false_label (gimple gs, tree label)
{
  GIMPLE_CHECK (gs, GIMPLE_COND);
  gimple_set_op (gs, 3, label);
}


/* Return the label used by conditional statement GS when its
   predicate evaluates to false.  */

static inline tree
gimple_cond_false_label (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_COND);
  return gimple_op (gs, 3);
}


/* Set the conditional COND_STMT to be of the form 'if (1 == 0)'.  */

static inline void
gimple_cond_make_false (gimple gs)
{
  gimple_cond_set_lhs (gs, boolean_true_node);
  gimple_cond_set_rhs (gs, boolean_false_node);
  gs->subcode = EQ_EXPR;
}


/* Set the conditional COND_STMT to be of the form 'if (1 == 1)'.  */

static inline void
gimple_cond_make_true (gimple gs)
{
  gimple_cond_set_lhs (gs, boolean_true_node);
  gimple_cond_set_rhs (gs, boolean_true_node);
  gs->subcode = EQ_EXPR;
}

/* Check if conditional statemente GS is of the form 'if (1 == 1)',
  'if (0 == 0)', 'if (1 != 0)' or 'if (0 != 1)' */

static inline bool
gimple_cond_true_p (const_gimple gs)
{
  tree lhs = gimple_cond_lhs (gs);
  tree rhs = gimple_cond_rhs (gs);
  enum tree_code code = gimple_cond_code (gs);

  if (lhs != boolean_true_node && lhs != boolean_false_node)
    return false;

  if (rhs != boolean_true_node && rhs != boolean_false_node)
    return false;

  if (code == NE_EXPR && lhs != rhs)
    return true;

  if (code == EQ_EXPR && lhs == rhs)
      return true;

  return false;
}

/* Check if conditional statement GS is of the form 'if (1 != 1)',
   'if (0 != 0)', 'if (1 == 0)' or 'if (0 == 1)' */

static inline bool
gimple_cond_false_p (const_gimple gs)
{
  tree lhs = gimple_cond_lhs (gs);
  tree rhs = gimple_cond_rhs (gs);
  enum tree_code code = gimple_cond_code (gs);

  if (lhs != boolean_true_node && lhs != boolean_false_node)
    return false;

  if (rhs != boolean_true_node && rhs != boolean_false_node)
    return false;

  if (code == NE_EXPR && lhs == rhs)
    return true;

  if (code == EQ_EXPR && lhs != rhs)
      return true;

  return false;
}

/* Set the code, LHS and RHS of GIMPLE_COND STMT from CODE, LHS and RHS.  */

static inline void
gimple_cond_set_condition (gimple stmt, enum tree_code code, tree lhs, tree rhs)
{
  gimple_cond_set_code (stmt, code);
  gimple_cond_set_lhs (stmt, lhs);
  gimple_cond_set_rhs (stmt, rhs);
}

/* Return the LABEL_DECL node used by GIMPLE_LABEL statement GS.  */

static inline tree
gimple_label_label (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_LABEL);
  return gimple_op (gs, 0);
}


/* Set LABEL to be the LABEL_DECL node used by GIMPLE_LABEL statement
   GS.  */

static inline void
gimple_label_set_label (gimple gs, tree label)
{
  GIMPLE_CHECK (gs, GIMPLE_LABEL);
  gimple_set_op (gs, 0, label);
}


/* Return the destination of the unconditional jump GS.  */

static inline tree
gimple_goto_dest (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_GOTO);
  return gimple_op (gs, 0);
}


/* Set DEST to be the destination of the unconditonal jump GS.  */

static inline void
gimple_goto_set_dest (gimple gs, tree dest)
{
  GIMPLE_CHECK (gs, GIMPLE_GOTO);
  gimple_set_op (gs, 0, dest);
}


/* Return the variables declared in the GIMPLE_BIND statement GS.  */

static inline tree
gimple_bind_vars (const_gimple gs)
{
  const gimple_statement_bind *bind_stmt =
    as_a <const gimple_statement_bind *> (gs);
  return bind_stmt->vars;
}


/* Set VARS to be the set of variables declared in the GIMPLE_BIND
   statement GS.  */

static inline void
gimple_bind_set_vars (gimple gs, tree vars)
{
  gimple_statement_bind *bind_stmt = as_a <gimple_statement_bind *> (gs);
  bind_stmt->vars = vars;
}


/* Append VARS to the set of variables declared in the GIMPLE_BIND
   statement GS.  */

static inline void
gimple_bind_append_vars (gimple gs, tree vars)
{
  gimple_statement_bind *bind_stmt = as_a <gimple_statement_bind *> (gs);
  bind_stmt->vars = chainon (bind_stmt->vars, vars);
}


static inline gimple_seq *
gimple_bind_body_ptr (gimple gs)
{
  gimple_statement_bind *bind_stmt = as_a <gimple_statement_bind *> (gs);
  return &bind_stmt->body;
}

/* Return the GIMPLE sequence contained in the GIMPLE_BIND statement GS.  */

static inline gimple_seq
gimple_bind_body (gimple gs)
{
  return *gimple_bind_body_ptr (gs);
}


/* Set SEQ to be the GIMPLE sequence contained in the GIMPLE_BIND
   statement GS.  */

static inline void
gimple_bind_set_body (gimple gs, gimple_seq seq)
{
  gimple_statement_bind *bind_stmt = as_a <gimple_statement_bind *> (gs);
  bind_stmt->body = seq;
}


/* Append a statement to the end of a GIMPLE_BIND's body.  */

static inline void
gimple_bind_add_stmt (gimple gs, gimple stmt)
{
  gimple_statement_bind *bind_stmt = as_a <gimple_statement_bind *> (gs);
  gimple_seq_add_stmt (&bind_stmt->body, stmt);
}


/* Append a sequence of statements to the end of a GIMPLE_BIND's body.  */

static inline void
gimple_bind_add_seq (gimple gs, gimple_seq seq)
{
  gimple_statement_bind *bind_stmt = as_a <gimple_statement_bind *> (gs);
  gimple_seq_add_seq (&bind_stmt->body, seq);
}


/* Return the TREE_BLOCK node associated with GIMPLE_BIND statement
   GS.  This is analogous to the BIND_EXPR_BLOCK field in trees.  */

static inline tree
gimple_bind_block (const_gimple gs)
{
  const gimple_statement_bind *bind_stmt =
    as_a <const gimple_statement_bind *> (gs);
  return bind_stmt->block;
}


/* Set BLOCK to be the TREE_BLOCK node associated with GIMPLE_BIND
   statement GS.  */

static inline void
gimple_bind_set_block (gimple gs, tree block)
{
  gimple_statement_bind *bind_stmt = as_a <gimple_statement_bind *> (gs);
  gcc_gimple_checking_assert (block == NULL_TREE
			      || TREE_CODE (block) == BLOCK);
  bind_stmt->block = block;
}


/* Return the number of input operands for GIMPLE_ASM GS.  */

static inline unsigned
gimple_asm_ninputs (const_gimple gs)
{
  const gimple_statement_asm *asm_stmt =
    as_a <const gimple_statement_asm *> (gs);
  return asm_stmt->ni;
}


/* Return the number of output operands for GIMPLE_ASM GS.  */

static inline unsigned
gimple_asm_noutputs (const_gimple gs)
{
  const gimple_statement_asm *asm_stmt =
    as_a <const gimple_statement_asm *> (gs);
  return asm_stmt->no;
}


/* Return the number of clobber operands for GIMPLE_ASM GS.  */

static inline unsigned
gimple_asm_nclobbers (const_gimple gs)
{
  const gimple_statement_asm *asm_stmt =
    as_a <const gimple_statement_asm *> (gs);
  return asm_stmt->nc;
}

/* Return the number of label operands for GIMPLE_ASM GS.  */

static inline unsigned
gimple_asm_nlabels (const_gimple gs)
{
  const gimple_statement_asm *asm_stmt =
    as_a <const gimple_statement_asm *> (gs);
  return asm_stmt->nl;
}

/* Return input operand INDEX of GIMPLE_ASM GS.  */

static inline tree
gimple_asm_input_op (const_gimple gs, unsigned index)
{
  const gimple_statement_asm *asm_stmt =
    as_a <const gimple_statement_asm *> (gs);
  gcc_gimple_checking_assert (index < asm_stmt->ni);
  return gimple_op (gs, index + asm_stmt->no);
}

/* Return a pointer to input operand INDEX of GIMPLE_ASM GS.  */

static inline tree *
gimple_asm_input_op_ptr (const_gimple gs, unsigned index)
{
  const gimple_statement_asm *asm_stmt =
    as_a <const gimple_statement_asm *> (gs);
  gcc_gimple_checking_assert (index < asm_stmt->ni);
  return gimple_op_ptr (gs, index + asm_stmt->no);
}


/* Set IN_OP to be input operand INDEX in GIMPLE_ASM GS.  */

static inline void
gimple_asm_set_input_op (gimple gs, unsigned index, tree in_op)
{
  gimple_statement_asm *asm_stmt = as_a <gimple_statement_asm *> (gs);
  gcc_gimple_checking_assert (index < asm_stmt->ni
			      && TREE_CODE (in_op) == TREE_LIST);
  gimple_set_op (gs, index + asm_stmt->no, in_op);
}


/* Return output operand INDEX of GIMPLE_ASM GS.  */

static inline tree
gimple_asm_output_op (const_gimple gs, unsigned index)
{
  const gimple_statement_asm *asm_stmt =
    as_a <const gimple_statement_asm *> (gs);
  gcc_gimple_checking_assert (index < asm_stmt->no);
  return gimple_op (gs, index);
}

/* Return a pointer to output operand INDEX of GIMPLE_ASM GS.  */

static inline tree *
gimple_asm_output_op_ptr (const_gimple gs, unsigned index)
{
  const gimple_statement_asm *asm_stmt =
    as_a <const gimple_statement_asm *> (gs);
  gcc_gimple_checking_assert (index < asm_stmt->no);
  return gimple_op_ptr (gs, index);
}


/* Set OUT_OP to be output operand INDEX in GIMPLE_ASM GS.  */

static inline void
gimple_asm_set_output_op (gimple gs, unsigned index, tree out_op)
{
  gimple_statement_asm *asm_stmt = as_a <gimple_statement_asm *> (gs);
  gcc_gimple_checking_assert (index < asm_stmt->no
			      && TREE_CODE (out_op) == TREE_LIST);
  gimple_set_op (gs, index, out_op);
}


/* Return clobber operand INDEX of GIMPLE_ASM GS.  */

static inline tree
gimple_asm_clobber_op (const_gimple gs, unsigned index)
{
  const gimple_statement_asm *asm_stmt =
    as_a <const gimple_statement_asm *> (gs);
  gcc_gimple_checking_assert (index < asm_stmt->nc);
  return gimple_op (gs, index + asm_stmt->ni + asm_stmt->no);
}


/* Set CLOBBER_OP to be clobber operand INDEX in GIMPLE_ASM GS.  */

static inline void
gimple_asm_set_clobber_op (gimple gs, unsigned index, tree clobber_op)
{
  gimple_statement_asm *asm_stmt = as_a <gimple_statement_asm *> (gs);
  gcc_gimple_checking_assert (index < asm_stmt->nc
			      && TREE_CODE (clobber_op) == TREE_LIST);
  gimple_set_op (gs, index + asm_stmt->ni + asm_stmt->no, clobber_op);
}

/* Return label operand INDEX of GIMPLE_ASM GS.  */

static inline tree
gimple_asm_label_op (const_gimple gs, unsigned index)
{
  const gimple_statement_asm *asm_stmt =
    as_a <const gimple_statement_asm *> (gs);
  gcc_gimple_checking_assert (index < asm_stmt->nl);
  return gimple_op (gs, index + asm_stmt->ni + asm_stmt->nc);
}

/* Set LABEL_OP to be label operand INDEX in GIMPLE_ASM GS.  */

static inline void
gimple_asm_set_label_op (gimple gs, unsigned index, tree label_op)
{
  gimple_statement_asm *asm_stmt = as_a <gimple_statement_asm *> (gs);
  gcc_gimple_checking_assert (index < asm_stmt->nl
			      && TREE_CODE (label_op) == TREE_LIST);
  gimple_set_op (gs, index + asm_stmt->ni + asm_stmt->nc, label_op);
}

/* Return the string representing the assembly instruction in
   GIMPLE_ASM GS.  */

static inline const char *
gimple_asm_string (const_gimple gs)
{
  const gimple_statement_asm *asm_stmt =
    as_a <const gimple_statement_asm *> (gs);
  return asm_stmt->string;
}


/* Return true if GS is an asm statement marked volatile.  */

static inline bool
gimple_asm_volatile_p (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASM);
  return (gs->subcode & GF_ASM_VOLATILE) != 0;
}


/* If VOLATLE_P is true, mark asm statement GS as volatile.  */

static inline void
gimple_asm_set_volatile (gimple gs, bool volatile_p)
{
  GIMPLE_CHECK (gs, GIMPLE_ASM);
  if (volatile_p)
    gs->subcode |= GF_ASM_VOLATILE;
  else
    gs->subcode &= ~GF_ASM_VOLATILE;
}


/* If INPUT_P is true, mark asm GS as an ASM_INPUT.  */

static inline void
gimple_asm_set_input (gimple gs, bool input_p)
{
  GIMPLE_CHECK (gs, GIMPLE_ASM);
  if (input_p)
    gs->subcode |= GF_ASM_INPUT;
  else
    gs->subcode &= ~GF_ASM_INPUT;
}


/* Return true if asm GS is an ASM_INPUT.  */

static inline bool
gimple_asm_input_p (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_ASM);
  return (gs->subcode & GF_ASM_INPUT) != 0;
}


/* Return the types handled by GIMPLE_CATCH statement GS.  */

static inline tree
gimple_catch_types (const_gimple gs)
{
  const gimple_statement_catch *catch_stmt =
    as_a <const gimple_statement_catch *> (gs);
  return catch_stmt->types;
}


/* Return a pointer to the types handled by GIMPLE_CATCH statement GS.  */

static inline tree *
gimple_catch_types_ptr (gimple gs)
{
  gimple_statement_catch *catch_stmt = as_a <gimple_statement_catch *> (gs);
  return &catch_stmt->types;
}


/* Return a pointer to the GIMPLE sequence representing the body of
   the handler of GIMPLE_CATCH statement GS.  */

static inline gimple_seq *
gimple_catch_handler_ptr (gimple gs)
{
  gimple_statement_catch *catch_stmt = as_a <gimple_statement_catch *> (gs);
  return &catch_stmt->handler;
}


/* Return the GIMPLE sequence representing the body of the handler of
   GIMPLE_CATCH statement GS.  */

static inline gimple_seq
gimple_catch_handler (gimple gs)
{
  return *gimple_catch_handler_ptr (gs);
}


/* Set T to be the set of types handled by GIMPLE_CATCH GS.  */

static inline void
gimple_catch_set_types (gimple gs, tree t)
{
  gimple_statement_catch *catch_stmt = as_a <gimple_statement_catch *> (gs);
  catch_stmt->types = t;
}


/* Set HANDLER to be the body of GIMPLE_CATCH GS.  */

static inline void
gimple_catch_set_handler (gimple gs, gimple_seq handler)
{
  gimple_statement_catch *catch_stmt = as_a <gimple_statement_catch *> (gs);
  catch_stmt->handler = handler;
}


/* Return the types handled by GIMPLE_EH_FILTER statement GS.  */

static inline tree
gimple_eh_filter_types (const_gimple gs)
{
  const gimple_statement_eh_filter *eh_filter_stmt =
    as_a <const gimple_statement_eh_filter *> (gs);
  return eh_filter_stmt->types;
}


/* Return a pointer to the types handled by GIMPLE_EH_FILTER statement
   GS.  */

static inline tree *
gimple_eh_filter_types_ptr (gimple gs)
{
  gimple_statement_eh_filter *eh_filter_stmt =
    as_a <gimple_statement_eh_filter *> (gs);
  return &eh_filter_stmt->types;
}


/* Return a pointer to the sequence of statement to execute when
   GIMPLE_EH_FILTER statement fails.  */

static inline gimple_seq *
gimple_eh_filter_failure_ptr (gimple gs)
{
  gimple_statement_eh_filter *eh_filter_stmt =
    as_a <gimple_statement_eh_filter *> (gs);
  return &eh_filter_stmt->failure;
}


/* Return the sequence of statement to execute when GIMPLE_EH_FILTER
   statement fails.  */

static inline gimple_seq
gimple_eh_filter_failure (gimple gs)
{
  return *gimple_eh_filter_failure_ptr (gs);
}


/* Set TYPES to be the set of types handled by GIMPLE_EH_FILTER GS.  */

static inline void
gimple_eh_filter_set_types (gimple gs, tree types)
{
  gimple_statement_eh_filter *eh_filter_stmt =
    as_a <gimple_statement_eh_filter *> (gs);
  eh_filter_stmt->types = types;
}


/* Set FAILURE to be the sequence of statements to execute on failure
   for GIMPLE_EH_FILTER GS.  */

static inline void
gimple_eh_filter_set_failure (gimple gs, gimple_seq failure)
{
  gimple_statement_eh_filter *eh_filter_stmt =
    as_a <gimple_statement_eh_filter *> (gs);
  eh_filter_stmt->failure = failure;
}

/* Get the function decl to be called by the MUST_NOT_THROW region.  */

static inline tree
gimple_eh_must_not_throw_fndecl (gimple gs)
{
  gimple_statement_eh_mnt *eh_mnt_stmt = as_a <gimple_statement_eh_mnt *> (gs);
  return eh_mnt_stmt->fndecl;
}

/* Set the function decl to be called by GS to DECL.  */

static inline void
gimple_eh_must_not_throw_set_fndecl (gimple gs, tree decl)
{
  gimple_statement_eh_mnt *eh_mnt_stmt = as_a <gimple_statement_eh_mnt *> (gs);
  eh_mnt_stmt->fndecl = decl;
}

/* GIMPLE_EH_ELSE accessors.  */

static inline gimple_seq *
gimple_eh_else_n_body_ptr (gimple gs)
{
  gimple_statement_eh_else *eh_else_stmt =
    as_a <gimple_statement_eh_else *> (gs);
  return &eh_else_stmt->n_body;
}

static inline gimple_seq
gimple_eh_else_n_body (gimple gs)
{
  return *gimple_eh_else_n_body_ptr (gs);
}

static inline gimple_seq *
gimple_eh_else_e_body_ptr (gimple gs)
{
  gimple_statement_eh_else *eh_else_stmt =
    as_a <gimple_statement_eh_else *> (gs);
  return &eh_else_stmt->e_body;
}

static inline gimple_seq
gimple_eh_else_e_body (gimple gs)
{
  return *gimple_eh_else_e_body_ptr (gs);
}

static inline void
gimple_eh_else_set_n_body (gimple gs, gimple_seq seq)
{
  gimple_statement_eh_else *eh_else_stmt =
    as_a <gimple_statement_eh_else *> (gs);
  eh_else_stmt->n_body = seq;
}

static inline void
gimple_eh_else_set_e_body (gimple gs, gimple_seq seq)
{
  gimple_statement_eh_else *eh_else_stmt =
    as_a <gimple_statement_eh_else *> (gs);
  eh_else_stmt->e_body = seq;
}

/* GIMPLE_TRY accessors. */

/* Return the kind of try block represented by GIMPLE_TRY GS.  This is
   either GIMPLE_TRY_CATCH or GIMPLE_TRY_FINALLY.  */

static inline enum gimple_try_flags
gimple_try_kind (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_TRY);
  return (enum gimple_try_flags) (gs->subcode & GIMPLE_TRY_KIND);
}


/* Set the kind of try block represented by GIMPLE_TRY GS.  */

static inline void
gimple_try_set_kind (gimple gs, enum gimple_try_flags kind)
{
  GIMPLE_CHECK (gs, GIMPLE_TRY);
  gcc_gimple_checking_assert (kind == GIMPLE_TRY_CATCH
			      || kind == GIMPLE_TRY_FINALLY);
  if (gimple_try_kind (gs) != kind)
    gs->subcode = (unsigned int) kind;
}


/* Return the GIMPLE_TRY_CATCH_IS_CLEANUP flag.  */

static inline bool
gimple_try_catch_is_cleanup (const_gimple gs)
{
  gcc_gimple_checking_assert (gimple_try_kind (gs) == GIMPLE_TRY_CATCH);
  return (gs->subcode & GIMPLE_TRY_CATCH_IS_CLEANUP) != 0;
}


/* Return a pointer to the sequence of statements used as the
   body for GIMPLE_TRY GS.  */

static inline gimple_seq *
gimple_try_eval_ptr (gimple gs)
{
  gimple_statement_try *try_stmt = as_a <gimple_statement_try *> (gs);
  return &try_stmt->eval;
}


/* Return the sequence of statements used as the body for GIMPLE_TRY GS.  */

static inline gimple_seq
gimple_try_eval (gimple gs)
{
  return *gimple_try_eval_ptr (gs);
}


/* Return a pointer to the sequence of statements used as the cleanup body for
   GIMPLE_TRY GS.  */

static inline gimple_seq *
gimple_try_cleanup_ptr (gimple gs)
{
  gimple_statement_try *try_stmt = as_a <gimple_statement_try *> (gs);
  return &try_stmt->cleanup;
}


/* Return the sequence of statements used as the cleanup body for
   GIMPLE_TRY GS.  */

static inline gimple_seq
gimple_try_cleanup (gimple gs)
{
  return *gimple_try_cleanup_ptr (gs);
}


/* Set the GIMPLE_TRY_CATCH_IS_CLEANUP flag.  */

static inline void
gimple_try_set_catch_is_cleanup (gimple g, bool catch_is_cleanup)
{
  gcc_gimple_checking_assert (gimple_try_kind (g) == GIMPLE_TRY_CATCH);
  if (catch_is_cleanup)
    g->subcode |= GIMPLE_TRY_CATCH_IS_CLEANUP;
  else
    g->subcode &= ~GIMPLE_TRY_CATCH_IS_CLEANUP;
}


/* Set EVAL to be the sequence of statements to use as the body for
   GIMPLE_TRY GS.  */

static inline void
gimple_try_set_eval (gimple gs, gimple_seq eval)
{
  gimple_statement_try *try_stmt = as_a <gimple_statement_try *> (gs);
  try_stmt->eval = eval;
}


/* Set CLEANUP to be the sequence of statements to use as the cleanup
   body for GIMPLE_TRY GS.  */

static inline void
gimple_try_set_cleanup (gimple gs, gimple_seq cleanup)
{
  gimple_statement_try *try_stmt = as_a <gimple_statement_try *> (gs);
  try_stmt->cleanup = cleanup;
}


/* Return a pointer to the cleanup sequence for cleanup statement GS.  */

static inline gimple_seq *
gimple_wce_cleanup_ptr (gimple gs)
{
  gimple_statement_wce *wce_stmt = as_a <gimple_statement_wce *> (gs);
  return &wce_stmt->cleanup;
}


/* Return the cleanup sequence for cleanup statement GS.  */

static inline gimple_seq
gimple_wce_cleanup (gimple gs)
{
  return *gimple_wce_cleanup_ptr (gs);
}


/* Set CLEANUP to be the cleanup sequence for GS.  */

static inline void
gimple_wce_set_cleanup (gimple gs, gimple_seq cleanup)
{
  gimple_statement_wce *wce_stmt = as_a <gimple_statement_wce *> (gs);
  wce_stmt->cleanup = cleanup;
}


/* Return the CLEANUP_EH_ONLY flag for a WCE tuple.  */

static inline bool
gimple_wce_cleanup_eh_only (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_WITH_CLEANUP_EXPR);
  return gs->subcode != 0;
}


/* Set the CLEANUP_EH_ONLY flag for a WCE tuple.  */

static inline void
gimple_wce_set_cleanup_eh_only (gimple gs, bool eh_only_p)
{
  GIMPLE_CHECK (gs, GIMPLE_WITH_CLEANUP_EXPR);
  gs->subcode = (unsigned int) eh_only_p;
}


/* Return the maximum number of arguments supported by GIMPLE_PHI GS.  */

static inline unsigned
gimple_phi_capacity (const_gimple gs)
{
  const gimple_statement_phi *phi_stmt =
    as_a <const gimple_statement_phi *> (gs);
  return phi_stmt->capacity;
}


/* Return the number of arguments in GIMPLE_PHI GS.  This must always
   be exactly the number of incoming edges for the basic block holding
   GS.  */

static inline unsigned
gimple_phi_num_args (const_gimple gs)
{
  const gimple_statement_phi *phi_stmt =
    as_a <const gimple_statement_phi *> (gs);
  return phi_stmt->nargs;
}


/* Return the SSA name created by GIMPLE_PHI GS.  */

static inline tree
gimple_phi_result (const_gimple gs)
{
  const gimple_statement_phi *phi_stmt =
    as_a <const gimple_statement_phi *> (gs);
  return phi_stmt->result;
}

/* Return a pointer to the SSA name created by GIMPLE_PHI GS.  */

static inline tree *
gimple_phi_result_ptr (gimple gs)
{
  gimple_statement_phi *phi_stmt = as_a <gimple_statement_phi *> (gs);
  return &phi_stmt->result;
}

/* Set RESULT to be the SSA name created by GIMPLE_PHI GS.  */

static inline void
gimple_phi_set_result (gimple gs, tree result)
{
  gimple_statement_phi *phi_stmt = as_a <gimple_statement_phi *> (gs);
  phi_stmt->result = result;
  if (result && TREE_CODE (result) == SSA_NAME)
    SSA_NAME_DEF_STMT (result) = gs;
}


/* Return the PHI argument corresponding to incoming edge INDEX for
   GIMPLE_PHI GS.  */

static inline struct phi_arg_d *
gimple_phi_arg (gimple gs, unsigned index)
{
  gimple_statement_phi *phi_stmt = as_a <gimple_statement_phi *> (gs);
  gcc_gimple_checking_assert (index <= phi_stmt->capacity);
  return &(phi_stmt->args[index]);
}

/* Set PHIARG to be the argument corresponding to incoming edge INDEX
   for GIMPLE_PHI GS.  */

static inline void
gimple_phi_set_arg (gimple gs, unsigned index, struct phi_arg_d * phiarg)
{
  gimple_statement_phi *phi_stmt = as_a <gimple_statement_phi *> (gs);
  gcc_gimple_checking_assert (index <= phi_stmt->nargs);
  phi_stmt->args[index] = *phiarg;
}

/* Return the PHI nodes for basic block BB, or NULL if there are no
   PHI nodes.  */

static inline gimple_seq
phi_nodes (const_basic_block bb)
{
  gcc_checking_assert (!(bb->flags & BB_RTL));
  return bb->il.gimple.phi_nodes;
}

/* Return a pointer to the PHI nodes for basic block BB.  */

static inline gimple_seq *
phi_nodes_ptr (basic_block bb)
{
  gcc_checking_assert (!(bb->flags & BB_RTL));
  return &bb->il.gimple.phi_nodes;
}

/* Return the tree operand for argument I of PHI node GS.  */

static inline tree
gimple_phi_arg_def (gimple gs, size_t index)
{
  return gimple_phi_arg (gs, index)->def;
}


/* Return a pointer to the tree operand for argument I of PHI node GS.  */

static inline tree *
gimple_phi_arg_def_ptr (gimple gs, size_t index)
{
  return &gimple_phi_arg (gs, index)->def;
}

/* Return the edge associated with argument I of phi node GS.  */

static inline edge
gimple_phi_arg_edge (gimple gs, size_t i)
{
  return EDGE_PRED (gimple_bb (gs), i);
}

/* Return the source location of gimple argument I of phi node GS.  */

static inline source_location
gimple_phi_arg_location (gimple gs, size_t i)
{
  return gimple_phi_arg (gs, i)->locus;
}

/* Return the source location of the argument on edge E of phi node GS.  */

static inline source_location
gimple_phi_arg_location_from_edge (gimple gs, edge e)
{
  return gimple_phi_arg (gs, e->dest_idx)->locus;
}

/* Set the source location of gimple argument I of phi node GS to LOC.  */

static inline void
gimple_phi_arg_set_location (gimple gs, size_t i, source_location loc)
{
  gimple_phi_arg (gs, i)->locus = loc;
}

/* Return TRUE if argument I of phi node GS has a location record.  */

static inline bool
gimple_phi_arg_has_location (gimple gs, size_t i)
{
  return gimple_phi_arg_location (gs, i) != UNKNOWN_LOCATION;
}


/* Return the region number for GIMPLE_RESX GS.  */

static inline int
gimple_resx_region (const_gimple gs)
{
  const gimple_statement_resx *resx_stmt =
    as_a <const gimple_statement_resx *> (gs);
  return resx_stmt->region;
}

/* Set REGION to be the region number for GIMPLE_RESX GS.  */

static inline void
gimple_resx_set_region (gimple gs, int region)
{
  gimple_statement_resx *resx_stmt = as_a <gimple_statement_resx *> (gs);
  resx_stmt->region = region;
}

/* Return the region number for GIMPLE_EH_DISPATCH GS.  */

static inline int
gimple_eh_dispatch_region (const_gimple gs)
{
  const gimple_statement_eh_dispatch *eh_dispatch_stmt =
    as_a <const gimple_statement_eh_dispatch *> (gs);
  return eh_dispatch_stmt->region;
}

/* Set REGION to be the region number for GIMPLE_EH_DISPATCH GS.  */

static inline void
gimple_eh_dispatch_set_region (gimple gs, int region)
{
  gimple_statement_eh_dispatch *eh_dispatch_stmt =
    as_a <gimple_statement_eh_dispatch *> (gs);
  eh_dispatch_stmt->region = region;
}

/* Return the number of labels associated with the switch statement GS.  */

static inline unsigned
gimple_switch_num_labels (const_gimple gs)
{
  unsigned num_ops;
  GIMPLE_CHECK (gs, GIMPLE_SWITCH);
  num_ops = gimple_num_ops (gs);
  gcc_gimple_checking_assert (num_ops > 1);
  return num_ops - 1;
}


/* Set NLABELS to be the number of labels for the switch statement GS.  */

static inline void
gimple_switch_set_num_labels (gimple g, unsigned nlabels)
{
  GIMPLE_CHECK (g, GIMPLE_SWITCH);
  gimple_set_num_ops (g, nlabels + 1);
}


/* Return the index variable used by the switch statement GS.  */

static inline tree
gimple_switch_index (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_SWITCH);
  return gimple_op (gs, 0);
}


/* Return a pointer to the index variable for the switch statement GS.  */

static inline tree *
gimple_switch_index_ptr (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_SWITCH);
  return gimple_op_ptr (gs, 0);
}


/* Set INDEX to be the index variable for switch statement GS.  */

static inline void
gimple_switch_set_index (gimple gs, tree index)
{
  GIMPLE_CHECK (gs, GIMPLE_SWITCH);
  gcc_gimple_checking_assert (SSA_VAR_P (index) || CONSTANT_CLASS_P (index));
  gimple_set_op (gs, 0, index);
}


/* Return the label numbered INDEX.  The default label is 0, followed by any
   labels in a switch statement.  */

static inline tree
gimple_switch_label (const_gimple gs, unsigned index)
{
  GIMPLE_CHECK (gs, GIMPLE_SWITCH);
  gcc_gimple_checking_assert (gimple_num_ops (gs) > index + 1);
  return gimple_op (gs, index + 1);
}

/* Set the label number INDEX to LABEL.  0 is always the default label.  */

static inline void
gimple_switch_set_label (gimple gs, unsigned index, tree label)
{
  GIMPLE_CHECK (gs, GIMPLE_SWITCH);
  gcc_gimple_checking_assert (gimple_num_ops (gs) > index + 1
			      && (label == NULL_TREE
			          || TREE_CODE (label) == CASE_LABEL_EXPR));
  gimple_set_op (gs, index + 1, label);
}

/* Return the default label for a switch statement.  */

static inline tree
gimple_switch_default_label (const_gimple gs)
{
  tree label = gimple_switch_label (gs, 0);
  gcc_checking_assert (!CASE_LOW (label) && !CASE_HIGH (label));
  return label;
}

/* Set the default label for a switch statement.  */

static inline void
gimple_switch_set_default_label (gimple gs, tree label)
{
  gcc_checking_assert (!CASE_LOW (label) && !CASE_HIGH (label));
  gimple_switch_set_label (gs, 0, label);
}

/* Return true if GS is a GIMPLE_DEBUG statement.  */

static inline bool
is_gimple_debug (const_gimple gs)
{
  return gimple_code (gs) == GIMPLE_DEBUG;
}

/* Return true if S is a GIMPLE_DEBUG BIND statement.  */

static inline bool
gimple_debug_bind_p (const_gimple s)
{
  if (is_gimple_debug (s))
    return s->subcode == GIMPLE_DEBUG_BIND;

  return false;
}

/* Return the variable bound in a GIMPLE_DEBUG bind statement.  */

static inline tree
gimple_debug_bind_get_var (gimple dbg)
{
  GIMPLE_CHECK (dbg, GIMPLE_DEBUG);
  gcc_gimple_checking_assert (gimple_debug_bind_p (dbg));
  return gimple_op (dbg, 0);
}

/* Return the value bound to the variable in a GIMPLE_DEBUG bind
   statement.  */

static inline tree
gimple_debug_bind_get_value (gimple dbg)
{
  GIMPLE_CHECK (dbg, GIMPLE_DEBUG);
  gcc_gimple_checking_assert (gimple_debug_bind_p (dbg));
  return gimple_op (dbg, 1);
}

/* Return a pointer to the value bound to the variable in a
   GIMPLE_DEBUG bind statement.  */

static inline tree *
gimple_debug_bind_get_value_ptr (gimple dbg)
{
  GIMPLE_CHECK (dbg, GIMPLE_DEBUG);
  gcc_gimple_checking_assert (gimple_debug_bind_p (dbg));
  return gimple_op_ptr (dbg, 1);
}

/* Set the variable bound in a GIMPLE_DEBUG bind statement.  */

static inline void
gimple_debug_bind_set_var (gimple dbg, tree var)
{
  GIMPLE_CHECK (dbg, GIMPLE_DEBUG);
  gcc_gimple_checking_assert (gimple_debug_bind_p (dbg));
  gimple_set_op (dbg, 0, var);
}

/* Set the value bound to the variable in a GIMPLE_DEBUG bind
   statement.  */

static inline void
gimple_debug_bind_set_value (gimple dbg, tree value)
{
  GIMPLE_CHECK (dbg, GIMPLE_DEBUG);
  gcc_gimple_checking_assert (gimple_debug_bind_p (dbg));
  gimple_set_op (dbg, 1, value);
}

/* The second operand of a GIMPLE_DEBUG_BIND, when the value was
   optimized away.  */
#define GIMPLE_DEBUG_BIND_NOVALUE NULL_TREE /* error_mark_node */

/* Remove the value bound to the variable in a GIMPLE_DEBUG bind
   statement.  */

static inline void
gimple_debug_bind_reset_value (gimple dbg)
{
  GIMPLE_CHECK (dbg, GIMPLE_DEBUG);
  gcc_gimple_checking_assert (gimple_debug_bind_p (dbg));
  gimple_set_op (dbg, 1, GIMPLE_DEBUG_BIND_NOVALUE);
}

/* Return true if the GIMPLE_DEBUG bind statement is bound to a
   value.  */

static inline bool
gimple_debug_bind_has_value_p (gimple dbg)
{
  GIMPLE_CHECK (dbg, GIMPLE_DEBUG);
  gcc_gimple_checking_assert (gimple_debug_bind_p (dbg));
  return gimple_op (dbg, 1) != GIMPLE_DEBUG_BIND_NOVALUE;
}

#undef GIMPLE_DEBUG_BIND_NOVALUE

/* Return true if S is a GIMPLE_DEBUG SOURCE BIND statement.  */

static inline bool
gimple_debug_source_bind_p (const_gimple s)
{
  if (is_gimple_debug (s))
    return s->subcode == GIMPLE_DEBUG_SOURCE_BIND;

  return false;
}

/* Return the variable bound in a GIMPLE_DEBUG source bind statement.  */

static inline tree
gimple_debug_source_bind_get_var (gimple dbg)
{
  GIMPLE_CHECK (dbg, GIMPLE_DEBUG);
  gcc_gimple_checking_assert (gimple_debug_source_bind_p (dbg));
  return gimple_op (dbg, 0);
}

/* Return the value bound to the variable in a GIMPLE_DEBUG source bind
   statement.  */

static inline tree
gimple_debug_source_bind_get_value (gimple dbg)
{
  GIMPLE_CHECK (dbg, GIMPLE_DEBUG);
  gcc_gimple_checking_assert (gimple_debug_source_bind_p (dbg));
  return gimple_op (dbg, 1);
}

/* Return a pointer to the value bound to the variable in a
   GIMPLE_DEBUG source bind statement.  */

static inline tree *
gimple_debug_source_bind_get_value_ptr (gimple dbg)
{
  GIMPLE_CHECK (dbg, GIMPLE_DEBUG);
  gcc_gimple_checking_assert (gimple_debug_source_bind_p (dbg));
  return gimple_op_ptr (dbg, 1);
}

/* Set the variable bound in a GIMPLE_DEBUG source bind statement.  */

static inline void
gimple_debug_source_bind_set_var (gimple dbg, tree var)
{
  GIMPLE_CHECK (dbg, GIMPLE_DEBUG);
  gcc_gimple_checking_assert (gimple_debug_source_bind_p (dbg));
  gimple_set_op (dbg, 0, var);
}

/* Set the value bound to the variable in a GIMPLE_DEBUG source bind
   statement.  */

static inline void
gimple_debug_source_bind_set_value (gimple dbg, tree value)
{
  GIMPLE_CHECK (dbg, GIMPLE_DEBUG);
  gcc_gimple_checking_assert (gimple_debug_source_bind_p (dbg));
  gimple_set_op (dbg, 1, value);
}

/* Return the line number for EXPR, or return -1 if we have no line
   number information for it.  */
static inline int
get_lineno (const_gimple stmt)
{
  location_t loc;

  if (!stmt)
    return -1;

  loc = gimple_location (stmt);
  if (loc == UNKNOWN_LOCATION)
    return -1;

  return LOCATION_LINE (loc);
}

/* Return a pointer to the body for the OMP statement GS.  */

static inline gimple_seq *
gimple_omp_body_ptr (gimple gs)
{
  return &static_cast <gimple_statement_omp *> (gs)->body;
}

/* Return the body for the OMP statement GS.  */

static inline gimple_seq
gimple_omp_body (gimple gs)
{
  return *gimple_omp_body_ptr (gs);
}

/* Set BODY to be the body for the OMP statement GS.  */

static inline void
gimple_omp_set_body (gimple gs, gimple_seq body)
{
  static_cast <gimple_statement_omp *> (gs)->body = body;
}


/* Return the name associated with OMP_CRITICAL statement GS.  */

static inline tree
gimple_omp_critical_name (const_gimple gs)
{
  const gimple_statement_omp_critical *omp_critical_stmt =
    as_a <const gimple_statement_omp_critical *> (gs);
  return omp_critical_stmt->name;
}


/* Return a pointer to the name associated with OMP critical statement GS.  */

static inline tree *
gimple_omp_critical_name_ptr (gimple gs)
{
  gimple_statement_omp_critical *omp_critical_stmt =
    as_a <gimple_statement_omp_critical *> (gs);
  return &omp_critical_stmt->name;
}


/* Set NAME to be the name associated with OMP critical statement GS.  */

static inline void
gimple_omp_critical_set_name (gimple gs, tree name)
{
  gimple_statement_omp_critical *omp_critical_stmt =
    as_a <gimple_statement_omp_critical *> (gs);
  omp_critical_stmt->name = name;
}


/* Return the kind of OMP for statemement.  */

static inline int
gimple_omp_for_kind (const_gimple g)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_FOR);
  return (gimple_omp_subcode (g) & GF_OMP_FOR_KIND_MASK);
}


/* Set the OMP for kind.  */

static inline void
gimple_omp_for_set_kind (gimple g, int kind)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_FOR);
  g->subcode = (g->subcode & ~GF_OMP_FOR_KIND_MASK)
		      | (kind & GF_OMP_FOR_KIND_MASK);
}


/* Return true if OMP for statement G has the
   GF_OMP_FOR_COMBINED flag set.  */

static inline bool
gimple_omp_for_combined_p (const_gimple g)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_FOR);
  return (gimple_omp_subcode (g) & GF_OMP_FOR_COMBINED) != 0;
}


/* Set the GF_OMP_FOR_COMBINED field in G depending on the boolean
   value of COMBINED_P.  */

static inline void
gimple_omp_for_set_combined_p (gimple g, bool combined_p)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_FOR);
  if (combined_p)
    g->subcode |= GF_OMP_FOR_COMBINED;
  else
    g->subcode &= ~GF_OMP_FOR_COMBINED;
}


/* Return true if OMP for statement G has the
   GF_OMP_FOR_COMBINED_INTO flag set.  */

static inline bool
gimple_omp_for_combined_into_p (const_gimple g)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_FOR);
  return (gimple_omp_subcode (g) & GF_OMP_FOR_COMBINED_INTO) != 0;
}


/* Set the GF_OMP_FOR_COMBINED_INTO field in G depending on the boolean
   value of COMBINED_P.  */

static inline void
gimple_omp_for_set_combined_into_p (gimple g, bool combined_p)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_FOR);
  if (combined_p)
    g->subcode |= GF_OMP_FOR_COMBINED_INTO;
  else
    g->subcode &= ~GF_OMP_FOR_COMBINED_INTO;
}


/* Return the clauses associated with OMP_FOR GS.  */

static inline tree
gimple_omp_for_clauses (const_gimple gs)
{
  const gimple_statement_omp_for *omp_for_stmt =
    as_a <const gimple_statement_omp_for *> (gs);
  return omp_for_stmt->clauses;
}


/* Return a pointer to the OMP_FOR GS.  */

static inline tree *
gimple_omp_for_clauses_ptr (gimple gs)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  return &omp_for_stmt->clauses;
}


/* Set CLAUSES to be the list of clauses associated with OMP_FOR GS.  */

static inline void
gimple_omp_for_set_clauses (gimple gs, tree clauses)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  omp_for_stmt->clauses = clauses;
}


/* Get the collapse count of OMP_FOR GS.  */

static inline size_t
gimple_omp_for_collapse (gimple gs)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  return omp_for_stmt->collapse;
}


/* Return the index variable for OMP_FOR GS.  */

static inline tree
gimple_omp_for_index (const_gimple gs, size_t i)
{
  const gimple_statement_omp_for *omp_for_stmt =
    as_a <const gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  return omp_for_stmt->iter[i].index;
}


/* Return a pointer to the index variable for OMP_FOR GS.  */

static inline tree *
gimple_omp_for_index_ptr (gimple gs, size_t i)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  return &omp_for_stmt->iter[i].index;
}


/* Set INDEX to be the index variable for OMP_FOR GS.  */

static inline void
gimple_omp_for_set_index (gimple gs, size_t i, tree index)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  omp_for_stmt->iter[i].index = index;
}


/* Return the initial value for OMP_FOR GS.  */

static inline tree
gimple_omp_for_initial (const_gimple gs, size_t i)
{
  const gimple_statement_omp_for *omp_for_stmt =
    as_a <const gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  return omp_for_stmt->iter[i].initial;
}


/* Return a pointer to the initial value for OMP_FOR GS.  */

static inline tree *
gimple_omp_for_initial_ptr (gimple gs, size_t i)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  return &omp_for_stmt->iter[i].initial;
}


/* Set INITIAL to be the initial value for OMP_FOR GS.  */

static inline void
gimple_omp_for_set_initial (gimple gs, size_t i, tree initial)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  omp_for_stmt->iter[i].initial = initial;
}


/* Return the final value for OMP_FOR GS.  */

static inline tree
gimple_omp_for_final (const_gimple gs, size_t i)
{
  const gimple_statement_omp_for *omp_for_stmt =
    as_a <const gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  return omp_for_stmt->iter[i].final;
}


/* Return a pointer to the final value for OMP_FOR GS.  */

static inline tree *
gimple_omp_for_final_ptr (gimple gs, size_t i)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  return &omp_for_stmt->iter[i].final;
}


/* Set FINAL to be the final value for OMP_FOR GS.  */

static inline void
gimple_omp_for_set_final (gimple gs, size_t i, tree final)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  omp_for_stmt->iter[i].final = final;
}


/* Return the increment value for OMP_FOR GS.  */

static inline tree
gimple_omp_for_incr (const_gimple gs, size_t i)
{
  const gimple_statement_omp_for *omp_for_stmt =
    as_a <const gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  return omp_for_stmt->iter[i].incr;
}


/* Return a pointer to the increment value for OMP_FOR GS.  */

static inline tree *
gimple_omp_for_incr_ptr (gimple gs, size_t i)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  return &omp_for_stmt->iter[i].incr;
}


/* Set INCR to be the increment value for OMP_FOR GS.  */

static inline void
gimple_omp_for_set_incr (gimple gs, size_t i, tree incr)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  omp_for_stmt->iter[i].incr = incr;
}


/* Return a pointer to the sequence of statements to execute before the OMP_FOR
   statement GS starts.  */

static inline gimple_seq *
gimple_omp_for_pre_body_ptr (gimple gs)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  return &omp_for_stmt->pre_body;
}


/* Return the sequence of statements to execute before the OMP_FOR
   statement GS starts.  */

static inline gimple_seq
gimple_omp_for_pre_body (gimple gs)
{
  return *gimple_omp_for_pre_body_ptr (gs);
}


/* Set PRE_BODY to be the sequence of statements to execute before the
   OMP_FOR statement GS starts.  */

static inline void
gimple_omp_for_set_pre_body (gimple gs, gimple_seq pre_body)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  omp_for_stmt->pre_body = pre_body;
}


/* Return the clauses associated with OMP_PARALLEL GS.  */

static inline tree
gimple_omp_parallel_clauses (const_gimple gs)
{
  const gimple_statement_omp_parallel *omp_parallel_stmt =
    as_a <const gimple_statement_omp_parallel *> (gs);
  return omp_parallel_stmt->clauses;
}


/* Return a pointer to the clauses associated with OMP_PARALLEL GS.  */

static inline tree *
gimple_omp_parallel_clauses_ptr (gimple gs)
{
  gimple_statement_omp_parallel *omp_parallel_stmt =
    as_a <gimple_statement_omp_parallel *> (gs);
  return &omp_parallel_stmt->clauses;
}


/* Set CLAUSES to be the list of clauses associated with OMP_PARALLEL
   GS.  */

static inline void
gimple_omp_parallel_set_clauses (gimple gs, tree clauses)
{
  gimple_statement_omp_parallel *omp_parallel_stmt =
    as_a <gimple_statement_omp_parallel *> (gs);
  omp_parallel_stmt->clauses = clauses;
}


/* Return the child function used to hold the body of OMP_PARALLEL GS.  */

static inline tree
gimple_omp_parallel_child_fn (const_gimple gs)
{
  const gimple_statement_omp_parallel *omp_parallel_stmt =
    as_a <const gimple_statement_omp_parallel *> (gs);
  return omp_parallel_stmt->child_fn;
}

/* Return a pointer to the child function used to hold the body of
   OMP_PARALLEL GS.  */

static inline tree *
gimple_omp_parallel_child_fn_ptr (gimple gs)
{
  gimple_statement_omp_parallel *omp_parallel_stmt =
    as_a <gimple_statement_omp_parallel *> (gs);
  return &omp_parallel_stmt->child_fn;
}


/* Set CHILD_FN to be the child function for OMP_PARALLEL GS.  */

static inline void
gimple_omp_parallel_set_child_fn (gimple gs, tree child_fn)
{
  gimple_statement_omp_parallel *omp_parallel_stmt =
    as_a <gimple_statement_omp_parallel *> (gs);
  omp_parallel_stmt->child_fn = child_fn;
}


/* Return the artificial argument used to send variables and values
   from the parent to the children threads in OMP_PARALLEL GS.  */

static inline tree
gimple_omp_parallel_data_arg (const_gimple gs)
{
  const gimple_statement_omp_parallel *omp_parallel_stmt =
    as_a <const gimple_statement_omp_parallel *> (gs);
  return omp_parallel_stmt->data_arg;
}


/* Return a pointer to the data argument for OMP_PARALLEL GS.  */

static inline tree *
gimple_omp_parallel_data_arg_ptr (gimple gs)
{
  gimple_statement_omp_parallel *omp_parallel_stmt =
    as_a <gimple_statement_omp_parallel *> (gs);
  return &omp_parallel_stmt->data_arg;
}


/* Set DATA_ARG to be the data argument for OMP_PARALLEL GS.  */

static inline void
gimple_omp_parallel_set_data_arg (gimple gs, tree data_arg)
{
  gimple_statement_omp_parallel *omp_parallel_stmt =
    as_a <gimple_statement_omp_parallel *> (gs);
  omp_parallel_stmt->data_arg = data_arg;
}


/* Return the clauses associated with OMP_TASK GS.  */

static inline tree
gimple_omp_task_clauses (const_gimple gs)
{
  const gimple_statement_omp_task *omp_task_stmt =
    as_a <const gimple_statement_omp_task *> (gs);
  return omp_task_stmt->clauses;
}


/* Return a pointer to the clauses associated with OMP_TASK GS.  */

static inline tree *
gimple_omp_task_clauses_ptr (gimple gs)
{
  gimple_statement_omp_task *omp_task_stmt =
    as_a <gimple_statement_omp_task *> (gs);
  return &omp_task_stmt->clauses;
}


/* Set CLAUSES to be the list of clauses associated with OMP_TASK
   GS.  */

static inline void
gimple_omp_task_set_clauses (gimple gs, tree clauses)
{
  gimple_statement_omp_task *omp_task_stmt =
    as_a <gimple_statement_omp_task *> (gs);
  omp_task_stmt->clauses = clauses;
}


/* Return the child function used to hold the body of OMP_TASK GS.  */

static inline tree
gimple_omp_task_child_fn (const_gimple gs)
{
  const gimple_statement_omp_task *omp_task_stmt =
    as_a <const gimple_statement_omp_task *> (gs);
  return omp_task_stmt->child_fn;
}

/* Return a pointer to the child function used to hold the body of
   OMP_TASK GS.  */

static inline tree *
gimple_omp_task_child_fn_ptr (gimple gs)
{
  gimple_statement_omp_task *omp_task_stmt =
    as_a <gimple_statement_omp_task *> (gs);
  return &omp_task_stmt->child_fn;
}


/* Set CHILD_FN to be the child function for OMP_TASK GS.  */

static inline void
gimple_omp_task_set_child_fn (gimple gs, tree child_fn)
{
  gimple_statement_omp_task *omp_task_stmt =
    as_a <gimple_statement_omp_task *> (gs);
  omp_task_stmt->child_fn = child_fn;
}


/* Return the artificial argument used to send variables and values
   from the parent to the children threads in OMP_TASK GS.  */

static inline tree
gimple_omp_task_data_arg (const_gimple gs)
{
  const gimple_statement_omp_task *omp_task_stmt =
    as_a <const gimple_statement_omp_task *> (gs);
  return omp_task_stmt->data_arg;
}


/* Return a pointer to the data argument for OMP_TASK GS.  */

static inline tree *
gimple_omp_task_data_arg_ptr (gimple gs)
{
  gimple_statement_omp_task *omp_task_stmt =
    as_a <gimple_statement_omp_task *> (gs);
  return &omp_task_stmt->data_arg;
}


/* Set DATA_ARG to be the data argument for OMP_TASK GS.  */

static inline void
gimple_omp_task_set_data_arg (gimple gs, tree data_arg)
{
  gimple_statement_omp_task *omp_task_stmt =
    as_a <gimple_statement_omp_task *> (gs);
  omp_task_stmt->data_arg = data_arg;
}


/* Return the clauses associated with OMP_TASK GS.  */

static inline tree
gimple_omp_taskreg_clauses (const_gimple gs)
{
  const gimple_statement_omp_taskreg *omp_taskreg_stmt =
    as_a <const gimple_statement_omp_taskreg *> (gs);
  return omp_taskreg_stmt->clauses;
}


/* Return a pointer to the clauses associated with OMP_TASK GS.  */

static inline tree *
gimple_omp_taskreg_clauses_ptr (gimple gs)
{
  gimple_statement_omp_taskreg *omp_taskreg_stmt =
    as_a <gimple_statement_omp_taskreg *> (gs);
  return &omp_taskreg_stmt->clauses;
}


/* Set CLAUSES to be the list of clauses associated with OMP_TASK
   GS.  */

static inline void
gimple_omp_taskreg_set_clauses (gimple gs, tree clauses)
{
  gimple_statement_omp_taskreg *omp_taskreg_stmt =
    as_a <gimple_statement_omp_taskreg *> (gs);
  omp_taskreg_stmt->clauses = clauses;
}


/* Return the child function used to hold the body of OMP_TASK GS.  */

static inline tree
gimple_omp_taskreg_child_fn (const_gimple gs)
{
  const gimple_statement_omp_taskreg *omp_taskreg_stmt =
    as_a <const gimple_statement_omp_taskreg *> (gs);
  return omp_taskreg_stmt->child_fn;
}

/* Return a pointer to the child function used to hold the body of
   OMP_TASK GS.  */

static inline tree *
gimple_omp_taskreg_child_fn_ptr (gimple gs)
{
  gimple_statement_omp_taskreg *omp_taskreg_stmt =
    as_a <gimple_statement_omp_taskreg *> (gs);
  return &omp_taskreg_stmt->child_fn;
}


/* Set CHILD_FN to be the child function for OMP_TASK GS.  */

static inline void
gimple_omp_taskreg_set_child_fn (gimple gs, tree child_fn)
{
  gimple_statement_omp_taskreg *omp_taskreg_stmt =
    as_a <gimple_statement_omp_taskreg *> (gs);
  omp_taskreg_stmt->child_fn = child_fn;
}


/* Return the artificial argument used to send variables and values
   from the parent to the children threads in OMP_TASK GS.  */

static inline tree
gimple_omp_taskreg_data_arg (const_gimple gs)
{
  const gimple_statement_omp_taskreg *omp_taskreg_stmt =
    as_a <const gimple_statement_omp_taskreg *> (gs);
  return omp_taskreg_stmt->data_arg;
}


/* Return a pointer to the data argument for OMP_TASK GS.  */

static inline tree *
gimple_omp_taskreg_data_arg_ptr (gimple gs)
{
  gimple_statement_omp_taskreg *omp_taskreg_stmt =
    as_a <gimple_statement_omp_taskreg *> (gs);
  return &omp_taskreg_stmt->data_arg;
}


/* Set DATA_ARG to be the data argument for OMP_TASK GS.  */

static inline void
gimple_omp_taskreg_set_data_arg (gimple gs, tree data_arg)
{
  gimple_statement_omp_taskreg *omp_taskreg_stmt =
    as_a <gimple_statement_omp_taskreg *> (gs);
  omp_taskreg_stmt->data_arg = data_arg;
}


/* Return the copy function used to hold the body of OMP_TASK GS.  */

static inline tree
gimple_omp_task_copy_fn (const_gimple gs)
{
  const gimple_statement_omp_task *omp_task_stmt =
    as_a <const gimple_statement_omp_task *> (gs);
  return omp_task_stmt->copy_fn;
}

/* Return a pointer to the copy function used to hold the body of
   OMP_TASK GS.  */

static inline tree *
gimple_omp_task_copy_fn_ptr (gimple gs)
{
  gimple_statement_omp_task *omp_task_stmt =
    as_a <gimple_statement_omp_task *> (gs);
  return &omp_task_stmt->copy_fn;
}


/* Set CHILD_FN to be the copy function for OMP_TASK GS.  */

static inline void
gimple_omp_task_set_copy_fn (gimple gs, tree copy_fn)
{
  gimple_statement_omp_task *omp_task_stmt =
    as_a <gimple_statement_omp_task *> (gs);
  omp_task_stmt->copy_fn = copy_fn;
}


/* Return size of the data block in bytes in OMP_TASK GS.  */

static inline tree
gimple_omp_task_arg_size (const_gimple gs)
{
  const gimple_statement_omp_task *omp_task_stmt =
    as_a <const gimple_statement_omp_task *> (gs);
  return omp_task_stmt->arg_size;
}


/* Return a pointer to the data block size for OMP_TASK GS.  */

static inline tree *
gimple_omp_task_arg_size_ptr (gimple gs)
{
  gimple_statement_omp_task *omp_task_stmt =
    as_a <gimple_statement_omp_task *> (gs);
  return &omp_task_stmt->arg_size;
}


/* Set ARG_SIZE to be the data block size for OMP_TASK GS.  */

static inline void
gimple_omp_task_set_arg_size (gimple gs, tree arg_size)
{
  gimple_statement_omp_task *omp_task_stmt =
    as_a <gimple_statement_omp_task *> (gs);
  omp_task_stmt->arg_size = arg_size;
}


/* Return align of the data block in bytes in OMP_TASK GS.  */

static inline tree
gimple_omp_task_arg_align (const_gimple gs)
{
  const gimple_statement_omp_task *omp_task_stmt =
    as_a <const gimple_statement_omp_task *> (gs);
  return omp_task_stmt->arg_align;
}


/* Return a pointer to the data block align for OMP_TASK GS.  */

static inline tree *
gimple_omp_task_arg_align_ptr (gimple gs)
{
  gimple_statement_omp_task *omp_task_stmt =
    as_a <gimple_statement_omp_task *> (gs);
  return &omp_task_stmt->arg_align;
}


/* Set ARG_SIZE to be the data block align for OMP_TASK GS.  */

static inline void
gimple_omp_task_set_arg_align (gimple gs, tree arg_align)
{
  gimple_statement_omp_task *omp_task_stmt =
    as_a <gimple_statement_omp_task *> (gs);
  omp_task_stmt->arg_align = arg_align;
}


/* Return the clauses associated with OMP_SINGLE GS.  */

static inline tree
gimple_omp_single_clauses (const_gimple gs)
{
  const gimple_statement_omp_single *omp_single_stmt =
    as_a <const gimple_statement_omp_single *> (gs);
  return omp_single_stmt->clauses;
}


/* Return a pointer to the clauses associated with OMP_SINGLE GS.  */

static inline tree *
gimple_omp_single_clauses_ptr (gimple gs)
{
  gimple_statement_omp_single *omp_single_stmt =
    as_a <gimple_statement_omp_single *> (gs);
  return &omp_single_stmt->clauses;
}


/* Set CLAUSES to be the clauses associated with OMP_SINGLE GS.  */

static inline void
gimple_omp_single_set_clauses (gimple gs, tree clauses)
{
  gimple_statement_omp_single *omp_single_stmt =
    as_a <gimple_statement_omp_single *> (gs);
  omp_single_stmt->clauses = clauses;
}


/* Return the clauses associated with OMP_TARGET GS.  */

static inline tree
gimple_omp_target_clauses (const_gimple gs)
{
  const gimple_statement_omp_target *omp_target_stmt =
    as_a <const gimple_statement_omp_target *> (gs);
  return omp_target_stmt->clauses;
}


/* Return a pointer to the clauses associated with OMP_TARGET GS.  */

static inline tree *
gimple_omp_target_clauses_ptr (gimple gs)
{
  gimple_statement_omp_target *omp_target_stmt =
    as_a <gimple_statement_omp_target *> (gs);
  return &omp_target_stmt->clauses;
}


/* Set CLAUSES to be the clauses associated with OMP_TARGET GS.  */

static inline void
gimple_omp_target_set_clauses (gimple gs, tree clauses)
{
  gimple_statement_omp_target *omp_target_stmt =
    as_a <gimple_statement_omp_target *> (gs);
  omp_target_stmt->clauses = clauses;
}


/* Return the kind of OMP target statemement.  */

static inline int
gimple_omp_target_kind (const_gimple g)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_TARGET);
  return (gimple_omp_subcode (g) & GF_OMP_TARGET_KIND_MASK);
}


/* Set the OMP target kind.  */

static inline void
gimple_omp_target_set_kind (gimple g, int kind)
{
  GIMPLE_CHECK (g, GIMPLE_OMP_TARGET);
  g->subcode = (g->subcode & ~GF_OMP_TARGET_KIND_MASK)
		      | (kind & GF_OMP_TARGET_KIND_MASK);
}


/* Return the child function used to hold the body of OMP_TARGET GS.  */

static inline tree
gimple_omp_target_child_fn (const_gimple gs)
{
  const gimple_statement_omp_target *omp_target_stmt =
    as_a <const gimple_statement_omp_target *> (gs);
  return omp_target_stmt->child_fn;
}

/* Return a pointer to the child function used to hold the body of
   OMP_TARGET GS.  */

static inline tree *
gimple_omp_target_child_fn_ptr (gimple gs)
{
  gimple_statement_omp_target *omp_target_stmt =
    as_a <gimple_statement_omp_target *> (gs);
  return &omp_target_stmt->child_fn;
}


/* Set CHILD_FN to be the child function for OMP_TARGET GS.  */

static inline void
gimple_omp_target_set_child_fn (gimple gs, tree child_fn)
{
  gimple_statement_omp_target *omp_target_stmt =
    as_a <gimple_statement_omp_target *> (gs);
  omp_target_stmt->child_fn = child_fn;
}


/* Return the artificial argument used to send variables and values
   from the parent to the children threads in OMP_TARGET GS.  */

static inline tree
gimple_omp_target_data_arg (const_gimple gs)
{
  const gimple_statement_omp_target *omp_target_stmt =
    as_a <const gimple_statement_omp_target *> (gs);
  return omp_target_stmt->data_arg;
}


/* Return a pointer to the data argument for OMP_TARGET GS.  */

static inline tree *
gimple_omp_target_data_arg_ptr (gimple gs)
{
  gimple_statement_omp_target *omp_target_stmt =
    as_a <gimple_statement_omp_target *> (gs);
  return &omp_target_stmt->data_arg;
}


/* Set DATA_ARG to be the data argument for OMP_TARGET GS.  */

static inline void
gimple_omp_target_set_data_arg (gimple gs, tree data_arg)
{
  gimple_statement_omp_target *omp_target_stmt =
    as_a <gimple_statement_omp_target *> (gs);
  omp_target_stmt->data_arg = data_arg;
}


/* Return the clauses associated with OMP_TEAMS GS.  */

static inline tree
gimple_omp_teams_clauses (const_gimple gs)
{
  const gimple_statement_omp_teams *omp_teams_stmt =
    as_a <const gimple_statement_omp_teams *> (gs);
  return omp_teams_stmt->clauses;
}


/* Return a pointer to the clauses associated with OMP_TEAMS GS.  */

static inline tree *
gimple_omp_teams_clauses_ptr (gimple gs)
{
  gimple_statement_omp_teams *omp_teams_stmt =
    as_a <gimple_statement_omp_teams *> (gs);
  return &omp_teams_stmt->clauses;
}


/* Set CLAUSES to be the clauses associated with OMP_TEAMS GS.  */

static inline void
gimple_omp_teams_set_clauses (gimple gs, tree clauses)
{
  gimple_statement_omp_teams *omp_teams_stmt =
    as_a <gimple_statement_omp_teams *> (gs);
  omp_teams_stmt->clauses = clauses;
}


/* Return the clauses associated with OMP_SECTIONS GS.  */

static inline tree
gimple_omp_sections_clauses (const_gimple gs)
{
  const gimple_statement_omp_sections *omp_sections_stmt =
    as_a <const gimple_statement_omp_sections *> (gs);
  return omp_sections_stmt->clauses;
}


/* Return a pointer to the clauses associated with OMP_SECTIONS GS.  */

static inline tree *
gimple_omp_sections_clauses_ptr (gimple gs)
{
  gimple_statement_omp_sections *omp_sections_stmt =
    as_a <gimple_statement_omp_sections *> (gs);
  return &omp_sections_stmt->clauses;
}


/* Set CLAUSES to be the set of clauses associated with OMP_SECTIONS
   GS.  */

static inline void
gimple_omp_sections_set_clauses (gimple gs, tree clauses)
{
  gimple_statement_omp_sections *omp_sections_stmt =
    as_a <gimple_statement_omp_sections *> (gs);
  omp_sections_stmt->clauses = clauses;
}


/* Return the control variable associated with the GIMPLE_OMP_SECTIONS
   in GS.  */

static inline tree
gimple_omp_sections_control (const_gimple gs)
{
  const gimple_statement_omp_sections *omp_sections_stmt =
    as_a <const gimple_statement_omp_sections *> (gs);
  return omp_sections_stmt->control;
}


/* Return a pointer to the clauses associated with the GIMPLE_OMP_SECTIONS
   GS.  */

static inline tree *
gimple_omp_sections_control_ptr (gimple gs)
{
  gimple_statement_omp_sections *omp_sections_stmt =
    as_a <gimple_statement_omp_sections *> (gs);
  return &omp_sections_stmt->control;
}


/* Set CONTROL to be the set of clauses associated with the
   GIMPLE_OMP_SECTIONS in GS.  */

static inline void
gimple_omp_sections_set_control (gimple gs, tree control)
{
  gimple_statement_omp_sections *omp_sections_stmt =
    as_a <gimple_statement_omp_sections *> (gs);
  omp_sections_stmt->control = control;
}


/* Set COND to be the condition code for OMP_FOR GS.  */

static inline void
gimple_omp_for_set_cond (gimple gs, size_t i, enum tree_code cond)
{
  gimple_statement_omp_for *omp_for_stmt =
    as_a <gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (TREE_CODE_CLASS (cond) == tcc_comparison
			      && i < omp_for_stmt->collapse);
  omp_for_stmt->iter[i].cond = cond;
}


/* Return the condition code associated with OMP_FOR GS.  */

static inline enum tree_code
gimple_omp_for_cond (const_gimple gs, size_t i)
{
  const gimple_statement_omp_for *omp_for_stmt =
    as_a <const gimple_statement_omp_for *> (gs);
  gcc_gimple_checking_assert (i < omp_for_stmt->collapse);
  return omp_for_stmt->iter[i].cond;
}


/* Set the value being stored in an atomic store.  */

static inline void
gimple_omp_atomic_store_set_val (gimple g, tree val)
{
  gimple_statement_omp_atomic_store *omp_atomic_store_stmt =
    as_a <gimple_statement_omp_atomic_store *> (g);
  omp_atomic_store_stmt->val = val;
}


/* Return the value being stored in an atomic store.  */

static inline tree
gimple_omp_atomic_store_val (const_gimple g)
{
  const gimple_statement_omp_atomic_store *omp_atomic_store_stmt =
    as_a <const gimple_statement_omp_atomic_store *> (g);
  return omp_atomic_store_stmt->val;
}


/* Return a pointer to the value being stored in an atomic store.  */

static inline tree *
gimple_omp_atomic_store_val_ptr (gimple g)
{
  gimple_statement_omp_atomic_store *omp_atomic_store_stmt =
    as_a <gimple_statement_omp_atomic_store *> (g);
  return &omp_atomic_store_stmt->val;
}


/* Set the LHS of an atomic load.  */

static inline void
gimple_omp_atomic_load_set_lhs (gimple g, tree lhs)
{
  gimple_statement_omp_atomic_load *omp_atomic_load_stmt =
    as_a <gimple_statement_omp_atomic_load *> (g);
  omp_atomic_load_stmt->lhs = lhs;
}


/* Get the LHS of an atomic load.  */

static inline tree
gimple_omp_atomic_load_lhs (const_gimple g)
{
  const gimple_statement_omp_atomic_load *omp_atomic_load_stmt =
    as_a <const gimple_statement_omp_atomic_load *> (g);
  return omp_atomic_load_stmt->lhs;
}


/* Return a pointer to the LHS of an atomic load.  */

static inline tree *
gimple_omp_atomic_load_lhs_ptr (gimple g)
{
  gimple_statement_omp_atomic_load *omp_atomic_load_stmt =
    as_a <gimple_statement_omp_atomic_load *> (g);
  return &omp_atomic_load_stmt->lhs;
}


/* Set the RHS of an atomic load.  */

static inline void
gimple_omp_atomic_load_set_rhs (gimple g, tree rhs)
{
  gimple_statement_omp_atomic_load *omp_atomic_load_stmt =
    as_a <gimple_statement_omp_atomic_load *> (g);
  omp_atomic_load_stmt->rhs = rhs;
}


/* Get the RHS of an atomic load.  */

static inline tree
gimple_omp_atomic_load_rhs (const_gimple g)
{
  const gimple_statement_omp_atomic_load *omp_atomic_load_stmt =
    as_a <const gimple_statement_omp_atomic_load *> (g);
  return omp_atomic_load_stmt->rhs;
}


/* Return a pointer to the RHS of an atomic load.  */

static inline tree *
gimple_omp_atomic_load_rhs_ptr (gimple g)
{
  gimple_statement_omp_atomic_load *omp_atomic_load_stmt =
    as_a <gimple_statement_omp_atomic_load *> (g);
  return &omp_atomic_load_stmt->rhs;
}


/* Get the definition of the control variable in a GIMPLE_OMP_CONTINUE.  */

static inline tree
gimple_omp_continue_control_def (const_gimple g)
{
  const gimple_statement_omp_continue *omp_continue_stmt =
    as_a <const gimple_statement_omp_continue *> (g);
  return omp_continue_stmt->control_def;
}

/* The same as above, but return the address.  */

static inline tree *
gimple_omp_continue_control_def_ptr (gimple g)
{
  gimple_statement_omp_continue *omp_continue_stmt =
    as_a <gimple_statement_omp_continue *> (g);
  return &omp_continue_stmt->control_def;
}

/* Set the definition of the control variable in a GIMPLE_OMP_CONTINUE.  */

static inline void
gimple_omp_continue_set_control_def (gimple g, tree def)
{
  gimple_statement_omp_continue *omp_continue_stmt =
    as_a <gimple_statement_omp_continue *> (g);
  omp_continue_stmt->control_def = def;
}


/* Get the use of the control variable in a GIMPLE_OMP_CONTINUE.  */

static inline tree
gimple_omp_continue_control_use (const_gimple g)
{
  const gimple_statement_omp_continue *omp_continue_stmt =
    as_a <const gimple_statement_omp_continue *> (g);
  return omp_continue_stmt->control_use;
}


/* The same as above, but return the address.  */

static inline tree *
gimple_omp_continue_control_use_ptr (gimple g)
{
  gimple_statement_omp_continue *omp_continue_stmt =
    as_a <gimple_statement_omp_continue *> (g);
  return &omp_continue_stmt->control_use;
}


/* Set the use of the control variable in a GIMPLE_OMP_CONTINUE.  */

static inline void
gimple_omp_continue_set_control_use (gimple g, tree use)
{
  gimple_statement_omp_continue *omp_continue_stmt =
    as_a <gimple_statement_omp_continue *> (g);
  omp_continue_stmt->control_use = use;
}

/* Return a pointer to the body for the GIMPLE_TRANSACTION statement GS.  */

static inline gimple_seq *
gimple_transaction_body_ptr (gimple gs)
{
  gimple_statement_transaction *transaction_stmt =
    as_a <gimple_statement_transaction *> (gs);
  return &transaction_stmt->body;
}

/* Return the body for the GIMPLE_TRANSACTION statement GS.  */

static inline gimple_seq
gimple_transaction_body (gimple gs)
{
  return *gimple_transaction_body_ptr (gs);
}

/* Return the label associated with a GIMPLE_TRANSACTION.  */

static inline tree
gimple_transaction_label (const_gimple gs)
{
  const gimple_statement_transaction *transaction_stmt =
    as_a <const gimple_statement_transaction *> (gs);
  return transaction_stmt->label;
}

static inline tree *
gimple_transaction_label_ptr (gimple gs)
{
  gimple_statement_transaction *transaction_stmt =
    as_a <gimple_statement_transaction *> (gs);
  return &transaction_stmt->label;
}

/* Return the subcode associated with a GIMPLE_TRANSACTION.  */

static inline unsigned int
gimple_transaction_subcode (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_TRANSACTION);
  return gs->subcode;
}

/* Set BODY to be the body for the GIMPLE_TRANSACTION statement GS.  */

static inline void
gimple_transaction_set_body (gimple gs, gimple_seq body)
{
  gimple_statement_transaction *transaction_stmt =
    as_a <gimple_statement_transaction *> (gs);
  transaction_stmt->body = body;
}

/* Set the label associated with a GIMPLE_TRANSACTION.  */

static inline void
gimple_transaction_set_label (gimple gs, tree label)
{
  gimple_statement_transaction *transaction_stmt =
    as_a <gimple_statement_transaction *> (gs);
  transaction_stmt->label = label;
}

/* Set the subcode associated with a GIMPLE_TRANSACTION.  */

static inline void
gimple_transaction_set_subcode (gimple gs, unsigned int subcode)
{
  GIMPLE_CHECK (gs, GIMPLE_TRANSACTION);
  gs->subcode = subcode;
}


/* Return a pointer to the return value for GIMPLE_RETURN GS.  */

static inline tree *
gimple_return_retval_ptr (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_RETURN);
  return gimple_op_ptr (gs, 0);
}

/* Return the return value for GIMPLE_RETURN GS.  */

static inline tree
gimple_return_retval (const_gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_RETURN);
  return gimple_op (gs, 0);
}


/* Set RETVAL to be the return value for GIMPLE_RETURN GS.  */

static inline void
gimple_return_set_retval (gimple gs, tree retval)
{
  GIMPLE_CHECK (gs, GIMPLE_RETURN);
  gimple_set_op (gs, 0, retval);
}


/* Returns true when the gimple statement STMT is any of the OpenMP types.  */

#define CASE_GIMPLE_OMP				\
    case GIMPLE_OMP_PARALLEL:			\
    case GIMPLE_OMP_TASK:			\
    case GIMPLE_OMP_FOR:			\
    case GIMPLE_OMP_SECTIONS:			\
    case GIMPLE_OMP_SECTIONS_SWITCH:		\
    case GIMPLE_OMP_SINGLE:			\
    case GIMPLE_OMP_TARGET:			\
    case GIMPLE_OMP_TEAMS:			\
    case GIMPLE_OMP_SECTION:			\
    case GIMPLE_OMP_MASTER:			\
    case GIMPLE_OMP_TASKGROUP:			\
    case GIMPLE_OMP_ORDERED:			\
    case GIMPLE_OMP_CRITICAL:			\
    case GIMPLE_OMP_RETURN:			\
    case GIMPLE_OMP_ATOMIC_LOAD:		\
    case GIMPLE_OMP_ATOMIC_STORE:		\
    case GIMPLE_OMP_CONTINUE

static inline bool
is_gimple_omp (const_gimple stmt)
{
  switch (gimple_code (stmt))
    {
    CASE_GIMPLE_OMP:
      return true;
    default:
      return false;
    }
}


/* Returns TRUE if statement G is a GIMPLE_NOP.  */

static inline bool
gimple_nop_p (const_gimple g)
{
  return gimple_code (g) == GIMPLE_NOP;
}


/* Return true if GS is a GIMPLE_RESX.  */

static inline bool
is_gimple_resx (const_gimple gs)
{
  return gimple_code (gs) == GIMPLE_RESX;
}

/* Return the predictor of GIMPLE_PREDICT statement GS.  */

static inline enum br_predictor
gimple_predict_predictor (gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_PREDICT);
  return (enum br_predictor) (gs->subcode & ~GF_PREDICT_TAKEN);
}


/* Set the predictor of GIMPLE_PREDICT statement GS to PREDICT.  */

static inline void
gimple_predict_set_predictor (gimple gs, enum br_predictor predictor)
{
  GIMPLE_CHECK (gs, GIMPLE_PREDICT);
  gs->subcode = (gs->subcode & GF_PREDICT_TAKEN)
		       | (unsigned) predictor;
}


/* Return the outcome of GIMPLE_PREDICT statement GS.  */

static inline enum prediction
gimple_predict_outcome (gimple gs)
{
  GIMPLE_CHECK (gs, GIMPLE_PREDICT);
  return (gs->subcode & GF_PREDICT_TAKEN) ? TAKEN : NOT_TAKEN;
}


/* Set the outcome of GIMPLE_PREDICT statement GS to OUTCOME.  */

static inline void
gimple_predict_set_outcome (gimple gs, enum prediction outcome)
{
  GIMPLE_CHECK (gs, GIMPLE_PREDICT);
  if (outcome == TAKEN)
    gs->subcode |= GF_PREDICT_TAKEN;
  else
    gs->subcode &= ~GF_PREDICT_TAKEN;
}


/* Return the type of the main expression computed by STMT.  Return
   void_type_node if the statement computes nothing.  */

static inline tree
gimple_expr_type (const_gimple stmt)
{
  enum gimple_code code = gimple_code (stmt);

  if (code == GIMPLE_ASSIGN || code == GIMPLE_CALL)
    {
      tree type;
      /* In general we want to pass out a type that can be substituted
         for both the RHS and the LHS types if there is a possibly
	 useless conversion involved.  That means returning the
	 original RHS type as far as we can reconstruct it.  */
      if (code == GIMPLE_CALL)
	{
	  if (gimple_call_internal_p (stmt)
	      && gimple_call_internal_fn (stmt) == IFN_MASK_STORE)
	    type = TREE_TYPE (gimple_call_arg (stmt, 3));
	  else
	    type = gimple_call_return_type (stmt);
	}
      else
	switch (gimple_assign_rhs_code (stmt))
	  {
	  case POINTER_PLUS_EXPR:
	    type = TREE_TYPE (gimple_assign_rhs1 (stmt));
	    break;

	  default:
	    /* As fallback use the type of the LHS.  */
	    type = TREE_TYPE (gimple_get_lhs (stmt));
	    break;
	  }
      return type;
    }
  else if (code == GIMPLE_COND)
    return boolean_type_node;
  else
    return void_type_node;
}

/* Enum and arrays used for allocation stats.  Keep in sync with
   gimple.c:gimple_alloc_kind_names.  */
enum gimple_alloc_kind
{
  gimple_alloc_kind_assign,	/* Assignments.  */
  gimple_alloc_kind_phi,	/* PHI nodes.  */
  gimple_alloc_kind_cond,	/* Conditionals.  */
  gimple_alloc_kind_rest,	/* Everything else.  */
  gimple_alloc_kind_all
};

extern int gimple_alloc_counts[];
extern int gimple_alloc_sizes[];

/* Return the allocation kind for a given stmt CODE.  */
static inline enum gimple_alloc_kind
gimple_alloc_kind (enum gimple_code code)
{
  switch (code)
    {
      case GIMPLE_ASSIGN:
	return gimple_alloc_kind_assign;
      case GIMPLE_PHI:
	return gimple_alloc_kind_phi;
      case GIMPLE_COND:
	return gimple_alloc_kind_cond;
      default:
	return gimple_alloc_kind_rest;
    }
}

/* Return true if a location should not be emitted for this statement
   by annotate_all_with_location.  */

static inline bool
gimple_do_not_emit_location_p (gimple g)
{
  return gimple_plf (g, GF_PLF_1);
}

/* Mark statement G so a location will not be emitted by
   annotate_one_with_location.  */

static inline void
gimple_set_do_not_emit_location (gimple g)
{
  /* The PLF flags are initialized to 0 when a new tuple is created,
     so no need to initialize it anywhere.  */
  gimple_set_plf (g, GF_PLF_1, true);
}


/* Macros for showing usage statistics.  */
#define SCALE(x) ((unsigned long) ((x) < 1024*10	\
		  ? (x)					\
		  : ((x) < 1024*1024*10			\
		     ? (x) / 1024			\
		     : (x) / (1024*1024))))

#define LABEL(x) ((x) < 1024*10 ? 'b' : ((x) < 1024*1024*10 ? 'k' : 'M'))

#endif  /* GCC_GIMPLE_H */
