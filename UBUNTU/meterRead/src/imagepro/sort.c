#include "licomdef.h"
#include "limem.h"
#include "lidebug.h"
/* Byte-wise swap two items of size SIZE. */
#define SWAP(a, b, lSize)						      \
  do									      \
    {									      \
      MLong __size = (lSize);					      \
      MChar *__a = (a), *__b = (b);				      \
      do								      \
	{								      \
	  MChar __tmp = *__a;						      \
	  *__a++ = *__b;						      \
	  *__b++ = __tmp;						      \
	} while (--__size > 0);						      \
    } while (0)

/* Discontinue quicksort algorithm when partition gets below this size.
   This particular magic number was chosen to work best on a Sun 4/260. */
#define MAX_THRESH 4

/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct
  {
    MChar *lo;
    MChar *hi;
  } stack_node;

/* The next 4 #defines implement a very fast in-line stack abstraction. */
#define STACK_SIZE	(8 * sizeof(MLong))
#define PUSH(low, high)	((MVoid) ((top->lo = (low)), (top->hi = (high)), ++top))
#define	POP(low, high)	((MVoid) (--top, (low = top->lo), (high = top->hi)))
#define	STACK_NOT_EMPTY	(stack < top)


/* Order size using quicksort.  This implementation incorporates
   four optimizations discussed in Sedgewick:

   1. Non-recursive, using an explicit stack of pointer that store the
      next array partition to sort.  To save time, this maximum amount
      of space required to store an array of MAX_INT is allocated on the
      stack.  Assuming a 32-bit integer, this needs only 32 *
      sizeof(stack_node) == 136 bits.  Pretty cheap, actually.

   2. Chose the pivot element using a median-of-three decision tree.
      This reduces the probability of selecting a bad pivot value and
      eliminates certain extraneous comparisons.

   3. Only quicksorts TOTAL_ELEMS / MAX_THRESH partitions, leaving
      insertion sort to order the MAX_THRESH items within each partition.
      This is a big win, since insertion sort is faster for small, mostly
      sorted array segments.

   4. The larger of the two sub-partitions is always pushed onto the
      stack first, with the algorithm then concentrating on the
      smaller partition.  This *guarantees* no more than log (n)
      stack size is needed (actually O(1) in this case)!  */

//void
//DEFUN(_quicksort, (pbase, total_elems, size, cmp),
//      PTR CONST pbase AND size_t total_elems AND size_t size AND
//      int EXFUN((*cmp), (CONST PTR, CONST PTR)))
MRESULT QuickSort(MHandle hMemMgr,MVoid *pbase, MLong lTotal_elems, MLong lSize, MLong (*cmp)(const MVoid*, const MVoid*))
{
  MRESULT res = LI_ERR_NONE;
  MChar *base_ptr = (MChar *) pbase;

  /* Allocating SIZE bytes for a pivot buffer facilitates a better
     algorithm below since we can do comparisons directly on the pivot. */
  //MChar *pivot_buffer = (MChar *) __alloca (size);
  MChar *pivot_buffer = MNull;
  MDWord max_thresh = MAX_THRESH *lSize;
  AllocVectMem(hMemMgr, pivot_buffer, lSize, MChar);

  if (lTotal_elems == 0)
    /* Avoid lossage with unsigned arithmetic below.  */
    goto EXT;

  if (lTotal_elems > MAX_THRESH)
    {
      MChar *lo = base_ptr;
      MChar *hi = &lo[lSize* (lTotal_elems- 1)];
      /* Largest size needed for 32-bit int!!! */
      stack_node stack[STACK_SIZE];
      stack_node *top = stack + 1;

      while (STACK_NOT_EMPTY)
      {
          MChar *left_ptr;
          MChar *right_ptr;

	     MChar *pivot = pivot_buffer;

	  /* Select median value from among LO, MID, and HI. Rearrange
	     LO and HI so the three values are sorted. This lowers the
	     probability of picking a pathological pivot value and
	     skips a comparison for both the LEFT_PTR and RIGHT_PTR. */

	  MChar *mid = lo + lSize * ((hi - lo) / lSize >> 1);

	  if ((*cmp)((MVoid*) mid, (MVoid*) lo) < 0)
	    SWAP(mid, lo, lSize);
	  if ((*cmp)((MVoid*) hi, (MVoid*) mid) < 0)
	    SWAP(mid, hi, lSize);
	  else
	    goto jump_over;
	  if ((*cmp)((MVoid*) mid, (MVoid*) lo) < 0)
	    SWAP(mid, lo, lSize);
	jump_over:;
	  JMemCpy(pivot, mid, lSize);
	  pivot = pivot_buffer;

	  left_ptr  = lo + lSize;
	  right_ptr = hi - lSize;

	  /* Here's the famous ``collapse the walls'' section of quicksort.
	     Gotta like those tight inner loops!  They are the main reason
	     that this algorithm runs much faster than others. */
	  do
	    {
	      while ((*cmp)((MVoid*) left_ptr, (MVoid*) pivot) < 0)
		left_ptr += lSize;

	      while ((*cmp)((MVoid*) pivot, (MVoid*) right_ptr) < 0)
		right_ptr -= lSize;

	      if (left_ptr < right_ptr)
		{
		  SWAP(left_ptr, right_ptr, lSize);
		  left_ptr += lSize;
		  right_ptr -= lSize;
		}
	      else if (left_ptr == right_ptr)
		{
		  left_ptr += lSize;
		  right_ptr -= lSize;
		  break;
		}
	    }
	  while (left_ptr <= right_ptr);

          /* Set up pointers for next iteration.  First determine whether
             left and right partitions are below the threshold size.  If so,
             ignore one or both.  Otherwise, push the larger partition's
             bounds on the stack and continue sorting the smaller one. */

          if ((MDWord) (right_ptr - lo) <= max_thresh)
            {
              if ((MDWord) (hi - left_ptr) <= max_thresh)
		/* Ignore both small partitions. */
                POP(lo, hi);
              else
		/* Ignore small left partition. */
                lo = left_ptr;
            }
          else if ((MDWord) (hi - left_ptr) <= max_thresh)
	    /* Ignore small right partition. */
            hi = right_ptr;
          else if ((right_ptr - lo) > (hi - left_ptr))
            {
	      /* Push larger left partition indices. */
              PUSH(lo, right_ptr);
              lo = left_ptr;
            }
          else
            {
	      /* Push larger right partition indices. */
              PUSH(left_ptr, hi);
              hi = right_ptr;
            }
        }
    }

  /* Once the BASE_PTR array is partially sorted by quicksort the rest
     is completely sorted using insertion sort, since this is efficient
     for partitions below MAX_THRESH size. BASE_PTR points to the beginning
     of the array to sort, and END_PTR points at the very last element in
     the array (*not* one beyond it!). */
#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

  {
    MChar *end_ptr = &base_ptr[lSize * (lTotal_elems - 1)];
    MChar *tmp_ptr = base_ptr;
    MChar *thresh = min(end_ptr, base_ptr + max_thresh);
    MChar *run_ptr;

    /* Find smallest element in first threshold and place it at the
       array's beginning.  This is the smallest array element,
       and the operation speeds up insertion sort's inner loop. */

    for (run_ptr = tmp_ptr + lSize; run_ptr <= thresh; run_ptr += lSize)
      if ((*cmp)((MVoid*) run_ptr, (MVoid*) tmp_ptr) < 0)
        tmp_ptr = run_ptr;

    if (tmp_ptr != base_ptr)
      SWAP(tmp_ptr, base_ptr, lSize);

    /* Insertion sort, running from left-hand-side up to right-hand-side.  */

    run_ptr = base_ptr + lSize;
    while ((run_ptr += lSize) <= end_ptr)
      {
	tmp_ptr = run_ptr - lSize;
	while ((*cmp)((MVoid*) run_ptr, (MVoid*) tmp_ptr) < 0)
	  tmp_ptr -= lSize;

	tmp_ptr += lSize;
        if (tmp_ptr != run_ptr)
          {
            MChar *trav;

	    trav = run_ptr + lSize;
	    while (--trav >= run_ptr)
              {
                MChar c = *trav;
                MChar *hi, *lo;

                for (hi = lo = trav; (lo -= lSize) >= tmp_ptr; hi = lo)
                  *hi = *lo;
                *hi = c;
              }
          }
      }
  }
EXT:
  FreeVectMem(hMemMgr, pivot_buffer);
  return res;
}
