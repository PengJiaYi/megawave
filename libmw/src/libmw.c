/*
 * libmw.c
 */

#include <stdlib.h>

#include "libmw-defs.h"
#include "error.h"

/*
 * native external (file) types sorted in decreasing priority order
 * and associated to an internal (memory) type, which must be listed
 * in first position on each line
 * you may change the order if you want to change default priorities.
 */

char * mw_native_ftypes[] = 
{
     "cimage", "IMG", "PM_C", "GIF", "TIFF", "PGMA", "PGMR", "BMP",
     "JFIF", "PS", "EPSF", "INR", "MTI", "BIN", NULL,
     "fimage", "RIM", "PM_F", NULL, 
     "ccimage", "PMC_C", "TIFFC", "BMPC", "PPM", "JFIFC", NULL, 
     "cfimage", "PMC_F", NULL, 
     "fsignal", "A_FSIGNAL", "WAVE_PCM", NULL, 
     "curve", "MW2_CURVE", NULL, 
     "curves", "MW2_CURVES", NULL, 
     "fcurve", "MW2_FCURVE", NULL, 
     "fcurves", "MW2_FCURVES", NULL, 
     "dcurve", "MW2_DCURVE", NULL, 
     "dcurves", "MW2_DCURVES", NULL, 
     "polygon", "A_POLY", NULL, 
     "polygons", "A_POLY", NULL, 
     "fpolygon", "A_FPOLY", NULL, 
     "fpolygons", "A_FPOLY", NULL, 
     "morpho_line", "MW2_MORPHO_LINE", NULL, 
     "fmorpho_line", "MW2_FMORPHO_LINE", NULL, 
     "morpho_set", "MW2_MORPHO_SET", NULL, 
     "morpho_sets", "MW2_MORPHO_SETS", NULL, 
     "mimage", "MW2_MIMAGE", NULL, 
     "cmorpho_line", "MW2_CMORPHO_LINE", NULL, 
     "cfmorpho_line", "MW2_CFMORPHO_LINE", NULL, 
     "cmorpho_set", "MW2_CMORPHO_SET", NULL, 
     "cmorpho_sets", "MW2_CMORPHO_SETS", NULL, 
     "cmimage", "MW2_CMIMAGE", NULL, 
     "shape", "MW2_SHAPE", NULL, 
     "shapes", "MW2_SHAPES", NULL, 
     "flist", "MW2_FLIST", NULL, 
     "flists", "MW2_FLISTS", NULL, 
     "dlist", "MW2_DLIST", NULL, 
     "dlists", "MW2_DLISTS", NULL, 
     "wpack2d", "A_WPACK2D", NULL, 
     NULL, 
};


/*
 * a short description for each implemented file types.
 */
char * mw_ftypes_description[] = 
{
     "IMG", "Char image format defined by the defunct PCVision"
             " software and used by MegaWave1 [1x8-bits plane]", 
     "PM_C", "PM char image format [1x8-bits plane]", 
     "GIF", "GIF87 char image format (Graphics Interchange Format)"
             " [1x8-bits plane]", 
     "TIFF", "Tag char image format [1x8-bits plane]", 
     "PGMA", "PGM char image format (portable graymap), ascii version"
             " [1x8-bits plane]",
     "PGMR", "PGM char image format (portable graymap), rawbits"
             " version [1x8-bits plane]",
     "BMP", "Microsoft Windows char image format (BitMaP) [1x8-bits plane]", 
     "JFIF", "JPEG/JFIF char image format from the Independent JPEG"
             " Group, monochrome version [1x8-bits plane]", 
     "PS", "PostScript (level 1) char image format, for output objects"
             " only [1x8-bits plane]", 
     "EPSF", "Encapsulated PostScript (level 1) char image format, for"
             " output objects only [1x8-bits plane]", 
     "INR", "Old char image format defined by the software Inrimage"
             " from INRIA) [1x8-bits plane]", 
     "MTI", "Old char image format defined by the software MultImage"
             " (from 2AI). Quite exotic now [1x8-bits plane]", 
     "BIN", "Universal binary char image format. No header (squared"
             " images only) [1x8-bits plane]", 
     "RIM", "Original float image format defined by MegaWave1"
             " [1x32-bits plane]", 
     "PM_F", "PM float image format [1x32-bits plane]", 
     "PMC_C", "PM color image format [3x8-bits planes]", 
     "TIFFC", "Tag color image format [3x8-bits planes]", 
     "BMPC", "Microsoft Windows color image format (BitMaP) [3x8-bits planes]", 
     "PPM", "Portable pixmap color image format, rawbits version"
             " [3x8-bits planes]", 
     "JFIFC", "JPEG/JFIFC color image format from the Independent JPEG"
             " Group [3x8-bits planes]", 
     "PMC_F", "PM color float image format [3x32-bits planes]", 
     "A_FSIGNAL", "MegaWave2 Data Ascii format for Fsignal internal type", 
     "WAVE_PCM", "Microsoft's RIFF WAVE sound file format with PCM encoding", 
     "MW2_CURVE", "MegaWave2 binary format for Curve internal type", 
     "MW2_CURVES", "MegaWave2 binary format for Curves internal type", 
     "MW2_FCURVE", "MegaWave2 binary format for Fcurve internal type", 
     "MW2_FCURVES", "MegaWave2 binary format for Fcurves internal type", 
     "MW2_DCURVE", "MegaWave2 binary format for Dcurve internal type", 
     "MW2_DCURVES", "MegaWave2 binary format for Dcurves internal type", 
     "A_POLY", "MegaWave2 Data Ascii format for Polygon and Polygons"
             " internal types", 
     "A_FPOLY", "MegaWave2 Data Ascii format for Fpolygon and"
             " Fpolygons internal types", 
     "MW2_MORPHO_LINE", "MegaWave2 binary format for Morpho_line"
             " internal type", 
     "MW2_FMORPHO_LINE", "MegaWave2 binary format for Fmorpho_line"
             " internal type", 
     "MW2_MORPHO_SET", "MegaWave2 binary format for Morpho_set internal type", 
     "MW2_MORPHO_SETS", "MegaWave2 binary Ascii format for Morpho_sets"
             " internal type", 
     "MW2_MIMAGE", "MegaWave2 Data binary format for Mimage internal type", 
     "MW2_CMORPHO_LINE", "MegaWave2 binary format for Cmorpho_line"
             " internal type", 
     "MW2_CFMORPHO_LINE", "MegaWave2 binary format for Cfmorpho_line"
             " internal type", 
     "MW2_CMORPHO_SET", "MegaWave2 binay format for Cmorpho_set internal type", 
     "MW2_CMORPHO_SETS", "MegaWave2 binary format for Cmorpho_sets"
             " internal type", 
     "MW2_CMIMAGE", "MegaWave2 binary format for Cmimage internal type", 
     "MW2_SHAPE", "MegaWave2 binary format for Shape internal type", 
     "MW2_SHAPES", "MegaWave2 binary format for Shapes internal type", 
     "MW2_FLIST", "MegaWave2 binary format for Flist internal type", 
     "MW2_FLISTS", "MegaWave2 binary format for Flists internal type", 
     "MW2_DLIST", "MegaWave2 binary format for Dlist internal type", 
     "MW2_DLISTS", "MegaWave2 binary format for Dlists internal type", 
     "A_WPACK2D", "MegaWave2 Data Ascii format for Wpack2d internal"
             " type (with pointers to signal and image files)", 
     NULL, 
};

/*
 * TYPE CONVERSION
 */

/* FIXME : global vars */

/*
 * list all output internal type conversion possibilities
 * using the following format :
 * i1, o11, o12, ..., o1n, NULL, i2, o21, ... o2p, NULL, ..., NULL, NULL
 * string i1 contains the first input internal type and strings
 * o11, o12, ..., o1n all possible output types in the conversion i->o
 */

char * mw_type_conv_out[] =
{
     "fimage", "cimage", "cfimage", "ccimage", NULL, 
     "cimage", "fimage", "cfimage", "ccimage", NULL, 
     "cfimage", "fimage", "cimage", "ccimage", NULL, 
     "ccimage", "fimage", "cimage", "cfimage", NULL, 
     "curves", "polygons", "fcurves", "fpolygons", "dcurves", "curve",
     "polygon", "fcurve", "fpolygon", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "polygons", "curves", "fcurves", "fpolygons", "dcurves", "curve",
     "polygon", "fcurve", "fpolygon", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "fcurves", "curves", "polygons", "fpolygons", "dcurves", "curve",
     "polygon", "fcurve", "fpolygon", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "fpolygons", "curves", "polygons", "fcurves", "dcurves", "curve",
     "polygon", "fcurve", "fpolygon", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "dcurves", "curves", "polygons", "fcurves", "fpolygons", "curve",
     "polygon", "fcurve", "fpolygon", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "curve", "curves", "polygons", "fcurves", "fpolygons", "dcurves",
     "polygon", "fcurve", "fpolygon", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "polygon", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flists", "flist", "dlists",
     "dlist", NULL, 
     "fcurve", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flists", "flist", "dlists",
     "dlist", NULL, 
     "fpolygon", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "point_curve", "point_fcurve", "point_dcurve", NULL, 
     "point_fcurve", "point_curve", "point_dcurve", NULL, 
     "point_dcurve", "point_curve", "point_fcurve", NULL, 
     "dcurve", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flists", "flist", "dlists",
     "dlist", NULL, 
     "morpho_line", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "fmorpho_line", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "mimage", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "morpho_set", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "morpho_sets", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "cmorpho_line", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "cfmorpho_line", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "cmimage", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "cmorpho_sets", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "cmorpho_set", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "flists", "flist", "dlists", "dlist", NULL, 
     "flists", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flist", "dlists", "dlist", NULL, 
     "flist", "curves", "polygons", "fcurves", "fpolygons", "dcurves",
     "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flists", "dlists", "dlist", NULL, 
     "dlists", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flists", "flist", "dlist", NULL, 
     "dlist", "curves", "polygons", "fcurves", "fpolygons", "dcurves",
     "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flists", "flist", "dlists", NULL, 
     NULL
};

/*
 * list all input internal type conversion possibilities
 * using the following format :
 * o1, i11, i12, ..., i1n, NULL, o2, i21, ... i2p, NULL, ..., NULL, NULL
 * string o1 contains the first output internal type and strings
 * i11, i12, ..., i1n all possible input types in the conversion i->o
 */

char * mw_type_conv_in[] =
{
     "fimage", "cimage", "cfimage", "ccimage", NULL, 
     "cimage", "fimage", "cfimage", "ccimage", NULL, 
     "cfimage", "fimage", "cimage", "ccimage", NULL, 
     "ccimage", "fimage", "cimage", "cfimage", NULL, 
     "curves", "polygons", "fcurves", "fpolygons", "dcurves", "curve",
     "polygon", "fcurve", "fpolygon", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "polygons", "curves", "fcurves", "fpolygons", "dcurves", "curve",
     "polygon", "fcurve", "fpolygon", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "fcurves", "curves", "polygons", "fpolygons", "dcurves", "curve",
     "polygon", "fcurve", "fpolygon", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "fpolygons", "curves", "polygons", "fcurves", "dcurves", "curve",
     "polygon", "fcurve", "fpolygon", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "dcurves", "curves", "polygons", "fcurves", "fpolygons", "curve",
     "polygon", "fcurve", "fpolygon", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "curve", "curves", "polygons", "fcurves", "fpolygons", "dcurves",
     "polygon", "fcurve", "fpolygon", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "polygon", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flists", "flist", "dlists",
     "dlist", NULL, 
     "fcurve", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flists", "flist", "dlists",
     "dlist", NULL, 
     "fpolygon", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "dcurve", "morpho_line",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "point_curve", "point_fcurve", "point_dcurve", NULL, 
     "point_fcurve", "point_curve", "point_dcurve", NULL, 
     "point_dcurve", "point_curve", "point_fcurve", NULL, 
     "dcurve", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flists", "flist", "dlists",
     "dlist", NULL, 
     "morpho_line", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "fmorpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "fmorpho_line", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "mimage", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "mimage", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "morpho_set", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "morpho_set", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_sets",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "morpho_sets", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "cmorpho_line", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "cmorpho_line", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cfmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "cfmorpho_line", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cmimage", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "cmimage", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmorpho_sets",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "cmorpho_sets", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_set", "flists", "flist", "dlists", "dlist", NULL, 
     "cmorpho_set", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "flists", "flist", "dlists", "dlist", NULL, 
     "flists", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flist", "dlists", "dlist", NULL, 
     "flist", "curves", "polygons", "fcurves", "fpolygons", "dcurves",
     "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flists", "dlists", "dlist", NULL, 
     "dlists", "curves", "polygons", "fcurves", "fpolygons",
     "dcurves", "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flists", "flist", "dlist", NULL, 
     "dlist", "curves", "polygons", "fcurves", "fpolygons", "dcurves",
     "curve", "polygon", "fcurve", "fpolygon", "dcurve",
     "morpho_line", "fmorpho_line", "mimage", "morpho_set",
     "morpho_sets", "cmorpho_line", "cfmorpho_line", "cmimage",
     "cmorpho_sets", "cmorpho_set", "flists", "flist", "dlists", NULL, 
     NULL
};

char * mwname = NULL;         /* Name of the module */
char * mwgroup = NULL;        /* Group of the module */
int mwdbg = FALSE;
