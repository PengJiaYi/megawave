/*--------------------------- Commande MegaWave -----------------------------*/
/* mwcommand
   name = {sshrink2};
   version = {"1.0"};
   author = {"Lionel Moisan"};
   function = {"Shrink a Fsignal and make its size a power of two"};
   usage = {
           in->in       "input Fsignal",
           out<-out     "shrinked Fsignal"
   };
*/

#include <stdio.h>
#include "mw.h"


/* NB : calling this module with out=in is nonsense */

Fsignal sshrink2(in,out)
Fsignal in,out;
{
    int n,nn,tmp,i,iofs;

    /* Compute new signal size */
    n = in->size;
    nn = 1; tmp = n>>1;
    while (tmp) {tmp>>=1; nn<<=1;}

    /* copy center part of input signal */
    out = mw_change_fsignal(out,nn);
    if (!out) mwerror(FATAL,1,"Not enough memory.");
    iofs = (n-nn)>>1;
    for (i=0;i<nn;i++) out->values[i] = in->values[i+iofs];
    
    return(out);
}



