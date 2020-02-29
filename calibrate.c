#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TWOPI 6.28318530717959
#define ndata 3.6e5

/*
	ulsb=1e-3 and fsamp=10000 lead to a noise floor of 4.08248290463863e-06
	
	gcc -Wall -o calibrate calibrate.c -lm 
*/
int main(void) {

	double fsamp=10.;			/* sampling frequency [Hz]*/
	double f1 = 1.234;			/* first signal frequency [Hz] */
	double amp1 = 2.82842712474619;		/* 2 Vrms */
	double f2 = 2.5002157;			/* second signal frequency [Hz] */
	double amp2 = 1.0;			/* 0.707 Vrms */
	double ulsb = 1e-3;			/* value of 1 LSB in Volt */

	int i;
	double t, u, ur;

	for (i=0;i<ndata;i++) {
		t=(double)i/fsamp;
		u=amp1*sin(TWOPI*f1*t)+amp2*sin(TWOPI*f2*t);
		/* add noise */
		ur=floor(u/ulsb+0.5)*ulsb;			/* rounding generates noise floor */
								/* with LSD=ulsb/sqrt(6*fsamp)    */
//		printf("%8.5f\n",ur);				/* ASCII output */
		printf("%10.6f %8.5f %8.5f\n",t,ur,1.1*ur);	/* ASCII output */
//		fwrite(&ur,sizeof(double),1,stdout);		/* double binary output */
	}

	return 0;
}
