#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "netlibi0.h"
#include "genwin.h"

/* genwin.c created by merging sbwin.c and ghhwin.c to generate windows for 
   LPSD and FFT usage, Michael Troebs, 05/2004 */

/* sbwin.c tfwin.c modified to provide smallest useful bin for each window */

/* tfwin.c - ghhwin.c modified for 'tf' usage (make sin-cos-window array) */

/* ghhwin.c - make window functions for Fourier spectral analysis 
Gerhard Heinzel AEI 03/2003 
All rights reserved */

/* DESCRIPTION :

These subroutines compute window functions for Fourier spectral
analysis as described in the document entitled

"Spectrum and spectral density estimation by the Discrete Fourier
transform (DFT), including a comprehensive list of window functions
and some new flat-top windows." by G.Heinzel et al, available from:

 Gerhard Heinzel                              e-mail: ghh@mpq.mpg.de
 Max-Planck-Institut fuer Gravitationsphysik (Albert-Einstein-Inst.)
 Institut Hannover, Callinstrasse 38, D-30167 Hannover, Germany
 Tel +49-(0)511-762-2788,               Fax +49-(0)511-762-2784

The two externally visible functions are:

void set_window (int type, double req_psll, char *name, double *psll, 
    double *rov, double *nenbw, double *w3db, double *flatness);

set_window must be called at least once initially, and every time
a new type of window is requested (but not if only NFFT changes)

input:  type   0...30 = window by number (see list below)
	           -1 = flat-top by PSL
		   -2 = Kaiser by PSLL
    		  
        req_psll: requested PSLL for types -1 and -2, ignored
    			otherwise
output: char *name : Name of window function. space must be reserved
		     by the caller (>= 20 bytes), e.g. by declaring
    		     char name[20];		_NOT_ char *name;
		     For Kaiser windows, the name contains the
		       parameter alpha.
	psll: actual PSLL in dB (differs from requested PSLL for type=-1)
    	rov: recommended overlap in %
    	nenbw: preliminary estimate of NENBW in bins
    	      (precise value will also be computed by makewin)
	w3dB: 3 dB bandwidth in bins
    	flatness: flatness for -0.5 <= f <=0.5 in dB
	    
	    
void makewinsincos (int nfft, double bin, double *win, double *winsum, 
    double *winsum2, double *nenbw);

makewin computes the actual window values and must be called
after set_window. It must be re-called if either a new type
of window has been set with set_win or if NFFT has changed.

input: nfft : length of FFT
       bin : (possibly non-integer) number of bin.
output: win : the actual values of the window. Adequate space
              must be reserved by the caller:
	      size = 2*nfft 
	      order: win[0]*cos, win[0]*sin, win[1]*cos, win[1]*sin, ...
	winsum : sum of all window values. Even for half=1,
	      the sum over all values 0...nfft-1 is returned
	winsum2 : sum of all squared window values. Even for half=1,
	      the sum over all values 0...nfft-1 is returned
	nenbw : precise value of NENBW in bins	    

*/

static double win_Rectangular (double);
static double win_Welch (double);
static double win_Bartlett (double);
static double win_Hanning (double);
static double win_Hamming (double);
static double win_Nuttall3 (double);
static double win_Nuttall4 (double);
static double win_Nuttall3a (double);
static double win_Nuttall3b (double);
static double win_Nuttall4a (double);
static double win_BH92 (double);
static double win_Nuttall4b (double);
static double win_Nuttall4c (double);
static double win_SFT3F (double);
static double win_SFT3M (double);
static double win_FTNI (double);
static double win_SFT4F (double);
static double win_SFT5F (double);
static double win_SFT4M (double);
static double win_FTHP (double);
static double win_HFT70 (double);
static double win_FTSRS (double);
static double win_SFT5M (double);
static double win_HFT90D (double);
static double win_HFT95 (double);
static double win_HFT116D (double);
static double win_HFT144D (double);
static double win_HFT169D (double);
static double win_HFT196D (double);
static double win_HFT223D (double);
static double win_HFT248D (double);
void gerror (const char *err);
double netlibi0 (double x);

struct window_t
{
  char name[20];
  double psll;
  double rov;
  double nenbw;
  double w3db;
  double flatness;
  double sbin;
  int isft;
  double (*winfun) (double);
};
typedef struct window_t window;



/* The following contains a list of all 'numbered' window functions.
Those with 'isft=1' are considered as useful flat-top windows,
one of which is chosen when 'type=-1' in set_window.
They must be ordered by increasing PSLL.
Otherwise the ordering is unimportant.*/

static const window winlist[] = {
/* 0*/ {"Rectangular", 013.3, 00.0, 1.0000, 0.8845, -3.9224, 1.000, 0,
	win_Rectangular},
/* 1*/ {"Welch", 021.3, 29.3, 1.2000, 1.1535, -2.2248, 1.430, 0, win_Welch},
/* 2*/ {"Bartlett", 026.5, 50.0, 1.3333, 1.2736, -1.8242, 2.000, 0, win_Bartlett},
/* 3*/ {"Hanning", 031.5, 50.0, 1.5000, 1.4382, -1.4236, 2.000, 0, win_Hanning},
/* 4*/ {"Hamming", 042.7, 50.0, 1.3628, 1.3008, -1.7514, 2.000, 0, win_Hamming},
/* 5*/ {"Nuttall3", 046.7, 64.7, 1.9444, 1.8496, -0.8630, 3.000, 0, win_Nuttall3},
/* 6*/ {"Nuttall4", 060.9, 70.5, 2.3100, 2.1884, -0.6184, 4.000, 0, win_Nuttall4},
/* 7*/ {"Nuttall3a", 064.2, 61.2, 1.7721, 1.6828, -1.0453, 3.000, 0,
	  win_Nuttall3a},
/* 8*/ {"Nuttall3b", 071.5, 59.8, 1.7037, 1.6162, -1.1352, 3.000, 0,
	  win_Nuttall3b},
/* 9*/ {"Nuttall4a", 082.6, 68.0, 2.1253, 2.0123, -0.7321, 4.000, 0,
	  win_Nuttall4a},
/*10*/ {"BH92", 092.0, 66.1, 2.0044, 1.8962, -0.8256, 4.000, 0, win_BH92},
/*11*/ {"Nuttall4b", 093.3, 66.3, 2.0212, 1.9122, -0.8118, 4.000, 0,
	  win_Nuttall4b},
/*12*/ {"Nuttall4c", 098.1, 65.6, 1.9761, 1.8687, -0.8506, 4.000, 0,
	  win_Nuttall4c},
/*13*/ {"SFT3F", 031.7, 66.7, 3.1681, 3.1502, +0.0082, 3.000, 0, win_SFT3F},
/*14*/ {"SFT3M", 044.2, 65.5, 2.9452, 2.9183, -0.0115, 3.000, 0, win_SFT3M},
/*15*/ {"FTNI", 044.4, 65.6, 2.9656, 2.9355, +0.0169, 3.000, 0, win_FTNI},
/*16*/ {"SFT4F", 044.7, 75.0, 3.7970, 3.7618, +0.0041, 4.000, 0, win_SFT4F},
/*17*/ {"SFT5F", 057.3, 78.5, 4.3412, 4.2910, -0.0025, 5.000, 0, win_SFT5F},
/*18*/ {"SFT4M", 066.5, 72.1, 3.3868, 3.3451, -0.0067, 4.000, 0, win_SFT4M},
/*19*/ {"FTHP", 070.4, 72.3, 3.4279, 3.3846, +0.0096, 4.000, 0, win_FTHP},
/*20*/ {"HFT70", 070.4, 72.2, 3.4129, 3.3720, -0.0065, 4.000, 0, win_HFT70},
/*21*/ {"FTSRS", 076.6, 75.4, 3.7702, 3.7274, -0.0156, 4.719, 0, win_FTSRS},
/*22*/ {"SFT5M", 089.9, 76.0, 3.8852, 3.8340, +0.0039, 5.000, 0, win_SFT5M},
/*23*/ {"HFT90D", 090.2, 76.0, 3.8832, 3.8320, -0.0039, 5.000, 1, win_HFT90D},
/*24*/ {"HFT95", 095.0, 75.6, 3.8112, 3.7590, +0.0044, 5.000, 0, win_HFT95},
/*25*/ {"HFT116D", 116.8, 78.2, 4.2186, 4.1579, -0.0028, 6.000, 1, win_HFT116D},
/*26*/ {"HFT144D", 144.1, 79.9, 4.5386, 4.4697, +0.0021, 7.000, 1, win_HFT144D},
/*27*/ {"HFT169D", 169.5, 81.2, 4.8347, 4.7588, +0.0017, 8.000, 1, win_HFT169D},
/*28*/ {"HFT196D", 196.2, 82.3, 5.1134, 5.0308, +0.0013, 9.000, 1, win_HFT196D},
/*29*/ {"HFT223D", 223.0, 83.3, 5.3888, 5.3000, -0.0011, 10.000, 1, win_HFT223D},
/*30*/ {"HFT248D", 248.4, 84.1, 5.6512, 5.5567, +0.0009, 11.000, 1, win_HFT248D}
};
static const int nwinlist = sizeof (winlist) / sizeof (window);

/* Compute the parameter alpha of Kaiser windows
   from the required PSLL [dB]. Best-fit polynomial
   was obtained from 180 data points between alpha=1
   and alpha=9.95. Maximum error is 0.05 
   Maximum error for PSLL > 30 dB is 0.02 */

static double
kaiser_alpha (const double psll)
{
  const double a0 = -0.0821377;
  const double a1 = 4.71469;
  const double a2 = -0.493285;
  const double a3 = 0.0889732;

  double x = psll / 100.;
  return (((((a3 * x) + a2) * x) + a1) * x + a0);

}

/* Compute the 'recommended overlap' (ROV) [%] of Kaiser windows
   from the parameter alpha. Best-fit polynomial
   was obtained from 180 data points between alpha=1
   and alpha=9.95. Maximum error is 1.5%, mainly due
   to insufficient precision in the data points */

static double
kaiser_rov (const double alpha)
{
  const double a0 = 0.0061076;
  const double a1 = 0.00912223;
  const double a2 = -0.000925946;
  const double a3 = 4.42204e-05;
  double x = alpha;
  return 100. - 1. / (((((a3 * x) + a2) * x) + a1) * x + a0);
}

/* Compute the 'normalized noise-equivalent bandwidth'
   (NENBW) [bins] of Kaiser windows from the parameter alpha.
   Best-fit polynomial was obtained from 180 data 
   points between alpha=1 and alpha=9.95.
   Maximum error is 0.007 bins 
   NOTE that NENBW can be computed precisely 
   from the actual time-domain window values */

static double
kaiser_nenbw (const double alpha)
{
  const double a0 = 0.768049;
  const double a1 = 0.411986;
  const double a2 = -0.0264817;
  const double a3 = 0.000962211;
  double x = alpha;
  return (((((a3 * x) + a2) * x) + a1) * x + a0);
}

/* Compute the 3dB bandwidth (W3db) [bins]
   of Kaiser windows from the parameter alpha.
   Best-fit polynomial was obtained from 180 data 
   points between alpha=1 and alpha=9.95.
   Maximum error is 0.006 bins. */

static double
kaiser_w3db (const double alpha)
{
  const double a0 = 0.757185;
  const double a1 = 0.377847;
  const double a2 = -0.0238342;
  const double a3 = 0.00086012;
  double x = alpha;
  return (((((a3 * x) + a2) * x) + a1) * x + a0);
}

/* Compute the flatness in the central bin [dB]
   of Kaiser windows from the parameter alpha.
   Best-fit polynomial was obtained from 180 data 
   points between alpha=1 and alpha=9.95.
   Maximum error is 0.013 dB. */

static double
kaiser_flatness (const double alpha)
{
  const double a0 = 0.141273;
  const double a1 = 0.262425;
  const double a2 = 0.00642551;
  const double a3 = -0.000405621;
  double x = alpha;
  return -1. / (((((a3 * x) + a2) * x) + a1) * x + a0);
}

static double
kaiser_sbin (const double alpha)
{
  return (sqrt(alpha*alpha+1.));
}

static double
win_Rectangular (double z __attribute__ ((unused)))
{
  return 1;
}

static double
win_Welch (double z)
{
  double w = 2 * z - 1.;
  return 1 - w * w;
}

static double
win_Bartlett (double z)
{
  z *= 2;
  if (z > 1)
    return 2 - z;
  else
    return z;
}

static double
win_Hanning (double z)
{
  return 0.5 * (1 - cos (2 * M_PI * z));
}

static double
win_Hamming (double z)
{
  return 0.54 - 0.46 * cos (2 * M_PI * z);
}

static double
win_Nuttall3 (double z)
{
  z *= 2 * M_PI;
  return 0.375 - 0.5 * cos (z) + 0.125 * cos (2 * z);
}

static double
win_Nuttall4 (double z)
{
  z *= 2 * M_PI;
  return 0.3125 - 0.46875 * cos
    (z) + 0.1875 * cos (2 * z) - 0.03125 * cos (3 * z);
}

static double
win_Nuttall3a (double z)
{
  z *= 2 * M_PI;
  return 0.40897 - 0.5 * cos (z) + 0.09103 * cos (2 * z);
}

static double
win_Nuttall3b (double z)
{
  z *= 2 * M_PI;
  return 0.4243801 - 0.4973406 * cos (z) + 0.0782793 * cos (2 * z);
}

static double
win_Nuttall4a (double z)
{
  z *= 2 * M_PI;
  return 0.338946 - 0.481973 *
    cos (z) + 0.161054 * cos (2 * z) - 0.018027 * cos (3 * z);
}

static double
win_BH92 (double z)
{
  z *= 2 * M_PI;

  return 0.35875 - 0.48829 * cos (z) + 0.14128 * cos (2 * z) -
    0.01168 * cos (3 * z);
}

static double
win_Nuttall4b (double z)
{
  z *= 2 * M_PI;
  return 0.355768 - 0.487396 *
    cos (z) + 0.144232 * cos (2 * z) - 0.012604 * cos (3 * z);
}

static double
win_Nuttall4c (double z)
{
  z *= 2 * M_PI;
  return 0.3635819 - 0.4891775 *
    cos (z) + 0.1365995 * cos (2 * z) - 0.0106411 * cos (3 * z);
}

static double
win_SFT3F (double z)
{
  z *= 2 * M_PI;
  return 0.26526 - 0.5 * cos (z) + 0.23474 * cos (2 * z);
}

static double
win_SFT3M (double z)
{
  z *= 2 * M_PI;
  return 0.28235 - 0.52105 * cos (z) + 0.19659 * cos (2 * z);
}

static double
win_FTNI (double z)
{
  z *= 2 * M_PI;
  return (0.2810639 - 0.5208972 * cos (z) + 0.1980399 * cos (2 * z));
}

static double
win_SFT4F (double z)
{
  z *= 2 * M_PI;
  return 0.21706 - 0.42103 * cos (z)
    + 0.28294 * cos (2 * z) - 0.07897 * cos (3 * z);
}

static double
win_SFT5F (double z)
{
  z *= 2 * M_PI;
  return 0.1881 - 0.36923 * cos (z) +
    0.28702 * cos (2 * z) - 0.13077 * cos (3 * z) + 0.02488 * cos (4 * z);
}

static double
win_SFT4M (double z)
{
  z *= 2 * M_PI;
  return 0.241906 - 0.460841 * cos
    (z) + 0.255381 * cos (2 * z) - 0.041872 * cos (3 * z);
}

static double
win_FTHP (double z)
{
  z = M_PI * (2. * z - 1);
  return (1.0 + 1.912510941
	  * cos (z) + 1.079173272 * cos (2.0 * z) +
	  0.1832630879 * cos (3.0 * z));
}

static double
win_HFT70 (double z)
{
  z *= 2 * M_PI;
  return (1 - 1.90796 * cos (z) +
	  1.07349 * cos (2 * z) - 0.18199 * cos (3 * z));
}

static double
win_FTSRS (double z)
{
  z *= 2 * M_PI;
  return (1.0 - 1.93 * cos (z) + 1.29
	  * cos (2 * z) - 0.388 * cos (3 * z) + 0.028 * cos (4 * z));
}

static double
win_SFT5M (double z)
{
  z *= 2 * M_PI;
  return 0.209671 - 0.407331 * cos
    (z) + 0.281225 * cos (2 * z) - 0.092669 * cos (3 * z) +
    0.0091036 * cos (4 * z);
}

static double
win_HFT90D (double z)
{
  z *= 2 * M_PI;
  return (1 - 1.942604 * cos (z) +
	  1.340318 * cos (2 * z) - 0.440811 * cos (3 * z) +
	  0.043097 * cos (4 * z));
}

static double
win_HFT95 (double z)
{
  z *= 2 * M_PI;
  return (1 - 1.9383379 * cos (z) +
	  1.3045202 * cos (2 * z) - 0.4028270 * cos (3 * z) +
	  0.0350665 * cos (4 * z));
}

static double
win_HFT116D (double z)
{
  z *= 2 * M_PI;
  return (1 - 1.9575375 * cos (z) +
	  1.4780705 * cos (2 * z) - 0.6367431 * cos (3 * z) +
	  0.1228389 * cos (4 * z) - 0.0066288 * cos (5 * z));
}

static double
win_HFT144D (double z)
{
  z *= 2 * M_PI;
  return (1 - 1.96760033 * cos (z)
	  + 1.57983607 * cos (2 * z) - 0.81123644 * cos (3 * z) +
	  0.22583558 * cos (4 * z) - 0.02773848 * cos (5 * z) +
	  0.00090360 * cos (6 * z));
}

static double
win_HFT169D (double z)
{
  z *= 2 * M_PI;
  return (1 - 1.97441843 * cos (z)
	  + 1.65409889 * cos (2 * z) - 0.95788187 * cos (3 * z) +
	  0.33673420 * cos (4 * z) - 0.06364622 * cos (5 * z) +
	  0.00521942 * cos (6 * z) - 0.00010599 * cos (7 * z));
}

static double
win_HFT196D (double z)
{
  z *= 2 * M_PI;
  return (1 - 1.979280420 * cos (z)
	  + 1.710288951 * cos (2 * z) - 1.081629853 * cos (3 * z) +
	  0.448734314 * cos (4 * z) - 0.112376628 * cos (5 * z) +
	  0.015122992 * cos (6 * z) - 0.000871252 * cos (7 * z) +
	  0.000011896 * cos (8 * z));
}

static double
win_HFT223D (double z)
{
  z *= 2 * M_PI;
  return (1 - 1.98298997309 * cos
	  (z) + 1.75556083063 * cos (2 * z) - 1.19037717712 * cos (3 * z) +
	  0.56155440797 * cos (4 * z) - 0.17296769663 * cos (5 * z) +
	  0.03233247087 * cos (6 * z) - 0.00324954578 * cos (7 * z) +
	  0.00013801040 * cos (8 * z) - 0.00000132725 * cos (9 * z));
}

static double
win_HFT248D (double z)
{
  z *= 2 * M_PI;
  return (1 - 1.985844164102 * cos
	  (z) + 1.791176438506 * cos (2 * z) - 1.282075284005 * cos (3 * z) +
	  0.667777530266 * cos (4 * z) - 0.240160796576 * cos (5 * z) +
	  0.056656381764 * cos (6 * z) - 0.008134974479 * cos (7 * z) +
	  0.000624544650 * cos (8 * z) - 0.000019808998 * cos (9 * z) +
	  0.000000132974 * cos (10 * z));
}

static int win_no = -2;
static double win_alpha;

void
set_window (int type, double req_psll, char *name, double *psll, double *rov,
	    double *nenbw, double *w3db, double *flatness, double *sbin)
{
  int i;
  if (type >= 0)
    {				/* window by number */
      if (type < nwinlist)
	{
	  strcpy (name, winlist[type].name);
	  *psll = winlist[type].psll;
	  *rov = winlist[type].rov;
	  *nenbw = winlist[type].nenbw;
	  *w3db = winlist[type].w3db;
	  *flatness = winlist[type].flatness;
	  *sbin = winlist[type].sbin;
	  win_no = type;
	  return;
	}
      else
	gerror ("illegal window number requested.");
    }
  else if (type == -1)
    {				/* flat-top by PSLL */
      win_no = -2;
      for (i = 0; i < nwinlist; i++)
	{
	  if (!(winlist[i].isft))
	    continue;
	  if (winlist[i].psll > req_psll)
	    {
	      win_no = i;
	      strcpy (name, winlist[i].name);
	      *psll = winlist[i].psll;
	      *rov = winlist[i].rov;
	      *nenbw = winlist[i].nenbw;
	      *w3db = winlist[i].w3db;
	      *flatness = winlist[i].flatness;
  	      *sbin = winlist[i].sbin;
	      break;
	    }
	}
      if (win_no == -2)
	gerror ("no matching flat-top window found.");
    }
  else if (type == -2)
    {				/* Kaiser by PSLL */
      if (req_psll < 25 || req_psll > 250)
	gerror ("Kaiser window requested PSLL outside range 25..250");
      win_alpha = kaiser_alpha (req_psll);
      sprintf (name, "Kaiser %.3f", win_alpha);
      *psll = req_psll;
      *rov = kaiser_rov (win_alpha);
      *nenbw = kaiser_nenbw (win_alpha);
      *w3db = kaiser_w3db (win_alpha);
      *flatness = kaiser_flatness (win_alpha);
      *sbin = kaiser_sbin(win_alpha);
      win_no = -1;
    }
  else
    gerror ("illegal window type");
}


void
makewinsincos (int nfft, double bin, double *win, double *winsum,
	       double *winsum2, double *nenbw)
{
  int j;
  double kaiser_scal = 1, z;
  double winval;
  double fact, arg;
  double s,c;

  *winsum = *winsum2 = 0;
  if (win_no == -2)
    gerror ("set_window has not been called.");
  if (win_no == -1)
    kaiser_scal = netlibi0 (M_PI * win_alpha);
  fact = 2.0 * M_PI * bin / ((double) nfft);
  for (j = 0; j < nfft; j++)
    {
      if (win_no == -1)
	{			/* Kaiser */
	  z = 2. * (double) j / (double) nfft - 1.;
	  winval =
	    netlibi0 (M_PI * win_alpha * sqrt (1 - z * z)) / kaiser_scal;
	}
      else
	{
	  z = (double) j / (double) nfft;
	  winval = (*(winlist[win_no].winfun)) (z);
	}
      *winsum += winval;
      *winsum2 += winval * winval;

      arg = fact * (double) j;
#ifdef SINCOS
    asm ("fsincos": "=t" (c), "=u" (s):"0" (arg));
#else
      s = sin (arg);
      c = cos (arg);
#endif
      *(win++) = c*winval;
      *(win++) = -s*winval;
    }
  *nenbw = nfft * *winsum2 / (*winsum * *winsum);
}

void
makewin (int nfft, int half, double *win, double *winsum, double *winsum2,
	 double *nenbw)
{
  int j;
  double kaiser_scal = 1, z;

  *winsum = *winsum2 = 0;
  if (win_no == -2)
    gerror ("set_window has not been called.");
  if (half && nfft % 2)
    gerror ("'half=YES' can only be used for even NFFT.");
  if (win_no == -1)
    kaiser_scal = netlibi0 (M_PI * win_alpha);
  for (j = 0; j <= nfft / 2; j++)
    {
      if (win_no == -1)
	{			/* Kaiser */
	  z = 2. * (double) j / (double) nfft - 1.;
	  win[j] =
	    netlibi0 (M_PI * win_alpha * sqrt (1 - z * z)) / kaiser_scal;
	}
      else
	{
	  z = (double) j / (double) nfft;
	  win[j] = (*(winlist[win_no].winfun)) (z);
	}
      *winsum += win[j];
      *winsum2 += win[j] * win[j];
    }
  if (half)			/* add missing items to winsum */
    {
      for (j = 1; j < nfft / 2; j++)
	{
	  *winsum += win[j];
	  *winsum2 += win[j] * win[j];
	}
    }
  else				/* fill up rest of array */
    {
      for (j = nfft / 2 + 1; j < nfft; j++)
	{
	  if (win_no == -1)
	    {			/* Kaiser */
	      z = 2. * (double) j / (double) nfft - 1.;
	      win[j] =
		netlibi0 (M_PI * win_alpha * sqrt (1 - z * z)) / kaiser_scal;
	    }
	  else
	    {
	      z = (double) j / (double) nfft;
	      win[j] = (*(winlist[win_no].winfun)) (z);
	    }
	  *winsum += win[j];
	  *winsum2 += win[j] * win[j];
	}
    }
  *nenbw = nfft * *winsum2 / (*winsum * *winsum);
}
