/******************************************************************************
    lpsd.c
		
    2019, 2020 by Mert Turkol, mturkol(at)gmail(dot)com - revision and bug fixes 
    2003, 2004 by Michael Troebs, mt@lzh.de and Gerhard Heinzel, ghh@mpq.mpg.de
    
    calculate spectra from time series using discrete Fourier 
    transforms at frequencies equally spaced on a logarithmic axis
    
    lpsd does everything except user interface and data output
 *****************************************************************************/
#define SINCOS
#define FAST 1


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <fftw3.h>
#include "config.h"
#include "ask.h"
#include "IO.h"
#include "genwin.h"
#include "debug.h"
#include "lpsd.h"
#include "misc.h"
#include "errors.h"

#define MAX(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define MIN(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

/*
20.03.2004: http://www.caddr.com/macho/archives/iolanguage/2003-9/549.html
*/
#ifndef __linux__
#include <windows.h>
struct timezone
{
  int tz_minuteswest;
  int tz_dsttime;
};
void
gettimeofday (struct timeval *tv, struct timezone *tz
	      __attribute__ ((unused)))
{
  long int count = GetTickCount ();
  tv->tv_sec = (int) (count / 1000);
  tv->tv_usec = (count % 1000) * 1000;
}

#else
#include <sys/time.h>		/* gettimeofday, timeval */
#endif

#ifdef __linux__
extern double round (double x);
/*
gcc (GCC) 3.3.1 (SuSE Linux) gives warning: implicit declaration of function `round'
without this line - math.h defines function round, but its declaration seems to be
missing in math.h 
*/

#else
/*
int round (double x) {
  return ((int) (floor (x + 0.5)));
}
*/
#endif

/********************************************************************************
 * 	global variables						   	
 ********************************************************************************/
static int nread;
static double winsum;
static double winsum2;
static double nenbw;		/* normalized equivalent noise bandwidth */
static double *dwin;		/* pointer to window function for FFT */

/********************************************************************************
 * 	functions								
 ********************************************************************************/
 // Added on 12/5/2019 for accurate computation of exp(x) - 1
 // Calculate exp(x) - 1.
 // The most direct method is inaccurate for very small arguments.
double 
ExpMinusOne(double x)
{
	const double p1 = 0.914041914819518e-09;
	const double p2 = 0.238082361044469e-01;
	const double q1 = -0.499999999085958e+00;
	const double q2 = 0.107141568980644e+00;
	const double q3 = -0.119041179760821e-01;
	const double q4 = 0.595130811860248e-03;

	double rexp = 0.0;

	// Use rational approximation for small arguments.
	if (fabs(x) < 0.15) {
		rexp = x * ( ( (p2*x + p1)*x + 1.0 ) / 
    ( ( ( (q4*x + q3)*x + q2)*x + q1)*x + 1.0) );
		
    return rexp;
	}

	// For large negative arguments, direct calculation is OK.
	double w = exp(x);
	if (x <= -0.15) {
		rexp = w - 1.0;
		return rexp;
	}

	// The following expression is algebraically equal to exp(x) - 1.
	// The advantage in finite precision arithmetic is that
	// it avoids subtracting nearly equal numbers.
	rexp = w * (0.5 + (0.5 - 1.0 / w));
	return rexp;
}

/* 
	copies nfft values from data to segm
	if drift removal is selected, a linear regression of data is 
	performed and the subtracted values are copied to segm
*/
static void
remove_drift (double *segm, double *data, int nfft, int LR)
{
  int i;
  long double sx, sy, stt, sty, xm, t;
  double a,b;
  
  if (LR == 2)  { /* subtract straight line through first and last point */
    a = data[0];
    b = data[nfft - 1] - data[0] / (double) (nfft - 1.0);

    for (i = 0; i < nfft; i++)  {
      segm[i] = data[i] - (a + b * i);
    }

  }

  else if (LR == 1) { /* linear regression */
    sx = sy = 0;
    for (i = 0; i < nfft; i++)  {
	    sx += i;
	    sy += data[i];
    }

    xm = sx / nfft;
    stt = sty = 0;
    for (i = 0; i < nfft; i++)  {
      t = i - xm;
      stt += t * t;
      sty += t * data[i];
	  }

    b = sty / stt;
    a = (sy - sx * b) / nfft;

    for (i = 0; i < nfft; i++)  {
      segm[i] = data[i] - (a + b * i);
    }

  } //end-if linear regression

  else if (LR == 0) { /* no detrending, just copy data */
      
    for (i = 0; i < nfft; i++)  {
      segm[i] = data[i];
	  }

  } //end-if no-detrend
} //end-of remove_drift()

static void
remove_drift2 (double *a, double *b, double *data, int nfft, int LR)
{
  int i;
  long double sx, sy, stt, sty, xm, ndbl;

  if (LR == 2)  { /* subtract straight line through first and last point */
      *a = data[0];
      *b = data[nfft - 1] - data[0] / (double) (nfft - 1.0);
  }

  else if (LR == 1) { /* linear regression */
    ndbl = (long double) nfft;
    xm = (ndbl - 1.0L) / 2.0L;
    sx = ndbl * xm;
    stt = (ndbl * ndbl - 1.0L) * ndbl / 12.0L;
    sy = sty = 0.L;

    for (i = 0; i < nfft; i++)  {
      sy += data[i];
      sty += (i - xm) * data[i];
	  }

    *b = sty / stt;
    *a = (sy - sx * *b) / nfft;
    
  } //end-if linear regression

  else if (LR == 0){  /* copy data */
    //*a = 1.0;
	  *a = 0.0;
    *b = 0.0;
  } //end-if no detrend
} //end-of remove_drift2


/********************************************************************************
 *	calculates DFT 
 *		
 *	Parameters
 *		nfft	dimension of fft
 *		bin	bin to be calculated
 *		rslt	array for DFT as spectral density and spectrum
 *			and variance
 *			rslt[0]=PSD, rslt[1]=variance(PSD) 
 *			rslt[2]=PS rslt[3]=variance(PS)
 ********************************************************************************/
static void
getDFT (int nfft, double bin, double fsamp, double ovlp, int LR, double *rslt,
	      int *avg)
{
  double *dwincs;		/* pointer to array containing window function*cos,window function*sin */
  int i, j;
  double dft_re, dft_im;	/* real and imaginary part of DFT */
  int start;			/* first index in data array */
  double *data;			/* start address of data */
  double dft2;			/* sum of real part squared and imag part squared */
  int nsum;			/* number of summands */
  double *segm;			/* contains data of one segment without drift */

  double west_q, west_r, west_temp;
  double west_sumw;		/* temp variable for West's averaging */
  double west_m, west_t;

  /* calculate window function */
  dwincs = (double *) xmalloc (2 * nfft * sizeof (double));
  assert (dwincs != 0);

  makewinsincos (nfft, bin, dwincs, &winsum, &winsum2, &nenbw);

  data = get_data ();
  assert (data != 0);

  segm = (double *) xmalloc (nfft * sizeof (double));
  assert (segm != 0);

  /* remove drift from first data segment */
  remove_drift (&segm[0], &data[0], nfft, LR);

  start = 0;
  dft2 = 0.;
  nsum = 1;

  /* calculate first DFT */
  dft_re = dft_im = 0;
  for (i = 0, j = 0; i < nfft; i++, j += 2) {
    dft_re += dwincs[j] * segm[i];
    dft_im += dwincs[j + 1] * segm[i];
  }

  dft2 = dft_re * dft_re + dft_im * dft_im;
  west_sumw = 1.;
  west_m = dft2;
  west_t = 0.;

  //start += nfft * (1.0 - (double) (ovlp / 100.));	/* go to next segment */
  //int xOlap = round(nfft * (double)(ovlp / 100.));
  int segOffset = MAX(1, floor(nfft * (1.0 - (double)(ovlp / 100.))));
  //int segOffset = MAX(1, nfft - xOlap);
  start += segOffset; /* go to next segment */
  /* process other segments if available */
  while (start + nfft <= nread) { //(start + nfft < nread)
      remove_drift (&segm[0], &data[start], nfft, LR);

      /* calculate DFT */
      dft_re = dft_im = 0;
      for (i = 0, j = 0; i < nfft; i++, j += 2) {
        dft_re += dwincs[j] * segm[i];
        dft_im += dwincs[j + 1] * segm[i];
	    }

      dft2 = dft_re * dft_re + dft_im * dft_im;

      west_q = dft2 - west_m; //(x_k - M_k-1)
      west_temp = west_sumw + 1.; //k
      //2nd term in M_k = M_k-1 + (x_k - M_k-1)/k
      west_r = west_q / west_temp;
      west_m += west_r; //M_k-1 + (x_k - M_k-1)/k)
      //S_k-1 + (x_k � M_k)*(x_k � M_k-1)
      west_t += west_r * west_sumw * west_q;
      west_sumw = west_temp;

      nsum++;
      //start += nfft * (1.0 - (double) (ovlp / 100.));	/* go to next segment */
  	  start += segOffset; /* go to next segment */
    } //end-while segmentEnds < nread

  /* return result */
  rslt[0] = west_m;
  /* if only one DFT has been computed, then stddev equals DFT 
     otherwise, divide variance by n-1, then take root
   */
  if (nsum > 2)
    rslt[1] = sqrt (west_t / ((double) nsum - 1.));
  else
    rslt[1] = rslt[0];

  rslt[2] = rslt[0];
  rslt[3] = rslt[1];
  rslt[0] *= 2. / (fsamp * winsum2);	/* power spectral density */
  rslt[1] *= 2. / (fsamp * winsum2);	/* variance of power spectral density */
  rslt[2] *= 2. / (winsum * winsum);	/* power spectrum */
  rslt[3] *= 2. / (winsum * winsum);	/* variance of power spectrum */

  *avg = nsum;

  /* clean up */
  xfree (segm);
  xfree (dwincs);
}

static void
getDFT2 (int nfft, double bin, double fsamp, double ovlp, int LR,
	      double *rslt, int *avg)
{
  double *dwincs;		/* pointer to array containing window function*cos,window function*sin */
  int i;
  double dft_re, dft_im;	/* real and imaginary part of DFT */
  int start;			/* first index in data array */
  double *data;			/* start address of data */
  double dft2;			/* sum of real part squared and imag part squared */
  int nsum;			/* number of summands */
  double a, b;			/* linear regression results */
  double y;			/* time series detrended with window */
  double *winp, *datp;

  double west_q, west_r, west_temp;
  double west_sumw;		/* temp variable for West's averaging */
  double west_m, west_t;

  /* calculate window function */
  dwincs = (double *) xmalloc (2 * nfft * sizeof (double));
  assert (dwincs != 0);

  makewinsincos (nfft, bin, dwincs, &winsum, &winsum2, &nenbw);

  data = get_data ();
  assert (data != 0);

  /* remove drift from first data segment */
  remove_drift2 (&a, &b, &data[0], nfft, LR);

  start = 0;
  dft2 = 0.;
  nsum = 1;

  /* calculate first DFT */
  dft_re = dft_im = 0;
  datp = data;
  winp = dwincs;
  for (i = 0; i < nfft; i++)  {
    y = *(datp++) - (a + b * i);
    dft_re += *(winp++) * y;
    dft_im += *(winp++) * y;
  }

  dft2 = dft_re * dft_re + dft_im * dft_im;
  west_sumw = 1.;
  west_m = dft2;
  west_t = 0.;

  //start += nfft * (1.0 - (double) (ovlp / 100.));	/* go to next segment */
  //int xOlap = round(nfft * (double)(ovlp / 100.));
  int segOffset = MAX(1, floor(nfft * (1.0 - (double)(ovlp / 100.))));
  //int segOffset = MAX( 1, nfft - xOlap );
  start += segOffset; /* go to next segment */
  /* process other segments if available */
  while (start + nfft <= nread) { //(start + nfft < nread)
    remove_drift2 (&a, &b, &data[start], nfft, LR);

    /* calculate DFT */
    dft_re = dft_im = 0;
    datp = data + start;
    winp = dwincs;

    for (i = 0; i < nfft; i++)  {
      y = *(datp++) - (a + b * i);
      dft_re += *(winp++) * y;
      dft_im += *(winp++) * y;
    }

    dft2 = dft_re * dft_re + dft_im * dft_im;

    west_q = dft2 - west_m; //(x_k - M_k-1)
    west_temp = west_sumw + 1.; //k
	  //2nd term in M_k = M_k-1 + (x_k - M_k-1)/k
    west_r = west_q / west_temp; 
    west_m += west_r; //M_k-1 + (x_k - M_k-1)/k)
    //S_k-1 + (x_k - M_k)*(x_k - M_k-1)
    west_t += west_r * west_sumw * west_q;
    west_sumw = west_temp;

    nsum++;
	  //start += nfft * (1.0 - (double) (ovlp / 100.));	/* go to next segment */
	  start += segOffset; /* go to next segment */
  } //end-while segmentEnds < nread

  /* return result */
  rslt[0] = west_m;
  /* if only one DFT has been computed, then stddev equals DFT 
     otherwise, divide variance by n-1, then take root
   */
  if (nsum > 2)
    rslt[1] = sqrt (west_t / ((double) nsum - 1.));
  else
    rslt[1] = rslt[0];

  rslt[2] = rslt[0];
  rslt[3] = rslt[1];
  rslt[0] *= 2. / (fsamp * winsum2);	/* power spectral density */
  rslt[1] *= 2. / (fsamp * winsum2);	/* variance of power spectral density */
  rslt[2] *= 2. / (winsum * winsum);	/* power spectrum */
  rslt[3] *= 2. / (winsum * winsum);	/* variance of power spectrum */

  *avg = nsum;

  /* clean up */
  xfree (dwincs);
}

/*
	calculates paramaters for DFTs
	
	input
		nread		number of data
		fsamp		sampling frequency
		ndiv		desired number of entries in spectrum
		sollavg		desired number of averages
	output
		ndiv		actual number of entries in spectrum
		fspec		frequencies in spectrum
		bins		bins for DFTs
		nffts		dimensions for DFTs
 ********************************************************************************
 	Naming convention	source code	publication
				i		j
				fresc		r_{min}
				fresb		r_{avg}
				fresa		r'
				fres		r''
				ndft		L(j)
				bin		m(j)
 ********************************************************************************/
static void
calc_params (tCFG * cfg, tDATA * data)
{
  double fres, f;
  int i, ndft;
  double bin;
  double navg;
  double ovfact, xov;
  double fresa, fresb, fresc;
  double logfact;

  ovfact = 1. / (1. - (*cfg).ovlp / 100.);
  xov = (1. - (*cfg).ovlp / 100.);

  /* smallest possible freq. resolution with minavg averages */
  fresc = (*cfg).fsamp / nread * (1 + xov * ((*cfg).minAVG - 1));
  /* smallest freq. res. for desAVG avgs. */
  fresb = (*cfg).fsamp / nread * (1 + xov * ((*cfg).desAVG - 1));
  //gfact = log((*cfg).fmax / (*cfg).fmin);
  logfact = 1.0 / ((*cfg).nspec - 1.0) * log((*cfg).fmax / (*cfg).fmin);

  for (i = 0, f = (*cfg).fmin; f <= (*cfg).fmax; i++) {
    /* desired freq. res. */
    //fresa = f / ((*cfg).nspec - 1.) * log ((*cfg).fmax / (*cfg).fmin);	
    /* desired freq. res.; using accurate exp(x) - 1 */
	  fresa = (*cfg).fmin * exp((double)i * logfact) * ( ExpMinusOne(logfact) );	
	  fres = (fresa >= fresb) ? fresa : sqrt (fresa * fresb); //Eqns. 18

    if (fres < fresc) fres = fresc;
    ndft = round ((*cfg).fsamp / fres);
    fres = (*cfg).fsamp / ndft;
    bin = (f / fres);
    navg = ((double) ((nread - ndft)) * ovfact) / ndft + 1; //not used
    (*data).fspec[i] = f;
    (*data).nffts[i] = ndft;
    (*data).bins[i] = bin;
    //(*data).avg[i] = round (navg);
    f = f + fres;
  } //end-for f[i] <= fmax

  (*cfg).nspec = i;		/* counter has been increased by 1 by for loop */
} //end-of calc_params()

void
calculate_lpsd (tCFG * cfg, tDATA * data)
{
  int k;			/* 0..nspec */
  double rslt[4];		/* rslt[0]=PSD, rslt[1]=variance(PSD) rslt[2]=PS rslt[3]=variance(PS) */
  double progress;

  struct timeval tv;
  double start, now, print;

  printf ("Computing output:  00.0%%");
  fflush (stdout);
  gettimeofday (&tv, NULL);
  start = tv.tv_sec + tv.tv_usec / 1e6;
  now = start;
  print = start;

  for (k = 0; k < (*cfg).nspec; k++)  {

    if (FAST)
      getDFT2 ((*data).nffts[k], (*data).bins[k], (*cfg).fsamp, (*cfg).ovlp,
	    (*cfg).LR, &rslt[0], &(*data).avg[k]);
    else
      getDFT ((*data).nffts[k], (*data).bins[k], (*cfg).fsamp, (*cfg).ovlp,
	      (*cfg).LR, &rslt[0], &(*data).avg[k]);
      
    (*data).psd[k] = rslt[0];
    (*data).varpsd[k] = rslt[1];
    (*data).ps[k] = rslt[2];
    (*data).varps[k] = rslt[3];
    gettimeofday (&tv, NULL);
    now = tv.tv_sec + tv.tv_usec / 1e6;
      
    if (now - print > PSTEP)  {
      print = now;
      progress = (100 * ((double) k)) / ((double) ((*cfg).nspec));
      printf ("\b\b\b\b\b\b%5.1f%%", progress);
      fflush (stdout);
    }

  }
  /* finish */
  printf ("\b\b\b\b\b\b  100%%\n");
  fflush (stdout);
  gettimeofday (&tv, NULL);
  printf ("Duration (s)=%5.3f\n\n", tv.tv_sec - start + tv.tv_usec / 1e6);
}

void
calculate_fftw (tCFG * cfg, tDATA * data)
{
  int nfft;			/* dimension of DFT */
  FILE *wfp;
  fftw_plan plan;
  double *rawdata;		/* start address of data */
  double *out;
  double *segm;			/* contains data of one segment without drift */
  int i, j;
  double d;
  int start;
  double *west_sumw;
  double west_q, west_r, west_temp;
  int navg;
  double *fft_ps, *fft_varps;

  struct timeval tv;
  double stt;

  gettimeofday (&tv, NULL);
  stt = tv.tv_sec + tv.tv_usec / 1e6;

  nfft = (*cfg).nfft;

  dwin = (double *) xmalloc (nfft * sizeof (double));
  segm = (double *) xmalloc (nfft * sizeof (double));
  out = (double *) xmalloc (nfft * sizeof (double));
  west_sumw = (double *) xmalloc ((nfft / 2 + 1) * sizeof (double));
  fft_ps = (double *) xmalloc ((nfft / 2 + 1) * sizeof (double));
  fft_varps = (double *) xmalloc ((nfft / 2 + 1) * sizeof (double));

  /* calculate window function */
  makewin (nfft, 0, dwin, &winsum, &winsum2, &nenbw);

  /* import fftw "wisdom" */
  if ((wfp = fopen ((*cfg).wfn, "r")) == NULL)
    message1 ("Cannot open '%s'", (*cfg).wfn);
  else  {
    if (fftw_import_wisdom_from_file (wfp) == 0)
	    message ("Error importing wisdom");
      fclose (wfp);
  }
  /* plan DFT */
  printf ("Planning...");
  fflush (stdout);

  plan = fftw_plan_r2r_1d (nfft, segm, out, FFTW_R2HC, FFTW_ESTIMATE);
  printf ("done.\n");
  fflush (stdout);

  rawdata = get_data ();
  assert (rawdata != 0);

  printf ("Computing output\n");

  /* remove drift from first data segment */
  remove_drift (&segm[0], &rawdata[0], nfft, (*cfg).LR);
  /* multiply data with window function */
  for (i = 0; i < nfft; i++)
    segm[i] = segm[i] * dwin[i];

  fftw_execute (plan);

  d = 2 * (out[0] * out[0]);

  (*data).fft_ps[0] = d;
  west_sumw[0] = 1.;
  (*data).fft_varps[0] = 0;
  for (j = 1; j < nfft / 2 + 1; j++)  {
    d = 2 * (out[j] * out[j] + out[nfft - j] * out[nfft - j]);
    (*data).fft_ps[j] = d;
    west_sumw[j] = 1.;
    (*data).fft_varps[j] = 0;
  }

  navg = 1;
  start = nfft * (1.0 - (double) ((*cfg).ovlp / 100.));

  /* remaining segments */
  while (start + nfft < nread)  {
    printf (".");
    fflush (stdout);
    if (navg % 75 == 0)
	    printf ("\n");

  navg++;
  remove_drift (&segm[0], &rawdata[start], nfft, (*cfg).LR);

  /* multiply data with window function */
  for (i = 0; i < nfft; i++)
  	segm[i] = segm[i] * dwin[i];

    fftw_execute (plan);

    d = 2 * (out[0] * out[0]);
    west_q = d - (*data).fft_ps[0];
    west_temp = west_sumw[0] + 1;
    west_r = west_q / west_temp;
    (*data).fft_ps[0] += west_r;
    (*data).fft_varps[0] += west_r * west_sumw[0] * west_q;
    west_sumw[0] = west_temp;

    for (j = 1; j < nfft / 2 + 1; j++)  {
  	  d = 2 * (out[j] * out[j] + out[nfft - j] * out[nfft - j]);
      west_q = d - (*data).fft_ps[j];
      west_temp = west_sumw[j] + 1;
      west_r = west_q / west_temp;
      (*data).fft_ps[j] += west_r;
      (*data).fft_varps[j] += west_r * west_sumw[j] * west_q;
      west_sumw[j] = west_temp;
    }

    start += nfft * (1.0 - (double) ((*cfg).ovlp / 100.));	/* go to next segment */
  } //end-while loop over remaning segments

  if (navg > 1) {
    
    for (i = 0; i < nfft / 2 + 1; i++)  {
	  (*data).fft_varps[i] = sqrt ((*data).fft_varps[i] / ((double) navg - 1));
	  }
  }
  else  {

    for (i = 0; i < nfft / 2 + 1; i++)  {
	    (*data).fft_varps[i] = (*data).fft_ps[i];
	  }
  }
  /* normalizations and additional information */
  j = 0;
  for (i = 0; i < nfft / 2 + 1; i++)  {

    if (((*cfg).fres * i >= (*cfg).fmin) &&
	      ((*cfg).fres * i <= (*cfg).fmax) && ((*cfg).sbin <= i))
	  {
	    (*data).fspec[j] = (*cfg).fres * i;
	    (*data).ps[j] = (*data).fft_ps[i] / (winsum * winsum);
      (*data).varps[j] = (*data).fft_varps[i] / (winsum * winsum);
      (*data).psd[j] = (*data).fft_ps[i] / ((*cfg).fsamp * winsum2);
      (*data).varpsd[j] = (*data).fft_varps[i] / ((*cfg).fsamp * winsum2);
      (*data).avg[j] = navg;
      (*data).nffts[j] = nfft;
      (*data).bins[j] = (double) i;
      j++;
	  }
  }

  printf ("done.\n");

  gettimeofday (&tv, NULL);
  printf ("Duration (s)=%5.3f\n\n", tv.tv_sec - stt + tv.tv_usec / 1e6);

  /* write wisdom to file */
  if ((wfp = fopen ((*cfg).wfn, "w")) == NULL)
    message1 ("Cannot open '%s'", (*cfg).wfn);
  else  {
    fftw_export_wisdom_to_file (wfp);
    fclose (wfp);
  }
  /* clean up */
  fftw_destroy_plan (plan);

  /* forget wisdom, free memory */
  fftw_forget_wisdom ();
  xfree (fft_ps);
  xfree (fft_varps);
  xfree (west_sumw);
  xfree (dwin);
  xfree (segm);
  xfree (out);
} //end-of calculate_fftw()

/*
	works on cfg, data structures of the calling program
*/
void
calculateSpectrum (tCFG * cfg, tDATA * data)
{
  nread = (*data).nread;

  /* read data file into memory */
  /* and subtract mean data value */
  printf ("\nReading data, subtracting mean...\n");
  nread = floor (((*cfg).tmax - (*cfg).tmin) * (*cfg).fsamp + 1);
  read_file ((*cfg).ifn, (*cfg).ulsb, (*data).mean,
	          (int) ((*cfg).tmin * (*cfg).fsamp), (*data).nread,
	          (*data).comma);

  if ((*cfg).METHOD == 0) {
    calc_params (cfg, data);
    calculate_lpsd (cfg, data);
  }
  else if ((*cfg).METHOD == 1)  {
      calculate_fftw (cfg, data);
  }
} //end-of calculateSpectrum()
