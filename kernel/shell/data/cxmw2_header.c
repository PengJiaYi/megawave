/* XMegaWave2 main file */
/* generated by the MegaWave2 cxmw2 macro */

#include "XMW2.h"

extern int mwrunmode;

main (argc, argv)
int argc;
char **argv;
{

/* This modules are used by XMegaWave2 internally */
/* So they must be defined */
extern _mw2_splot();
extern _mw2_cmview();
extern _mw2_ccmview();
static _MW_OPTION NullGroup[] = {
{_MW_BUTTON,"splot",_mw2_splot},
{_MW_BUTTON,"cmview",_mw2_cmview},
{_MW_BUTTON,"ccmview",_mw2_ccmview},
{0,NULL,NULL}
   };


