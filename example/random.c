#include <time.h>
#include <math.h>

#include "random.h"

static int iset_nrand0 = 0;
static int iset_nrand = 0;

double
nrand0 (void)
/* normal distribution random number */
{
  static double gset;
  double fac, rsq, v1, v2;

  if (iset_nrand0 == 0)
    {
      do
	{
	  v1 = 2.0 * frand () - 1.0;
	  v2 = 2.0 * frand () - 1.0;
	  rsq = v1 * v1 + v2 * v2;
	}
      while (rsq >= 1.0 || rsq < 1e-8);
      fac = sqrt (-2.0 * log (rsq) / rsq);
      gset = v1 * fac;
      iset_nrand0 = 1;
      return (v2 * fac);
    }
  else
    {
      iset_nrand0 = 0;
      return gset;
    }
}

double
nrand (double mean, double sigma)
/* normal distribution random number */
{
  static double gset;
  double fac, rsq, v1, v2;

  if (iset_nrand == 0)
    {
      do
	{
	  v1 = 2.0 * frand () - 1.0;
	  v2 = 2.0 * frand () - 1.0;
	  rsq = v1 * v1 + v2 * v2;
	}
      while (rsq >= 1.0 || rsq < 1e-8);
      fac = sqrt (-2.0 * log (rsq) / rsq);
      gset = v1 * fac;
      iset_nrand = 1;
      return (v2 * fac) * sigma + mean;
    }
  else
    {
      iset_nrand = 0;
      return gset * sigma + mean;
    }
}

int
irand (int max)
/* integer random number 0...(max-1) */
{
  int result;

  while ((result = (int) (frand () * (double) max)) >= max);
  return result;
}

/* A C-program for MT19937: Real number version([0,1)-interval) */
/* (1999/10/28)                                                 */
/*   genrand() generates one pseudorandom real number (double)  */
/* which is uniformly distributed on [0,1)-interval, for each   */
/* call. sgenrand(seed) sets initial values to the working area */
/* of 624 words. Before genrand(), sgenrand(seed) must be       */
/* called once. (seed is any 32-bit integer.)                   */
/* Integer generator is obtained by modifying two lines.        */
/*   Coded by Takuji Nishimura, considering the suggestions by  */
/* Topher Cooper and Marc Rieffel in July-Aug. 1997.            */

/* This library is free software; you can redistribute it and/or   */
/* modify it under the terms of the GNU Library General Public     */
/* License as published by the Free Software Foundation; either    */
/* version 2 of the License, or (at your option) any later         */
/* version.                                                        */
/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            */
/* See the GNU Library General Public License for more details.    */
/* You should have received a copy of the GNU Library General      */
/* Public License along with this library; if not, write to the    */
/* Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA   */
/* 02111-1307  USA                                                 */

/* Copyright (C) 1997, 1999 Makoto Matsumoto and Takuji Nishimura. */
/* When you use this, send an email to: matumoto@math.keio.ac.jp   */
/* with an appropriate reference to your work.                     */

/* REFERENCE                                                       */
/* M. Matsumoto and T. Nishimura,                                  */
/* "Mersenne Twister: A 623-Dimensionally Equidistributed Uniform  */
/* Pseudo-Random Number Generator",                                */
/* ACM Transactions on Modeling and Computer Simulation,           */
/* Vol. 8, No. 1, January 1998, pp 3--30.                          */

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df	/* constant vector a */
#define UPPER_MASK 0x80000000	/* most significant w-r bits */
#define LOWER_MASK 0x7fffffff	/* least significant r bits */

/* Tempering parameters */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

static unsigned long mt[N];	/* the array for the state vector  */
static int mti = N + 1;		/* mti==N+1 means mt[N] is not initialized */

/* Initializing the array with a seed */
void
frandseed (long iseed)
{
  int i;
  unsigned long seed;

  iset_nrand0 = 0;
  iset_nrand = 0;
  if (iseed == 0)
    iseed = (unsigned) ((long) time (NULL));
  seed = (unsigned) iseed;


  for (i = 0; i < N; i++)
    {
      mt[i] = seed & 0xffff0000;
      seed = 69069 * seed + 1;
      mt[i] |= (seed & 0xffff0000) >> 16;
      seed = 69069 * seed + 1;
    }
  mti = N;
}

/* Initialization by "sgenrand()" is an example. Theoretically,      */
/* there are 2^19937-1 possible states as an intial state.           */
/* This function allows to choose any of 2^19937-1 ones.             */
/* Essential bits in "seed_array[]" is following 19937 bits:         */
/*  (seed_array[0]&UPPER_MASK), seed_array[1], ..., seed_array[N-1]. */
/* (seed_array[0]&LOWER_MASK) is discarded.                          */
/* Theoretically,                                                    */
/*  (seed_array[0]&UPPER_MASK), seed_array[1], ..., seed_array[N-1]  */
/* can take any values except all zeros.                             */

void
lsgenrand (long unsigned int *seed_array)
    /* the length of seed_array[] must be at least N */
{
  int i;

  for (i = 0; i < N; i++)
    mt[i] = seed_array[i];
  mti = N;
}

double				/* generating reals */
									    /* unsigned long *//* for integer generation */
frand (void)
{
  unsigned long y;
  static unsigned long mag01[2] = { 0x0, MATRIX_A };
  volatile int kk;
  double res;
  /* mag01[x] = x * MATRIX_A  for x=0,1 */

  if (mti >= N)
    {				/* generate N words at one time */

      if (mti == N + 1)		/* if sgenrand() has not been called, */
	frandseed (4357);	/* a default initial seed is used   */

      for (kk = 0; kk < N - M; kk++)
	{
/*      printf("kk=%d kk+M=%d\n",kk,kk+M); */
	  y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
	  mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1];
	}
      for (; kk < N - 1; kk++)
	{
	  y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
	  mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1];
	}
      y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
      mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1];

      mti = 0;
    }

  y = mt[mti++];
  y ^= TEMPERING_SHIFT_U (y);
  y ^= TEMPERING_SHIFT_S (y) & TEMPERING_MASK_B;
  y ^= TEMPERING_SHIFT_T (y) & TEMPERING_MASK_C;
  y ^= TEMPERING_SHIFT_L (y);
  res = (double) y *2.3283064365386963e-10;
/* printf("=== rnd=%.20f\n",res); */
  return (res);			/* reals: [0,1)-interval */
  /* return y; *//* for integer generation */
}

#define DFLEN 64
static double df[DFLEN] = {
  -1.8370939043417808943e-06,
  -6.1196945589982073985e-06,
  -7.5587701579763848592e-07,
  2.5562408350808373654e-05,
  4.0758326159745981627e-05,
  -2.5131872908960921123e-05,
  -0.00014876214901936348016,
  -0.00011893811742986104748,
  0.00021896415862725504128,
  0.00052444196762459233553,
  0.00011727581338660759027,
  -0.00095051536688676254964,
  -0.0012527267103039769756,
  0.00048357364652680834428,
  0.0027983445801144007873,
  0.0019952329898457937768,
  -0.0028299913322717383303,
  -0.0061757318063923029715,
  -0.001405108411686069795,
  0.0086145480140459759784,
  0.010554659468493140902,
  -0.0033774389783623680766,
  -0.019616438026572809159,
  -0.013502662547377131163,
  0.017384385505533751742,
  0.037703687212232158688,
  0.0091398768675829813617,
  -0.05266831278756242539,
  -0.07033376546222222311,
  0.024396819308122194912,
  0.2055912251957120361,
  0.35282470939550079266,
  0.35282470939550079266,
  0.2055912251957120361,
  0.024396819308122194912,
  -0.07033376546222222311,
  -0.05266831278756242539,
  0.0091398768675829813617,
  0.037703687212232158688,
  0.017384385505533751742,
  -0.013502662547377131163,
  -0.019616438026572809159,
  -0.0033774389783623680766,
  0.010554659468493140902,
  0.0086145480140459759784,
  -0.001405108411686069795,
  -0.0061757318063923029715,
  -0.0028299913322717383303,
  0.0019952329898457937768,
  0.0027983445801144007873,
  0.00048357364652680834428,
  -0.0012527267103039769756,
  -0.00095051536688676254964,
  0.00011727581338660759027,
  0.00052444196762459233553,
  0.00021896415862725504128,
  -0.00011893811742986104748,
  -0.00014876214901936348016,
  -2.5131872908960921123e-05,
  4.0758326159745981627e-05,
  2.5562408350808373654e-05,
  -7.5587701579763848592e-07,
  -6.1196945589982073985e-06,
  -1.8370939043417808943e-06
};

double
filrand (void)
/*
bandlimited white noise
PSD (fsamp=1Hz) :
1 / sqrt(Hz)  for 0<f<0.15
dropping      for 0.15<f<0.25
<1e-6/sqrt(Hz) for f>0.25
GHH AEI 23.10.2002
*/
{
  static int fill = 0, pos;
  int i;
  double ret;
  static double save[DFLEN];

  if (fill == 0)
    {				/* first call */
      for (i = 0; i < DFLEN; i++)
	save[i] = nrand0 ();
      pos = 0;
      fill = 1;
    }
  else
    {
      save[pos] = nrand0 ();
      pos = (pos + 1) % DFLEN;
    }
  ret = 0.;
  for (i = 0; i < DFLEN; i++)
    {
      ret += save[(i + pos) % DFLEN] * df[i];
    }
  return ret * M_SQRT1_2;
}
