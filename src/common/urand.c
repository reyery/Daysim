#ifndef lint
static const char	RCSid[] = "$Id: urand.c,v 2.11 2015/05/27 08:42:06 greg Exp $";
#endif
/*
 * Anticorrelated random function due to Christophe Schlick
 */

#include "copyright.h"

#include  "standard.h"
#include  "random.h"


/* 48 bit linear congruential generator, identical to POSIX drand48()/lrand48()/srand48() */
static unsigned long long	rad_rand48_state = 0x1234ABCD330EULL;	/* POSIX default state before srand48() */

static void
rad_rand48_step(void)
{
	rad_rand48_state = (rad_rand48_state * 0x5DEECE66DULL + 0xBULL) & 0xFFFFFFFFFFFFULL;
}

void
rad_srand48(long seed)
{
	rad_rand48_state = (((unsigned long long)seed & 0xFFFFFFFFULL) << 16) | 0x330EULL;
}

long
rad_lrand48(void)
{
	rad_rand48_step();
	return (long)(rad_rand48_state >> 17);
}

double
rad_drand48(void)
{
	rad_rand48_step();
	return (double)rad_rand48_state / 281474976710656.0;	/* 2^48 */
}

#undef initurand

#define  MAXORDER	(8*sizeof(unsigned short))

static unsigned short  empty_tab = 0;

unsigned short  *urperm = &empty_tab;	/* urand() permutation */
int  urmask = 0;			/* bits used in permutation */


int
initurand(			/* initialize urand() for size entries */
	int  size
)
{
	int  order, n;
	int  i, offset;

	if ((urperm != NULL) & (urperm != &empty_tab))
		free((void *)urperm);
	if (--size <= 0) {
		empty_tab = 0;
		urperm = &empty_tab;
		urmask = 0;
		return(0);
	}
	for (i = 1; (size >>= 1); i++)
		if (i == MAXORDER)
			break;
	order = i;
	urmask = (1<<order) - 1;
	urperm = (unsigned short *)malloc((urmask+1)*sizeof(unsigned short));
	if (urperm == NULL) {
		eputs("out of memory in initurand\n");
		quit(1);
	}
	urperm[0] = 0;
	for (n = 1, offset = 1; n <= order; n++, offset <<= 1)
		for (i = offset; i--; ) {
			urperm[i+offset] = urperm[i] <<= 1;
			if (random() & 0x4000)
				urperm[i]++;
			else
				urperm[i+offset]++;
		}
	return(urmask+1);
}


int
ilhash(				/* hash a set of integer values */
	int  *d,
	int  n
)
{
	static int  tab[8] = {103699,96289,73771,65203,81119,87037,92051,98899};
	int  hval;

	hval = 0;
	while (n-- > 0)
		hval ^= *d++ * tab[n&7];
	return(hval & 0x7fffffff);
}
