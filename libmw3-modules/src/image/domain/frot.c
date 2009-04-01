/*--------------------------- MegaWave2 Module -----------------------------*/
/* mwcommand
 name = {frot};
 version = {"1.1"};
 author = {"Lionel Moisan"};
 function = {"Rotate a Fimage using bilinear interpolation"};
 usage = {
   'a':[a=0.0]->a  "rotation angle (in degrees, counterclockwise)",
   'b':[b=0.0]->b  "background grey value",
   'k'->k_flag     "to keep original input image size (keep center position)",
   in->in          "input Fimage",
   out<-out        "output Fimage"
};
*/
/*----------------------------------------------------------------------
 v1.1 (04/2007): simplified header (LM)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include "mw3.h"
#include "mw3-modules.h"

static void bound(int x, int y, float ca, float sa, int *xmin, int *xmax,
                  int *ymin, int *ymax)
{
    int rx, ry;

    rx = floor(ca * (float) x + sa * (float) y);
    ry = floor(-sa * (float) x + ca * (float) y);
    if (rx < *xmin)
        *xmin = rx;
    if (rx > *xmax)
        *xmax = rx;
    if (ry < *ymin)
        *ymin = ry;
    if (ry > *ymax)
        *ymax = ry;
}

/* NB : calling this module with out=in is nonsense */

void frot(Fimage in, Fimage out, float *a, float *b, char *k_flag)
{
    int nx, ny, x, y, x1, y1, adr;
    float ca, sa, xp, yp, a11, a12, a21, a22, ux, uy, xtrans, ytrans;
    int tx1, ty1, tx2, ty2, xmin, xmax, ymin, ymax, sx, sy;

    nx = in->ncol;
    ny = in->nrow;
    ca = (float) cos((double) (*a) * M_PI / 180.0);
    sa = (float) sin((double) (*a) * M_PI / 180.0);

    /********** Compute new image location **********/
    if (k_flag)
    {
        /* crop image and fix center */
        xmin = ymin = 0;
        xmax = nx - 1;
        ymax = ny - 1;
        xtrans =
            0.5 * ((float) (nx - 1) * (1.0 - ca) + (float) (ny - 1) * sa);
        ytrans =
            0.5 * ((float) (ny - 1) * (1.0 - ca) - (float) (nx - 1) * sa);
    }
    else
    {
        /* extend image size to include the whole input image */
        xmin = xmax = ymin = ymax = 0;
        bound(nx - 1, 0, ca, sa, &xmin, &xmax, &ymin, &ymax);
        bound(0, ny - 1, ca, sa, &xmin, &xmax, &ymin, &ymax);
        bound(nx - 1, ny - 1, ca, sa, &xmin, &xmax, &ymin, &ymax);
        xtrans = ytrans = 0.0;
    }
    sx = xmax - xmin + 1;
    sy = ymax - ymin + 1;
    out = mw_change_fimage(out, sy, sx);
    if (!out)
        mwerror(FATAL, 1, "not enough memory\n");

    /********** Rotate image **********/
    for (x = xmin; x <= xmax; x++)
        for (y = ymin; y <= ymax; y++)
        {
            xp = ca * (float) x - sa * (float) y + xtrans;
            yp = sa * (float) x + ca * (float) y + ytrans;
            x1 = floor(xp);
            y1 = floor(yp);
            ux = xp - (float) x1;
            uy = yp - (float) y1;
            adr = y1 * nx + x1;
            tx1 = (x1 >= 0 && x1 < nx);
            tx2 = (x1 + 1 >= 0 && x1 + 1 < nx);
            ty1 = (y1 >= 0 && y1 < ny);
            ty2 = (y1 + 1 >= 0 && y1 + 1 < ny);
            a11 = (tx1 && ty1 ? in->gray[adr] : *b);
            a12 = (tx1 && ty2 ? in->gray[adr + nx] : *b);
            a21 = (tx2 && ty1 ? in->gray[adr + 1] : *b);
            a22 = (tx2 && ty2 ? in->gray[adr + nx + 1] : *b);
            out->gray[(y - ymin) * sx + x - xmin] =
                (1.0 - uy) * ((1.0 - ux) * a11 + ux * a21) +
                uy * ((1.0 - ux) * a12 + ux * a22);
        }
}