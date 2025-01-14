//------------------------------------------------------------------------------
// UMFPACK/Source/umfpack_triplet_to_col: convert triplets to CSC sparse matrix
//------------------------------------------------------------------------------

// UMFPACK, Copyright (c) 2005-2022, Timothy A. Davis, All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0+

//------------------------------------------------------------------------------

/*
    User callable.  Converts triplet input to column-oriented form.  Duplicate
    entries may exist (they are summed in the output).  The columns of the
    column-oriented form are in sorted order.  The input is not modified.
    Returns 1 if OK, 0 if an error occurred.  See umfpack.h for
    details.

    If Map is present (a non-NULL pointer to an Int array of size nz), then on
    output it holds the position of the triplets in the column-form matrix.
    That is, suppose p = Map [k], and the k-th triplet is i=Ti[k], j=Tj[k], and
    aij=Tx[k].  Then i=Ai[p], and aij will have been summed into Ax[p].  Also,
    Ap[j] <= p < Ap[j+1].  The Map array is not computed if it is (Int *) NULL.

    Dynamic memory usage:

	If numerical values are present, then one (two for complex version)
	workspace of size (nz+1)*sizeof(double) is allocated via UMF_malloc.
	Next, 4 calls to UMF_malloc are made to obtain workspace of size
	((nz+1) + (n_row+1) + n_row + MAX (n_row,n_col)) * sizeof(Int).  All of
	this workspace (4 to 6 objects) are free'd via UMF_free on return.

	For the complex version, additional space is allocated.

	An extra array of size nz*sizeof(Int) is allocated if Map is present.
*/

#include "umf_internal.h"
#include "umf_malloc.h"
#include "umf_free.h"
#include "umf_triplet.h"

#ifndef NDEBUG
PRIVATE Int init_count ;
#endif

/* ========================================================================== */

GLOBAL int UMFPACK_triplet_to_col
(
    Int n_row,
    Int n_col,
    Int nz,
    const Int Ti [ ],		/* size nz */
    const Int Tj [ ],		/* size nz */
    const double Tx [ ],	/* size nz */
#ifdef COMPLEX
    const double Tz [ ],	/* size nz */
#endif
    Int Ap [ ],			/* size n_col + 1 */
    Int Ai [ ],			/* size nz */
    double Ax [ ]		/* size nz */
#ifdef COMPLEX
    , double Az [ ]		/* size nz */
#endif
    , Int Map [ ]		/* size nz */
)
{

    /* ---------------------------------------------------------------------- */
    /* local variables */
    /* ---------------------------------------------------------------------- */

    Int *RowCount, *Rp, *Rj, *W, nn, do_values, do_map, *Map2 ;
    int status ;
    double *Rx ;
#ifdef COMPLEX
    double *Rz ;
    Int split ;
#endif

#ifndef NDEBUG
    UMF_dump_start ( ) ;
    init_count = UMF_malloc_count ;
#endif

    /* ---------------------------------------------------------------------- */
    /* check inputs */
    /* ---------------------------------------------------------------------- */

    if (!Ai || !Ap || !Ti || !Tj)
    {
	return (UMFPACK_ERROR_argument_missing) ;
    }

    if (n_row <= 0 || n_col <= 0)		/* must be > 0 */
    {
	return (UMFPACK_ERROR_n_nonpositive) ;
    }

    if (nz < 0)		/* nz must be >= 0 (singular matrices are OK) */
    {
	return (UMFPACK_ERROR_invalid_matrix) ;
    }

    nn = MAX (n_row, n_col) ;

    /* ---------------------------------------------------------------------- */
    /* allocate workspace */
    /* ---------------------------------------------------------------------- */

    Rx = (double *) NULL ;

    do_values = Ax && Tx ;

    if (do_values)
    {
#ifdef COMPLEX
	Rx = (double *) UMF_malloc (2*nz+2, sizeof (double)) ;
	split = SPLIT (Tz) && SPLIT (Az) ;
	if (split)
	{
	    Rz = Rx + nz ;
	}
	else
	{
	    Rz = (double *) NULL ;
	}
#else
	Rx = (double *) UMF_malloc (nz+1, sizeof (double)) ;
#endif
	if (!Rx)
	{
	    DEBUGm4 (("out of memory: triplet work \n")) ;
	    ASSERT (UMF_malloc_count == init_count) ;
	    return (UMFPACK_ERROR_out_of_memory) ;
	}
    }

    do_map = (Map != (Int *) NULL) ;
    Map2 = (Int *) NULL ;
    if (do_map)
    {
	DEBUG0 (("Do map:\n")) ;
	Map2 = (Int *) UMF_malloc (nz+1, sizeof (Int)) ;
	if (!Map2)
	{
	    DEBUGm4 (("out of memory: triplet map\n")) ;
	    (void) UMF_free ((void *) Rx) ;
	    ASSERT (UMF_malloc_count == init_count) ;
	    return (UMFPACK_ERROR_out_of_memory) ;
	}
    }

    Rj = (Int *) UMF_malloc (nz+1, sizeof (Int)) ;
    Rp = (Int *) UMF_malloc (n_row+1, sizeof (Int)) ;
    RowCount = (Int *) UMF_malloc (n_row, sizeof (Int)) ;
    W = (Int *) UMF_malloc (nn, sizeof (Int)) ;
    if (!Rj || !Rp || !RowCount || !W)
    {
	DEBUGm4 (("out of memory: triplet work (int)\n")) ;
	(void) UMF_free ((void *) Rx) ;
	(void) UMF_free ((void *) Map2) ;
	(void) UMF_free ((void *) Rp) ;
	(void) UMF_free ((void *) Rj) ;
	(void) UMF_free ((void *) RowCount) ;
	(void) UMF_free ((void *) W) ;
	ASSERT (UMF_malloc_count == init_count) ;
	return (UMFPACK_ERROR_out_of_memory) ;
    }

    ASSERT (UMF_malloc_count == init_count + 4 +
	(Rx != (double *) NULL) + do_map) ;

    /* ---------------------------------------------------------------------- */
    /* convert from triplet to column form */
    /* ---------------------------------------------------------------------- */

    if (do_map)
    {
	if (do_values)
	{
	    status = UMF_triplet_map_x (n_row, n_col, nz, Ti, Tj, Ap, Ai, Rp,
		Rj, W, RowCount, Tx, Ax, Rx
#ifdef COMPLEX
		, Tz, Az, Rz
#endif
		, Map, Map2) ;
	}
	else
	{
	    status = UMF_triplet_map_nox (n_row, n_col, nz, Ti, Tj, Ap, Ai, Rp,
		Rj, W, RowCount, Map, Map2) ;
	}
    }
    else
    {
	if (do_values)
	{
	    status = UMF_triplet_nomap_x (n_row, n_col, nz, Ti, Tj, Ap, Ai, Rp,
		Rj, W, RowCount , Tx, Ax, Rx
#ifdef COMPLEX
		, Tz, Az, Rz
#endif
		) ;
	}
	else
	{
	    status = UMF_triplet_nomap_nox (n_row, n_col, nz, Ti, Tj, Ap, Ai,
		Rp, Rj, W, RowCount) ;
	}
    }

    /* ---------------------------------------------------------------------- */
    /* free the workspace */
    /* ---------------------------------------------------------------------- */

    (void) UMF_free ((void *) Rx) ;
    (void) UMF_free ((void *) Map2) ;
    (void) UMF_free ((void *) Rp) ;
    (void) UMF_free ((void *) Rj) ;
    (void) UMF_free ((void *) RowCount) ;
    (void) UMF_free ((void *) W) ;
    ASSERT (UMF_malloc_count == init_count) ;

    return (status) ;
}
