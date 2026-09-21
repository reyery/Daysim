/* RCSid $Id: random.h,v 2.23 2016/03/06 01:13:17 schorsch Exp $ */
/*
 *  random.h - header file for random(3) and urand() function.
 */
#ifndef _RAD_RANDOM_H_
#define _RAD_RANDOM_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(RAD_PORTABLE_RANDOM)
/*
 * The Windows C runtime has no drand48() family and its rand() is a different, 15 bit generator, so the random
 * sample streams (and therefore the results) differed from Linux and macOS. Use the same 48 bit linear
 * congruential generator as POSIX drand48()/lrand48()/srand48(), so all platforms draw the same numbers.
 * RAD_PORTABLE_RANDOM forces this on other platforms, to check the two implementations agree.
 */
extern void	rad_srand48(long seed);
extern long	rad_lrand48(void);
extern double	rad_drand48(void);
#define	 random()	rad_lrand48()
#define  srandom(s)	rad_srand48((long)(s))
#define	 frandom()	rad_drand48()
#else
#define	 random()	lrand48()
#define  srandom(s)	srand48((long)(s))
#define	 frandom()	drand48()
#endif
extern unsigned short	*urperm;
extern int	urmask;

#define	 urand(i)	(urmask ? (urperm[(i)&urmask]+frandom())/(urmask+1.) \
				: frandom())

extern int	initurand(int size);

				/* defined in urand.c */
extern int	ilhash(int *d, int n);
				/* defined in urind.c */
extern int	urind(int s, int i);
				/* defined in multisamp.c */
extern void	multisamp(double t[], int n, double r);


#ifdef __cplusplus
}
#endif
#endif /* _RAD_RANDOM_H_ */

