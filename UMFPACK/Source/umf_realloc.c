//------------------------------------------------------------------------------
// UMFPACK/Source/umf_realloc: reallocate a block of memory
//------------------------------------------------------------------------------

// UMFPACK, Copyright (c) 2005-2022, Timothy A. Davis, All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0+

//------------------------------------------------------------------------------

/*
    Realloc a block previously allocated by UMF_malloc.
    Return NULL on failure (in which case the block is still allocated, and will
    be kept at is present size).  This routine is only used for Numeric->Memory.
*/

#include "umf_internal.h"
#include "umf_realloc.h"

#if defined (UMF_MALLOC_COUNT) || !defined (NDEBUG)
#include "umf_malloc.h"
#endif

GLOBAL void *UMF_realloc
(
    void *p,
    Int n_objects,
    size_t size_of_object
)
{
    size_t size ;
    void *p2 ;

#ifdef UMF_TCOV_TEST
    /* For exhaustive statement coverage testing only! */
    /* Pretend to fail, to test out-of-memory conditions. */
    umf_realloc_fail-- ;
    if (umf_realloc_fail <= umf_realloc_hi &&
	umf_realloc_fail >= umf_realloc_lo)
    {
	return ((void *) NULL) ;
    }
#endif

    /* make sure that we allocate something */
    n_objects = MAX (1, n_objects) ;

    size = (size_t) n_objects ;
    ASSERT (size_of_object > 1) ;
    if (size > Int_MAX / size_of_object)
    {
	/* :: int overflow in umf_realloc :: */
	return ((void *) NULL) ;
    }
    size *= size_of_object ;

    DEBUG0 (("UMF_realloc: "ID" n_objects "ID"  size_of_object "ID"\n",
	(Int) p, n_objects, (Int) size_of_object)) ;

    p2 = SuiteSparse_config.realloc_func (p, size) ;

#if defined (UMF_MALLOC_COUNT) || !defined (NDEBUG)
    /* If p didn't exist on input, and p2 exists, then a new object has been
     * allocated. */
    if (p == (void *) NULL && p2 != (void *) NULL)
    {
	UMF_malloc_count++ ;
    }
#endif

    DEBUG0 (("UMF_realloc: "ID" new malloc count "ID"\n",
	(Int) p2, UMF_malloc_count)) ;

    return (p2) ;
}
