/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2025-02-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

#define SWAP(a, b, size)\
do{\
BIG _size = (size);\
char *_a = (a);\
char *_b = (b);\
do{\
char _tmp = *_a;\
*_a++ = *_b;\
*_b++ = _tmp;\
} while (--_size > 0);\
} while (0)

#define MAX_THRESH 4
#define PUSH(low, high) ((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#define POP(low, high) ((void) (--top, (low = top->lo), (high = top->hi)))
#define MINN(a,b) (((a) < (b)) ? (a) : (b))

typedef struct
{
	char* lo;
	char* hi;
} stack_node;

void _Os_quicksort(void* const pbase, BIG total_elems, BIG size, int (cmp)(const void* context, const void* a, const void* b), void* arg)
{
	if (total_elems == 0)
		return;

	char* base_ptr = (char*)pbase;
	const BIG max_thresh = MAX_THRESH * size;

	if (total_elems > MAX_THRESH)
	{
		char* lo = base_ptr;
		char* hi = &lo[size * (total_elems - 1)];
		stack_node stack[CHAR_BIT * sizeof(BIG)];
		stack_node* top = stack;
		PUSH(NULL, NULL);

		while (stack < top && StdProgress_is())
		{
			char* left_ptr;
			char* right_ptr;
			char* mid = lo + size * ((hi - lo) / size >> 1);
			if ((*cmp) (arg, (void*)mid, (void*)lo) < 0)	SWAP(mid, lo, size);
			if ((*cmp) (arg, (void*)hi, (void*)mid) < 0)	SWAP(mid, hi, size);
			else
				goto jump_over;
			if ((*cmp) (arg, (void*)mid, (void*)lo) < 0)	SWAP(mid, lo, size);

		jump_over:;
			left_ptr = lo + size;
			right_ptr = hi - size;
			do
			{
				while ((*cmp) (arg, (void*)left_ptr, (void*)mid) < 0)	left_ptr += size;
				while ((*cmp) (arg, (void*)mid, (void*)right_ptr) < 0)	right_ptr -= size;
				if (left_ptr < right_ptr)
				{
					SWAP(left_ptr, right_ptr, size);
					if (mid == left_ptr)
						mid = right_ptr;
					else if (mid == right_ptr)
						mid = left_ptr;
					left_ptr += size;
					right_ptr -= size;
				}
				else if (left_ptr == right_ptr)
				{
					left_ptr += size;
					right_ptr -= size;
					break;
				}
			} while (left_ptr <= right_ptr);

			if ((BIG)(right_ptr - lo) <= max_thresh)
			{
				if ((BIG)(hi - left_ptr) <= max_thresh)
					POP(lo, hi);
				else
					lo = left_ptr;
			}
			else if ((BIG)(hi - left_ptr) <= max_thresh)
				hi = right_ptr;
			else if ((right_ptr - lo) > (hi - left_ptr))
			{
				PUSH(lo, right_ptr);
				lo = left_ptr;
			}
			else
			{
				PUSH(left_ptr, hi);
				hi = right_ptr;
			}

			StdProgress_setEx("SHORTING", (((UBIG)stack) - ((UBIG)top)), CHAR_BIT * sizeof(BIG));
		}
	}

	{
		char* const end_ptr = &base_ptr[size * (total_elems - 1)];
		char* tmp_ptr = base_ptr;
		char* thresh = MINN(end_ptr, base_ptr + max_thresh);
		char* run_ptr;

		for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
			if ((*cmp) (arg, (void*)run_ptr, (void*)tmp_ptr) < 0)
				tmp_ptr = run_ptr;
		if (tmp_ptr != base_ptr)
			SWAP(tmp_ptr, base_ptr, size);

		run_ptr = base_ptr + size;
		while ((run_ptr += size) <= end_ptr && StdProgress_is())
		{
			tmp_ptr = run_ptr - size;
			while ((*cmp) (arg, (void*)run_ptr, (void*)tmp_ptr) < 0)
				tmp_ptr -= size;
			tmp_ptr += size;
			if (tmp_ptr != run_ptr)
			{
				char* trav;
				trav = run_ptr + size;
				while (--trav >= run_ptr)
				{
					char c = *trav;
					char* hi, * lo;
					for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo)
						*hi = *lo;
					*hi = c;
				}
			}
		}
	}
}