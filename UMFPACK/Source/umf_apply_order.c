//------------------------------------------------------------------------------
// UMFPACK/Source/umf_apply_order: apply postordering of supernodal etree
//------------------------------------------------------------------------------

// UMFPACK, Copyright (c) 2005-2022, Timothy A. Davis, All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0+

//------------------------------------------------------------------------------

/*
    Apply post-ordering of supernodal elimination tree.
*/

#include "umf_internal.h"
#include "umf_apply_order.h"

GLOBAL void UMF_apply_order
(
    Int Front [ ],	    /* of size nn on input, size nfr on output */
    const Int Order [ ],    /* Order [i] = k, i in the range 0..nn-1,
			     * and k in the range 0..nfr-1, means that node
			     * i is the kth node in the postordered tree. */
    Int Temp [ ],	    /* workspace of size nfr */
    Int nn,		    /* nodes are numbered in the range 0..nn-1 */
    Int nfr		    /* the number of nodes actually in use */
)
{
    Int i, k ;
    for (i = 0 ; i < nn ; i++)
    {
	k = Order [i] ;
	ASSERT (k >= EMPTY && k < nfr) ;
	if (k != EMPTY)
	{
	    Temp [k] = Front [i] ;
	}
    }

    for (k = 0 ; k < nfr ; k++)
    {
	Front [k] = Temp [k] ;
    }
}
