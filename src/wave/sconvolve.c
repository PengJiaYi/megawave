/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
name = {sconvolve};
version = {"1.4"};
author = {"Jean-Pierre D'Ales"};
function = {"Convolves a signal with a filter"};
usage = {
 'd':[Decimation=1]->DownRate [1,10]   "Downsampling rate", 
 'i':[Interpolation=1]->UpRate [1,10]  "Upsampling rate",
 'r'->ReflIR                           "Convolution with symetric filter", 
 'e':[Edgemode=0]->Edge [0,3]          "Edge processing mode",
 'b'->Band                             "Convolution with high pass filter", 
 'p':[EdgeProl=0]->Prolong [0,2]       "Extension of output on edges",
 Signal->Signal          "Input signal (fsignal)", 
 FicConvol<-Output       "Result of convolution (fsignal)", 
 ImpulseResponse->Ri     "Impulse response of inner filter (fsignal)", 
	{
	EdgeIR->Edge_Ri
	     "Impulse reponses of edge and preconditionning filters (fimage)"
	}
	};
*/
/*----------------------------------------------------------------------
 v1.4 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/



#include <stdio.h>
/*
  #include <assert.h>
*/

#include  "mw.h"


#define Max(x,y) ((x) >= (y) ? (x) : (y))
#define Min(x,y) ((x) >= (y) ? (y) : (x))
#define Rdup(n,d) ((n) >= 0 ? ((n)/(d))+1 : ((n)/(d)) )
#define Rddown(n,d) ((n) >= 0 ? ((n)/(d)) : ((n)/(d))+1 )

/*----- Return integral round(a/2) for a>=0 -----*/

#define rd2(a) ( ((a) & 1) == 1 ? ((a) >>1)+1 : ((a) >>1) )


/*----- Variables for filters -----*/

static long     sizeres;	/* Size of `Output` */
static long     firstp, lastp;	/* Indices of the first and last prolongation 
				 * coefficients in `result` */
static long	length;		/* length of filter ri */
static long     lhshift, rhshift;	/* Indices of the first and last 
				 * non zero coefficients in h */
static short    lshift, rshift;	/* Indices of the fist and last non zero
				 * coefficients for RI */
static int      prolong;	/* Equals 0 (default) if 
				 * sizeres = signal->size * uprate / downrate
				 * 1 if prolongation of `result` at edges 
				 * 2 if shrinkage of `result` at edges */
static double   (*RI) ();	/* Pointer to low or high-pass impulse
				 * response function */
static Fsignal  ri;		/* Low-pass filter for computation inner
				 * coefficients */
static int      Low = FALSE;	/* Convolution with low-pass filter */
static int      High = TRUE;	/* "	"   high  "     "   */
static int      Reflir = TRUE;  /* Convolution with reflected filter h/g(-n) */


static double
h(n)

  /*----- Low-pass imulse response function -----*/

short           n;

{
  if ((n < lhshift) || (n > rhshift))
    return (0.0);

  /*  assert((n - lhshift >= 0) && (n - lhshift < ri->size)); */

  return (ri->values[n - lhshift]);
}


static double
hr(n)

  /*----- Reflected low-pass imulse response function -----*/

short           n;

{
  return (h(-n));
}



static double
g(n)

  /*----- High-pass imulse response function -----*/

short           n;

{
  if ((n & 1) == 1)
    return (h(1 - n));
  else
    return (-h(1 - n));
}


static double
gr(n)

  /*----- Reflected high-pass imulse response function -----*/

short           n;

{
  return (g(-n));
}



static void
INIT_RI(signal, Ri, band, Prolong, edge, reflir, uprate, downrate)

	/*----- Initializes RI (low or high-pass filtering) -----*/

Fsignal         signal;	        /* original signal */
Fsignal         Ri;		/* Pointer to h or g */
int            *band;	        /* Indicates low or high-pass filtering */
int            *Prolong;	/* Indicates prolongation or shrinkage of
				 * edges */
int             edge;           /* Edge processing mode */
int            *reflir;	        /* Indicates reflexion of filter's ir */
int             uprate;	        /* upsampling rate of signal */
int             downrate;	/* downsampling rate of result */

{
  long            sshift;

  if ((*Prolong >= 1) && ((edge == 1) || (edge == 3)))
    *Prolong = 0;

  prolong = *Prolong;

  ri = Ri;
  length = ri->size;
  lhshift = (long) ri->shift;
  rhshift = lhshift + length - 1;
  if (!band) {
    lshift = lhshift;
    if (reflir)
      RI = hr;
    else
      RI = h;
  } else
    {
      lshift = 2 - lhshift - ri->size;
      if (reflir)
	RI = gr;
      else
	RI = g;
    }

  /*--- Modification for convolution with reflected filters ---*/

  if (reflir) {
    rshift = -lshift;
    lshift = 1 - length + rshift;
  } else
    rshift = length + lshift - 1;

  /*--- Modification for convolution with prolongation ---*/
  /*---or shrinkage near edges ---*/

  switch (prolong) {
 case 0:
    sizeres = uprate * signal->size / downrate;
    break;

 case 1:
    firstp = -Rdup(lshift, downrate);
    lastp = firstp + uprate * signal->size / downrate - 1;
    sizeres = Rddown(uprate * (signal->size - 1) + rshift, downrate) + firstp + 1;
    sshift = Rdup(lshift, downrate) * downrate;
    break;

 case 2:
    sizeres = uprate * (signal->lastp - signal->firstp + 1) / downrate + (long) signal->param;
    sshift = uprate * signal->firstp;
    break;
  }

  if (prolong >= 1) {
    if ((reflir && !band) || (!reflir && band)) {
      lhshift += sshift;
      rhshift += sshift;
    } else
      {
	lhshift -= sshift;
	rhshift -= sshift;
      }
    lshift -= sshift;
    rshift -= sshift;
  }
}



static void
INIT_EDGE_RI(edge_ri, left_ri, right_ri, band)

	/*----- Create separated buffers -----*/
	/*----- for left and right edge processing -----*/

Fimage          edge_ri;	/* Buffer containing impulse responses of all
				 * edge filters */
Fimage          left_ri;	/* Buffer containing impulse responses of all
				 * left edge filters */
Fimage          right_ri;	/* Buffer containing impulse responses of all
				 * right edge filters */
int            *band;	        /* Indicates low or high-pass filtering */

{
  int             N;
  short           i, j;
  short           decal;

  N = left_ri->nrow = right_ri->nrow = edge_ri->nrow / 8;
  left_ri->ncol = right_ri->ncol = edge_ri->ncol;
  if (!band)
    decal = 0;
  else
    decal = 2 * N;

  for (i = 0; i < N; i++) {
    for (j = 0; j < edge_ri->ncol; j++) {
      left_ri->gray[i * left_ri->ncol + j] = edge_ri->gray[(i + decal) * edge_ri->ncol + j];
      right_ri->gray[i * right_ri->ncol + j] = edge_ri->gray[(i + decal + N) * edge_ri->ncol + j];
    }
  }
}







static void
CONV_0(signal, result, uprate, downrate)

		/*--- Convolution of the input 'signal' ---*/
	/*--- with filter 'RI' and prolongation on edges with 0 ---*/

Fsignal         signal;	        /* original signal */
Fsignal         result;	        /* filtered signal */
int             uprate;	        /* upsampling rate of signal */
int             downrate;	/* downsampling rate of result */

{
  long            cs;	       	/* Index of the current point in signal */
  long            k;	       	/* Index of the current point in signal */
  long            ak, bk;      	/* Lower and upper bounds for `k` */
  long            sizei;       	/* Size of upsampled signal */

  sizei = sizeres * downrate;
  for (cs = 0; cs < sizei; cs += downrate) {
    ak = Max(0, (cs - rshift + uprate - 1) / uprate);
    bk = Min(signal->size - 1, (cs - lshift) / uprate);

    for (k = ak; k <= bk; k++)
      result->values[cs / downrate] += signal->values[k] * RI(cs - uprate * k);
  }


  if (prolong == 1) {
    result->firstp = firstp;
    result->lastp = lastp;
    result->param = (float) (signal->size % downrate);
  } else
    {
      result->firstp = 0;
      result->lastp = sizeres - 1;
    }

  result->sgrate = signal->sgrate * (double) uprate / (double) downrate;
}




static void
CONV_PER(signal, result, uprate, downrate)

		/*--- Convolution of the input 'signal' ---*/
	      /*--- with filter 'RI' and edges periodized ---*/

Fsignal         signal;	        /* original signal */
Fsignal         result;	        /* filtered signal */
int             uprate;	        /* upsampling rate of signal */
int             downrate;	/* downsampling rate of result */

{
  long            cs;	       	/* Index of the current point in signal */
  long            k;	       	/* Index of the current point in signal */
  long            ak, bk;      	/* Lower and upper bounds for `k` */
  long            sizei;       	/* Size of upsampled signal */

  sizei = signal->size * uprate;
  for (cs = 0; cs < sizei; cs += downrate) {
    ak = Max(0, (cs - rshift + uprate - 1) / uprate);
    bk = Min(signal->size - 1, (cs - lshift) / uprate);

    for (k = ak; k <= bk; k++)
      result->values[cs / downrate] += signal->values[k] * RI(cs - uprate * k);
  }

  /*---- Left edge processing ----*/

  for (cs = 0; cs <= rshift - uprate; cs += downrate) {
    ak = (cs - rshift) / uprate;
    bk = -1;

    for (k = ak; k <= bk; k++)
      result->values[cs / downrate] += signal->values[signal->size + k] * RI(cs - uprate * k);
  }

  /*---- Right edge processing ----*/

  for (cs = ((sizei + lshift) / downrate) * downrate; cs < sizei; cs += downrate) {
    ak = signal->size;
    bk = (cs - lshift) / uprate;

    for (k = ak; k <= bk; k++)
      result->values[cs / downrate] += signal->values[k - signal->size] * RI(cs - uprate * k);

  }

  result->firstp = rshift;
  result->lastp = result->size + lshift;
  result->sgrate = signal->sgrate * (double) uprate / (double) downrate;
}



static void
CONV_REFL(signal, result, uprate, downrate, band)

		/*--- Convolution of the input ---*/
	/*--- 'signal' with filter 'RI' and edges reflected ---*/

Fsignal         signal;	        /* original signal */
Fsignal         result;	        /* filtered signal */
int             uprate;	        /* upsampling rate of signal */
int             downrate;	/* downsampling rate of result */
int            *band;	        /* Low/High-pass filtering */

{
  long            cs;		    /* Index of the current point in result */
  long            k;		    /* Index of the current point in signal */
  long            ak, bk;	    /* Lower and upper bounds for `k` */
  double          s;		    /* Partial sum for edge processing */
  long            sizei;	    /* Size of upsampled signal */

  sizei = sizeres * downrate;
  for (cs = 0; cs < sizei; cs += downrate) {
    ak = Max(0, (cs - rshift + uprate - 1) / uprate);
    bk = Min(signal->size - 1, (cs - lshift) / uprate);

    for (k = ak; k <= bk; k++) {

      /*
	 assert((cs / downrate >= 0) && (cs / downrate < result->size) && (k >= 0) && (k < signal->size));
	 */
      result->values[cs / downrate] += signal->values[k] * RI(cs - uprate * k);
    }
  }

  /*---- Left edge processing ----*/

  for (cs = 0; cs <= rshift - uprate; cs += downrate) {
    s = 0.0;
    ak = (cs - rshift) / uprate;
    bk = -1;

    if ((!band || (downrate >= 2)) && (length % 2 == 1))
      for (k = ak; k <= bk; k++) {
	/* assert((- k >= 0) && (- k < signal->size)); */
	s += signal->values[-k] * RI(cs - uprate * k);
      }
    else
      for (k = ak; k <= bk; k++) {
	/* assert((- k - 1 >= 0) && (- k - 1 < signal->size)); */
	s += signal->values[-k - 1] * RI(cs - uprate * k);
      }

    /* assert((cs / downrate >= 0) && (cs / downrate < result->size)); */
    if ((!band || (length % 2 == 1)) || (downrate >= 2))
      result->values[cs / downrate] += s;
    else
      result->values[cs / downrate] -= s;
  }

  /*---- Right edge processing ----*/

  for (cs = ((signal->size + lshift) / downrate) * downrate; cs < sizei; cs += downrate) {
    s = 0.0;
    ak = signal->size;
    bk = (cs - lshift) / uprate;

    if ((band || (downrate >= 2)) && (length % 2 == 1))
      for (k = ak; k <= bk; k++) {
	/* 
	   assert((2 * signal->size - k - 2 >= 0) && (2 * signal->size - k - 2 < signal->size));
	   */
	s += signal->values[2 * signal->size - k - 2] * RI(cs - uprate * k);
      }
    else
      for (k = ak; k <= bk; k++) {
	/* 
	   assert((2 * signal->size - k - 1 >= 0) && (2 * signal->size - k - 1 < signal->size));
	   */
	s += signal->values[2 * signal->size - k - 1] * RI(cs - uprate * k);
      }

    /* assert((cs / downrate >= 0) && (cs / downrate < result->size)); */
    if ((!band || (length % 2 == 1)) || (downrate >= 2))
      result->values[cs / downrate] += s;
    else
      result->values[cs / downrate] -= s;
  }

  if (prolong == 1) {
    result->firstp = firstp;
    result->lastp = lastp;
    result->param = (float) (signal->size % downrate);
  } else
    {
      result->firstp = 0;
      result->lastp = sizeres - 1;
    }

  result->sgrate = signal->sgrate * (double) uprate / (double) downrate;
}



static void
CONV_DEC_SPE(signal, result, downrate, left_ri, right_ri)

	/*--- Convolution and decimation of the input 'signal' ---*/
	  /*--- with filter 'RI' and special edge processing ---*/

Fsignal         signal;	        /* original signal */
Fsignal         result;	        /* filtered signal */
int             downrate;	/* downrateation of result */
Fimage          left_ri, right_ri;	/* Impulse responses for left and
					 * right filters */

{
  long            c, c2;	/* Indices of the current point in result and
			       	 * of the corresponding point in signal */
  long            k;	       	/* Index of the current point in signal */
  long            ak, bk;      	/* Lower and upper bounds for `k` */
  long            N;	       	/* Cancellation degree of wavelet */
  Fsignal         i;

  i = signal;
  N = left_ri->nrow;

  for (c = N; c < result->size - N; c++) {
    c2 = c * downrate;
    ak = Max(0, c2 - rshift);
    bk = Min(i->size - 1, c2 - lshift);

    for (k = ak; k <= bk; k++)
      result->values[c] += i->values[k] * RI(c2 - k);

  }

  /*---- Left edge processing ----*/

  for (c = 0; c < N; c++) {
    c2 = c * downrate;

    for (k = 0; k < N + c2 + 1; k++)
      result->values[c] += i->values[k] * left_ri->gray[c * left_ri->ncol + k];
  }

  /*---- Right edge processing ----*/

  for (c = result->size - N; c < result->size; c++) {
    c2 = c * downrate;

    for (k = c2 - N + 1; k < i->size; k++)
      result->values[c] += i->values[k] * right_ri->gray[(result->size - c - 1) * right_ri->ncol + i->size - k - 1];
  }

  result->firstp = N;
  result->lastp = result->size - N - 1;
  result->sgrate = signal->sgrate / (double) downrate;
}




static void
CONV_INT_SPE(signal, result, uprate, left_ri, right_ri)

	/*--- Interpolation and convolution of the input 'signal' ---*/
	   /*--- with filter 'RI' and special edge processing ---*/

    Fsignal         signal;	/* original signal */
    Fsignal         result;	/* filtered signal */
    int             uprate;	/* upsampling rate of signal */
    Fimage          left_ri, right_ri;	/* Impulse responses for left and
					 * right filters */

{
  long            c;	       	/* Index of the current point in result */
  long            k;	       	/* Index of the current point in signal */
  long            ak, bk;      	/* Lower and upper bounds for `k` */
  long            N;	       	/* Cancellation degree of wavelet */
  Fsignal         i;

  i = signal;
  N = left_ri->nrow;


  for (c = N + 1; c < result->size - N - 1; c++) {
    ak = Max(N, (c - rshift + uprate - 1) / uprate);
    bk = Min(i->size - N - 1, (c - lshift) / uprate);

    for (k = ak; k <= bk; k++)
      result->values[c] += i->values[k] * RI(c - 2 * k);
  }

  /*---- Left edge processing ----*/

  for (c = 0; c <= 3 * N - 2; c++) {
    ak = Max(0, (c - N + 1) / uprate);
    bk = N - 1;

    for (k = ak; k <= bk; k++)
      result->values[c] += i->values[k] * left_ri->gray[k * left_ri->ncol + c];
  }

  /*---- Right edge processing ----*/

  for (c = result->size - 3 * N + 1; c < result->size; c++) {
    ak = i->size - N;
    bk = Min(i->size - 1, (c + N - 1) / uprate);

    for (k = ak; k <= bk; k++)
      result->values[c] += i->values[k] * right_ri->gray[(i->size - k - 1) * left_ri->ncol + result->size - c - 1];
  }

  result->firstp = 3 * N - 1;
  result->lastp = result->size - 3 * N;
  result->sgrate = signal->sgrate * (double) uprate;
}



static void
CONV_SPE(signal, result, uprate, downrate, band, left_ri, right_ri)

/*--- Interpolation and convolution of row number '' of the input ---*/
/*--- 'signal' with filter 'RI' and special edge processing ---*/

    Fsignal         signal;	/* original signal */
    Fsignal         result;	/* filtered signal */
    int             uprate;	/* upsampling rate of signal */
    int             downrate;	/* downrateation of result */
    int            *band;	/* Low/High-pass filtering */
    Fimage          left_ri, right_ri;	/* Impulse responses for left and
					 * right filters */

{

    if (downrate >= 2)
	CONV_DEC_SPE(signal, result, downrate, left_ri, right_ri);
    else
	CONV_INT_SPE(signal, result, uprate, left_ri, right_ri);
}





void
sconvolve(Signal, Output, DownRate, UpRate, ReflIR, Band, Edge, Prolong, Ri, Edge_Ri)

  /*----- Convolves `Signal` with `Ri`, eventually after interpolation -----*/
		/*----- and/or before decimation -----*/

Fsignal         Signal;	        /* Input signal */
Fsignal         Output;	        /* Output : convolved signal */
int            *DownRate;	/* Indicates the rate of decimation */
int            *UpRate;	        /* Indicates the rate of interpolation */
int            *ReflIR;	        /* Indicates reflexion of the filter's IR */
int            *Edge;	        /* Equal 0 if extension with 0 */
                                /* 1 if periodization */
                                /* 2 if reflexion */
                                /* 3 (default) if special treatment of edges */
int            *Band;	        /* Indicates convolution with low or
			         * high-pass filter */
int            *Prolong;	/* Equals 0 (default) if 
				 * sizeres = signal->size * uprate / downrate
				 * 1 if prolongation of `Output` at edges 
				 * 2 if shrinkage of `Output` at edges */
Fsignal         Ri;		/* Impulse response of the low pass filter */
Fimage          Edge_Ri;	/* Impulse responses of filters for special
				 * edge processing */

{
  void            (*CONVOL) ();	       /* Pointer to the convolution
				        * function */
  Fimage          Left_Ri, Right_Ri;   /* Buffer containing impulse
				        * responses of all left and right
				        * edge filters */
  long            l;		       /* Index of current point in `Output` */


  /*--- Initialization of filter for inner coefficients processing ---*/

  INIT_RI(Signal, Ri, Band, Prolong, *Edge, ReflIR, *UpRate, *DownRate);


  /*--- Initialization of filters for edge coefficients processing ---*/
  /*--- (if selected) ---*/

  if (*Edge == 3) {
    Left_Ri = mw_new_fimage();
    Right_Ri = mw_new_fimage();
    if (mw_alloc_fimage(Left_Ri, Edge_Ri->nrow / 8, Edge_Ri->ncol) == NULL)
      mwerror(FATAL, 1, "Allocation of buffer for edge r.i. refused!\n");
    if (mw_alloc_fimage(Right_Ri, Edge_Ri->nrow / 8, Edge_Ri->ncol) == NULL)
      mwerror(FATAL, 1, "Allocation of buffer for edge r.i. refused!\n");
    INIT_EDGE_RI(Edge_Ri, Left_Ri, Right_Ri, Band);
  }

  /*----- Selection of the type of edge processing -----*/

  switch (*Edge) {
 case 0:
    CONVOL = CONV_0;
    break;

 case 1:
    CONVOL = CONV_PER;
    break;

 case 2:
    CONVOL = CONV_REFL;
    break;

 case 3:
    CONVOL = CONV_SPE;
    break;
  }

  /*----- Initialization of `Output` -----*/

  Output = mw_change_fsignal(Output, sizeres);
  if (Output == NULL)
    mwerror(FATAL, 1, "Not enough memory for output!\n");

  for (l = 0; l < sizeres; l++)
    Output->values[l] = 0.0;


  /*----- Convolution of Signal -----*/

  CONVOL(Signal, Output, *UpRate, *DownRate, Band, Left_Ri, Right_Ri);

  if (*Edge == 3) {
    mw_delete_fimage(Left_Ri);
    mw_delete_fimage(Right_Ri);
  }
}
