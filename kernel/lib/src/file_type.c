/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   file_type.c
  
   Vers. 2.17
   (C) 1993-2001 Jacques Froment
   Give the type of MegaWave2 external (file) structures and (addition of
   the 2.0 version) the associated type in MegaWave2 internal (C) structures.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*~~~~~~~~~~  This file is part of the MegaWave2 system library ~~~~~~~~~~~~~~~
MegaWave2 is a "soft-publication" for the scientific community. It has
been developed for research purposes and it comes without any warranty.
The last version is available at http://www.cmla.ens-cachan.fr/Cmla/Megawave
CMLA, Ecole Normale Superieure de Cachan, 61 av. du President Wilson,
      94235 Cachan cedex, France. Email: megawave@cmla.ens-cachan.fr 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>     

#ifdef sun4_5
/* Needed for atof() on Sun 5.7 */
#include <stdlib.h>
#endif

#include "ascii_file.h"
#include "file_type.h"
#include "mw.h"
#include "pm.h"
#include "tiffio.h"

static char temp_char;
#define flipl2(p) temp_char = *p ;  *p = *(p+3); *(p+3) = temp_char; \
                  temp_char = *(p+1); *(p+1) = *(p+2); *(p+2) = temp_char   
#define flipl(x) flipl2((char *)(&x))
#define flips(p) temp_char = *p; *p = *(p+1); *(p+1) = temp_char

/* ***** Common functions to be used on mw_type_conv_out[],
   ***** mw_type_conv_in[] and mw_native_ftypes[] arrays.
*/

/* Return the range (order in the array A[]) of the string <b> in the line
   beginning by the string <a>.
*/

int _mw_get_range_array(a,b,A)

char *a;
char *b;
char *A[];

{
  int i,j;

  for (i=0; 
       (((A[i]!=NULL)||(A[i+1]!=NULL)) &&
	((A[i]==NULL)||(strcmp(a,A[i])!=0)||((i!=0)&&(A[i-1]!=NULL)))); i++);
  if (A[i]==NULL)
    mwerror(INTERNAL,1,"[_mw_get_range_array] Unknown line beginning by the string \"%s\" on array \"%s\",\"%s\",...\n",a,A[0],A[1]);
  /* i = index of a in the array A */

  for (j=i+1; (A[j]!=NULL)&&(strcmp(b,A[j])!=0) ; j++);
  return(j-i);
  
}

/* Return the maximal range (order in the array A[]) of the strings in the line
   beginning by the string <a> (that is, the number of strings).
   Return 0 if string <a> is not found.
*/

int _mw_get_max_range_array(a,A)

char *a;
char *A[];

{
  int i,j;

  for (i=0; 
       (((A[i]!=NULL)||(A[i+1]!=NULL)) &&
	((A[i]==NULL)||(strcmp(a,A[i])!=0)||((i!=0)&&(A[i-1]!=NULL)))); i++);
  if (A[i]==NULL) return(0);
  /* i = index of a in the array A */

  for (j=i+1; A[j]!=NULL; j++);
  return(j-i-1);
}

/* Put in <b> the string of range (order in A[]) <r> in the line 
   beginning by the string <a>.
*/

void _mw_put_range_array(a,b,r,A)

char *a;
char *b;
int r;
char *A[];

{
  int i,j;

  for (i=0; 
       (((A[i]!=NULL)||(A[i+1]!=NULL)) &&
	((A[i]==NULL)||(strcmp(a,A[i])!=0)||((i!=0)&&(A[i-1]!=NULL)))); i++);
  if (A[i]==NULL)
    mwerror(INTERNAL,1,"[_mw_put_range_array] Unknown line beginning by the string \"%s\" on array \"%s\",\"%s\",...\n",a,A[0],A[1]);
  /* i = index of a in the array A */  

  for (j=i+1; (A[j]!=NULL); j++);
  if ((r <= 0)||(r >= j-i))
    mwerror(INTERNAL,1,"[_mw_put_range_array] Invalid range %d (out of [1,%d]) for line beginning by the string \"%s\" on array \"%s\",\"%s\",...\n",
	    r,j-i-1,a,A[0],A[1]);
  strcpy(b,A[i+r]);
}


/*  Say if the string <b> in the line beginnning by the string <a> exists or 
    not in the array A[]. Return 1 if it exists, 0 if not.
*/

int _mw_exist_array(a,b,A)

char *a;
char *b;
char *A[];

{
  int i,j;

  for (i=0; 
       (((A[i]!=NULL)||(A[i+1]!=NULL)) &&
	((A[i]==NULL)||(strcmp(a,A[i])!=0)||((i!=0)&&(A[i-1]!=NULL)))); i++);
  if (A[i]==NULL)
    mwerror(INTERNAL,1,"[_mw_exist_array] Unknown line beginning by the string \"%s\" on array \"%s\",\"%s\",...\n",a,A[0],A[1]);

  /* i = index of a in the array A */

  for (j=i+1; (A[j]!=NULL)&&(strcmp(b,A[j])!=0) ; j++);
  if (A[j]!=NULL) return(1); else return(0);

}

/*
   ***
   ***
*/


/*      Converts a string to lowercases */

void _mw_lower_type(type)

char type[];

{
  int i;

  for (i=0; type[i] != '\0'; i++) type[i] = (char) tolower((int) type[i]);
}


/* Return the priority range (order in mw_native_ftypes[]) of the 
   external type <type> in the line <mtype> 
*/

int _mw_get_range_type(type,mtype)

char type[];
char *mtype;

{
  return(_mw_get_range_array(mtype,type,mw_native_ftypes));
}

/* Put in <type> the external type of range (order in mw_native_ftypes[]) 
   <r> in the line <mtype>.
*/

void _mw_put_range_type(type,mtype,r)

char type[];
char *mtype;
int r;

{
  _mw_put_range_array(mtype,type,r,mw_native_ftypes);
}


/*  Say if the native external type <type> associated to the internal (memory)
    type <mtype> exists or not in  mw_native_ftypes[]. 
    Return 1 if it exists, 0 if not.
*/

int _mw_native_ftype_exists(type,mtype)

char type[];
char *mtype;

{
  return(_mw_exist_array(mtype,type,mw_native_ftypes));
}

/*   Say if the external (file) type <ftype> exists for an output conversion 
     of a structure of internal (memory) type <mtype>.
     Return 1 if it exists, 0 if not.
*/

int _mw_ftype_exists_for_output(ftype,mtype)

char *ftype;  
char *mtype;

{
  int i,r;
  char convmtype[mw_mtype_size];

  r = _mw_get_max_range_array(mtype,mw_type_conv_out);
  if (r<=0) return(0); /* No conversion functions for mtype */

  for (i=1; i<=r; i++)
    {
      _mw_put_range_array(mtype,convmtype,i,mw_type_conv_out);
      if (_mw_native_ftype_exists(ftype,convmtype)==1)
	return(1);
    }
  return(0);
}


/*      Make a first choice for the type of the structure to save */

void _mw_make_type(type,type_in,mtype)

char type[],type_in[];
char *mtype;

{
  int r1,r2;

  if ((type[0] == '?') || (type[0] == '\0'))
    /* No former type */
    strcpy(type,type_in);
  else
    {
      r1 = _mw_get_range_type(type,mtype);
      r2 = _mw_get_range_type(type_in,mtype);
      if (r1 > r2) {r1=r2; strcpy(type,type_in);}
    }
}

/* Print the available external type associated to the internal (memory) type
   <mtype>.
*/

void _mw_print_available_ftype_for_output(mtype)

char *mtype; /* internal (memory) type (input) */

{
  int i,r,j,s;
  char convmtype[mw_mtype_size];
  char ftype[mw_ftype_size];

  printf("Internal type \"%s\" may be saved using the following external types :\n",mtype);

  r = _mw_get_max_range_array(mtype,mw_native_ftypes);
  if (r > 0)
    {
      printf("- Native formats are : ");
      for (i=1; i<=r; i++)
	{
	  _mw_put_range_array(mtype,ftype,i,mw_native_ftypes);
	  printf("%s ",ftype);
	}
      printf("\n");
    }
  else
    printf("- The system reports no native format !\n");
  
  r = _mw_get_max_range_array(mtype,mw_type_conv_out);
  if (r > 0)
    {
      printf("- Other formats are : ");
      for (i=1; i<=r; i++)
	{
	  _mw_put_range_array(mtype,convmtype,i,mw_type_conv_out);
	  if (i>1) printf("/ ");
	  printf("(%s) ",convmtype);
	  
	  s = _mw_get_max_range_array(convmtype,mw_native_ftypes);
	  for (j=1; j<=s; j++)
	    {
	      _mw_put_range_array(convmtype,ftype,j,mw_native_ftypes);
	      printf("%s ",ftype);
	    }
	}
      printf("\n");
    }
  else
    printf("- No other formats available.\n");
}

/* Choose the external type to be used to save the structure */

void _mw_choose_type(type,type_force,mtype)

char type[]; /* External type (input : default choice, output :
		effective external type to be used)
	     */
char *type_force; /* User's choice, if any. Input. */
char *mtype; /* internal (memory) type (input) */

{
  if (_mw_native_ftype_exists(type,mtype) == 0)
    /* type is not native : choose new type to be the first native one */
    _mw_put_range_type(type,mtype,1);
  
  if (type_force[0] != '?') 
    /* User forced the type */
    {
      if ((_mw_native_ftype_exists(type_force,mtype) == 0) &&
	  (_mw_ftype_exists_for_output(type_force,mtype)==0))
	/* Forced type is neither native neither available with conversion */
	{
	  mwerror(WARNING,0,"Invalid external type \"%s\" for the internal type \"%s\"; Using \"%s\" instead.\n",type_force,mtype,type);
	  _mw_print_available_ftype_for_output(mtype);
	}
      else strcpy(type,type_force);
    }
}

/*    Make a new comment */

void  _mw_make_comment(comment,comment_in)

char comment[],comment_in[];

{
  char ch[255];

  if (comment_in[0] == '\0') return;

  if (strlen(comment_in) > 25)
      {
	comment_in[20] = '\0';
	strcat(comment_in,"...");
      }

  comment[200] = '\0';
  if (comment[0] != '\0')
    sprintf(ch,"\"%s\",\"%s\"",comment,comment_in);
  else
    sprintf(ch,"\"%s\"",comment_in);
  strcpy(comment,ch);
}

/*----- Additional functions to check the type of a file -----*/

/* Find the sub-type of a PM file */
/* Return 0 if not a PM file, 1 if doesn't need flipping, 2 if it does */

/* In subtype,  PM_X means Monochrome PM format with pixel values in X */
/*              PMC_X mean Color PM format with pixel values in X      */

/* mtype is the associated MegaWave2 memory format */

static int what_kind_of_PM(file,subtype,mtype)

char *file;
char *subtype;
char *mtype;

{
 int need_flipping;
 pmpic   *pm;
 FILE *fp;

 strcpy(subtype,"?");
 strcpy(mtype,"?");
 
 pm = (pmpic*)malloc(sizeof(pmpic));
 if (pm == NULL)
   mwerror(FATAL,0,"Not enough memory to load any file !\n");

 mw_bzero(pm,sizeof(pmpic)); 
 pm->pm_id = PM_MAGICNO;
 pm->pm_np = pm->pm_nband = 1;
 pm->pm_nrow = pm->pm_ncol = 512;
 pm->pm_form = PM_C;
 pm->pm_cmtsize = 0;
 pm->pm_image = NULL;
 pm->pm_cmt = NULL;

 if (!(fp = fopen(file, "r"))) { free(pm); return(0); }
  
 if (fread(pm,PM_IOHDR_SIZE,1,fp) == 0) { free(pm); fclose(fp); return(0);}
 fclose(fp);

 need_flipping = 0;
 if (pm->pm_id != PM_MAGICNO)
    {
      flipl(pm->pm_id);
      flipl(pm->pm_np);
      flipl(pm->pm_nrow);
      flipl(pm->pm_ncol);
      flipl(pm->pm_nband);
      flipl(pm->pm_form);
      flipl(pm->pm_cmtsize);
  
      need_flipping=1;
      if (pm->pm_id != PM_MAGICNO) {free(pm); return(0);}
    }

 strcpy(subtype,"PM");
 if (pm->pm_np > 1)
   {
     if (pm->pm_np == 3)        /* 24-bit color image */
    	 {
    	   strcat(subtype,"C");  
    	 }
     else strcat(subtype,"?");  /* other cases not implemented */
   }

 if (pm->pm_form == PM_A) strcat(subtype,"_A");
 if (pm->pm_form == PM_C) 
 	{
 	  strcat(subtype,"_C");
          if (pm->pm_np == 3) strcpy(mtype,"ccimage");
	  else strcpy(mtype,"cimage");
         }
 if (pm->pm_form == PM_S) strcat(subtype,"_S");
 if (pm->pm_form == PM_I) strcat(subtype,"_I");
 if ((pm->pm_form == PM_F) || (pm->pm_form == PM_F_YUV)
     || (pm->pm_form == PM_F_HSI) || (pm->pm_form == PM_F_HSV))
 	{
 	  strcat(subtype,"_F");
 	  if (pm->pm_np == 3) strcpy(mtype,"cfimage");
	  else strcpy(mtype,"fimage");
 	 }

 free(pm);
 return(need_flipping+1);
}

/* Find the sub-type of a TIFF file */
/* Return 0 if not a TIFF file, 1 if it is */

/* In subtype,  TIFF means Monochrome TIFF format with char pixel values */
/*              TIFFC means 24-bits Color TIFF format with char pixel values */
/* mtype is the associated MegaWave2 memory format */

static int what_kind_of_TIFF(file,subtype,mtype)

char *file;
char *subtype;
char *mtype;

{
 FILE *fp;
 TIFF  *tif;
 short photo,spp;

 strcpy(subtype,"?");
 strcpy(mtype,"?");
 
 tif=TIFFOpen(file,"r");
 if (!tif) return(0);

 TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photo);
 TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
 if (spp == 1) /* 1 plane */
   {
     if (photo == PHOTOMETRIC_PALETTE)
       return(0); /* 8-bits color image : not implemented in MegaWave2 */
     strcpy(subtype,"TIFF");
     strcpy(mtype,"cimage");
   }
 else /* 3 planes */
{
  strcpy(subtype,"TIFFC");
  strcpy(mtype,"ccimage");
}

 TIFFClose(tif);

 return(1);
}

/* Header Format for MW2 files : MW2_<X>/<V>/... 
   Where <X> = name of the structure,
         <V> = version number of this file format (0 if field does not exist).
   Old header format (mwvers <= 2.00) are  MW2_<X>... (no V field)
*/

/* Size of the MW2_ header */
#define h_mw2_size 4 
/* Size min of <X> */
#define h_X_minsize 5
/* Size max of <X> */
#define h_X_maxsize 16

/* Get the size of the X_V header and the format version number.
*/

static void  _Get_MW2_X_V_header(header,Xsize,X_Vsize,version)

char header[]; /* Input X_V header */
int Xsize;     /* Input size of the X header, in bytes */
int *X_Vsize;  /* Return the size of the X_V header, in bytes */
float *version;/* Return the version number of the file format (0=no number) */

{
  char Vfield[SIZE_OF_MW2_BIN_TYPE_ID-1];
  
  *X_Vsize = Xsize;
  *version=0.0;
  if (strlen(header) < Xsize+SIZE_OF_MW2_BIN_TYPE_ID) 
    /* No V field in the header */
    return;
  if ((header[Xsize]!='/')||(header[Xsize+SIZE_OF_MW2_BIN_TYPE_ID-1]!='/')||
      (header[Xsize+2]!='.'))
    return;  /* No V field in the header */

  strncpy(Vfield,&header[Xsize+1],SIZE_OF_MW2_BIN_TYPE_ID-2);
  Vfield[SIZE_OF_MW2_BIN_TYPE_ID-2]='\0';
  *version = atof(Vfield);
  /*printf("[_Get_MW2_X_V_header] header=%s Vfield=%s version=%f\n",header,Vfield,*version);*/
  if (*version <= 0.0) 
    /* No V field in the header */
    {
      *version=0.0;
      return; 
    }
  *X_Vsize+=SIZE_OF_MW2_BIN_TYPE_ID; /* There is a V field */
  return;
}


/* Find the sub-type of a MW2 file */
/* Return 0 if not a MW2 file, 1 if doesn't need flipping, 2 if it does */

/* In subtype, ... */
/*    MW2_CURVES means the MegaWave2 format for the Curves structure */
/*    MW2_FCURVES means the MegaWave2 format for the Fcurves structure */
/* (put here new MW2 binary formats...) */
/* mtype is the associated MegaWave2 memory structure */


static int what_kind_of_MW2(file,subtype,mtype,hsize,version)

char *file;
char *subtype;
char *mtype;
int *hsize;  /* Return the size of the header, in bytes (0=unknown) */
float *version;/* Return the version number of the file format (0=no number) */

{
 int need_flipping;
 short fd;
 char header[h_X_maxsize + SIZE_OF_MW2_BIN_TYPE_ID];      /* read after "MW_2" */
 char h2[3];                    /* read "_2" */
 unsigned short bytesread;      /* Nbre de bytes lus */
 unsigned short first2bytes;    /* The first 2 bytes (MW) */

 strcpy(subtype,"MW2?");
 strcpy(mtype,"?");

 /* Read first 2 bytes (MW) to check flipping */
 fd = open(file, O_RDONLY);
 if (fd == -1) 
   mwerror(FATAL, 0,"File \"%s\" not found or unreadable\n",file);
 bytesread = read(fd,&first2bytes,2) + read(fd,h2,2);
 h2[2]='\0';
 if (bytesread != 4) return(0);
 switch (first2bytes)
   {
   case 0x4D57: /* MW */
     need_flipping=0;
     if (strcmp(h2,"2_")==0) break;
   case 0x574D: /* WM */
     need_flipping=1;
     if (strcmp(h2,"2_")==0) break;
   default:
     mwerror(INTERNAL,1,"[what_kind_of_MW2] file %s is not of MW2 type !\n",file);
   }     

 /* Read end of header format */
 bytesread = read(fd,header,h_X_maxsize + SIZE_OF_MW2_BIN_TYPE_ID);
 if (bytesread <= h_X_minsize) return(0);
 close(fd);

 /* WARNING : check long name before shorter with same root !
              e.g. CURVES before CURVE.
 */
 
 if (strncmp(header,"CURVES",6) == 0)
   {
     strcpy(subtype,"MW2_CURVES");
     strcpy(mtype,"curves");
     _Get_MW2_X_V_header(header,6,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"FCURVES",7) == 0)
   {
     strcpy(subtype,"MW2_FCURVES");
     strcpy(mtype,"fcurves");
     _Get_MW2_X_V_header(header,7,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"DCURVES",7) == 0)
   {
     strcpy(subtype,"MW2_DCURVES");
     strcpy(mtype,"dcurves");
     _Get_MW2_X_V_header(header,7,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"CURVE",5) == 0)
   {
     strcpy(subtype,"MW2_CURVE");
     strcpy(mtype,"curve");
     _Get_MW2_X_V_header(header,5,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"FCURVE",6) == 0)
   {
     strcpy(subtype,"MW2_FCURVE");
     strcpy(mtype,"fcurve");
     _Get_MW2_X_V_header(header,6,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"DCURVE",6) == 0)
   {
     strcpy(subtype,"MW2_DCURVE");
     strcpy(mtype,"dcurve");
     _Get_MW2_X_V_header(header,6,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"MORPHO_LINE",11) == 0)
   {
     strcpy(subtype,"MW2_MORPHO_LINE");
     strcpy(mtype,"morpho_line");
     _Get_MW2_X_V_header(header,11,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"FMORPHO_LINE",12) == 0)
   {
     strcpy(subtype,"MW2_FMORPHO_LINE");
     strcpy(mtype,"fmorpho_line");
     _Get_MW2_X_V_header(header,12,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"MIMAGE",6) == 0)
   {
     strcpy(subtype,"MW2_MIMAGE");
     strcpy(mtype,"mimage");
     _Get_MW2_X_V_header(header,6,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"MORPHO_SETS",10) == 0)
   {
     strcpy(subtype,"MW2_MORPHO_SETS");
     strcpy(mtype,"morpho_sets");
     _Get_MW2_X_V_header(header,10,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"MORPHO_SET",9) == 0)
   {
     strcpy(subtype,"MW2_MORPHO_SET");
     strcpy(mtype,"morpho_set");
     _Get_MW2_X_V_header(header,9,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"CMORPHO_LINE",12) == 0)
   {
     strcpy(subtype,"MW2_CMORPHO_LINE");
     strcpy(mtype,"cmorpho_line");
     _Get_MW2_X_V_header(header,12,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"CFMORPHO_LINE",13) == 0)
   {
     strcpy(subtype,"MW2_CFMORPHO_LINE");
     strcpy(mtype,"cfmorpho_line");
     _Get_MW2_X_V_header(header,13,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"CMIMAGE",7) == 0)
   {
     strcpy(subtype,"MW2_CMIMAGE");
     strcpy(mtype,"cmimage");
     _Get_MW2_X_V_header(header,7,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"CMORPHO_SETS",11) == 0)
   {
     strcpy(subtype,"MW2_CMORPHO_SETS");
     strcpy(mtype,"cmorpho_sets");
     _Get_MW2_X_V_header(header,11,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"CMORPHO_SET",10) == 0)
   {
     strcpy(subtype,"MW2_CMORPHO_SET");
     strcpy(mtype,"cmorpho_set");
     _Get_MW2_X_V_header(header,10,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"SHAPES",6) == 0)
   {
     strcpy(subtype,"MW2_SHAPES");
     strcpy(mtype,"shapes");
     _Get_MW2_X_V_header(header,6,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"SHAPE",5) == 0)
   {
     strcpy(subtype,"MW2_SHAPE");
     strcpy(mtype,"shape");
     _Get_MW2_X_V_header(header,5,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"FLISTS",6) == 0)
   {
     strcpy(subtype,"MW2_FLISTS");
     strcpy(mtype,"flists");
     _Get_MW2_X_V_header(header,6,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"FLIST",5) == 0)
   {
     strcpy(subtype,"MW2_FLIST");
     strcpy(mtype,"flist");
     _Get_MW2_X_V_header(header,5,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"DLISTS",6) == 0)
   {
     strcpy(subtype,"MW2_DLISTS");
     strcpy(mtype,"dlists");
     _Get_MW2_X_V_header(header,6,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 if (strncmp(header,"DLIST",5) == 0)
   {
     strcpy(subtype,"MW2_DLIST");
     strcpy(mtype,"dlist");
     _Get_MW2_X_V_header(header,5,hsize,version);
     *hsize += h_mw2_size;
     return(1+need_flipping);
   }

 return(0);
}

/*     Return in ftype the type of a binary MegaWave2 file (must have a 
       magic number) and in mtype the associated MegaWave2 memory structure. 
       The function returns 2 if a flip has to be performed, 1 else and 0 if 
       no type found. 
*/

int _mw_get_binary_file_type(fname,ftype,mtype,hsize,version)


char *fname;
char *ftype;
char *mtype;
int *hsize;  /* Return the size of the header, in bytes (0=unknown) */
float *version;/* Return the version number of the file format (0=no number) */

{
  short fd;
  unsigned short first2bytes;    /* The first 2 bytes */
  unsigned short bytesread;         /* Nbre de bytes lus */

  strcpy(ftype,"?");
  strcpy(mtype,"?");

  fd = open(fname, O_RDONLY);
  if (fd == -1) 
    mwerror(FATAL, 0,"File \"%s\" not found or unreadable\n",fname);

  *hsize=0;
  *version=0.0;

  bytesread = read(fd,&first2bytes,2);
  close(fd);

  if (bytesread != 2) return(0);

  switch (first2bytes)
    {
    case 0x494D:
      strcpy(ftype,"IMG");
      strcpy(mtype,"cimage");
      return(1); /* No flipping */
      break;
    case 0x4D49:
      strcpy(ftype,"IMG");
      strcpy(mtype,"cimage");
      return(2); /* Do flipping */
      break;      
    case 0x5249:
      strcpy(ftype,"RIM");
      strcpy(mtype,"fimage");
      return(1); /* No flipping */
      break;
    case 0x4952:
      strcpy(ftype,"RIM");
      strcpy(mtype,"fimage");
      return(2); /* Do flipping */
      break;

    case 0x4946:
      strcpy(ftype,"IFP_FSIGNAL");
      strcpy(mtype,"fsignal");
      return(1); /* No flipping */
      break;
    case 0x4649:
      strcpy(ftype,"IFP_FSIGNAL");
      strcpy(mtype,"fsignal");
      return(2); /* Do flipping */
      break;

    case 0x5649: case 0x4956: case 0x5745: case 0x4557:
      /* PM files: check VI, IV, WE, EW */
      return (what_kind_of_PM(fname,ftype,mtype));
      break;

    case 0x4D4D: /* TIFF files */
    case 0x4949:
      return (what_kind_of_TIFF(fname,ftype,mtype));
      break;
      
    case 0x4749:
      strcpy(ftype,"GIF");
      strcpy(mtype,"cimage");
      return(1); /* No flipping */
      break;

    case 0x4947:
      strcpy(ftype,"GIF");
      strcpy(mtype,"cimage");
      return(2); /* Do flipping */
      break;

    case 0x2521:
      strcpy(ftype,"PS");
      strcpy(mtype,"cimage");
      return(1); /* No flipping */
      break;

    case 0x2125:
      strcpy(ftype,"PS");
      strcpy(mtype,"cimage");
      return(2); /* Do flipping */
      break;

    case 0x5032:
      strcpy(ftype,"PGMA");
      strcpy(mtype,"cimage");
      return(1); /* No flipping */
    break;
      
    case 0x3250:
      strcpy(ftype,"PGMA");
      strcpy(mtype,"cimage");
      return(2); /* Do flipping */
      break;

    case 0x5035:
      strcpy(ftype,"PGMR");
      strcpy(mtype,"cimage");
      return(1); /* No flipping */
    break;
      
    case 0x3550:
      strcpy(ftype,"PGMR");
      strcpy(mtype,"cimage");
      return(2); /* Do flipping */
      break;
      
    case 0x4D57: case 0x574D:/* MW : One of the home MegaWave2 binary format */
      return(what_kind_of_MW2(fname,ftype,mtype,hsize,version));
      break;

    default:
      return(0);
      break;
    }
}


/*     Return in ftype the type of a MegaWave2 Ascii file and in mtype the 
       associated MegaWave2 memory structure.
*/

int _mw_get_ascii_file_type(fname,ftype,mtype,hsize,version)


char *fname;
char *ftype;
char *mtype;
int *hsize;  /* Return the size of the header, in bytes (0=unknown) */
float *version;/* Return the version number of the file format (0=no number) */

{
  FILE *fd;
  char line[BUFSIZ];

  strcpy(ftype,"?");
  strcpy(mtype,"?");

  if (!(fd = fopen(fname, "r")))
    {
      mwerror(FATAL, 0,"File \"%s\" not found or unreadable\n",fname);
      return(0);
    }
  
  if (_mw_fascii_search_string(fd,_MW_DATA_ASCII_FILE_HEADER) == EOF)
    {
      fclose(fd);
      return(0);
    }
  *hsize = strlen(_MW_DATA_ASCII_FILE_HEADER);
  *version = 0.0;

  while (fscanf(fd,"%[^\n]\n",line) == 1)
    {
      if (strncmp(line,"def Polygon",11) == 0) 
	{
	  strcpy(ftype,"A_POLY");
	  strcpy(mtype,"polygon");
	  fclose(fd);
	  return(1);
	}
      if (strncmp(line,"def Fpolygon",12) == 0) 
	{
	  strcpy(ftype,"A_FPOLY");
	  strcpy(mtype,"fpolygon");
	  fclose(fd);
	  return(1);
	}
      if ((strncmp(line,"#def Fsignal",12) == 0) ||
	  (strncmp(line,"def Fsignal",11) == 0))
	{
	  strcpy(ftype,"A_FSIGNAL");
	  strcpy(mtype,"fsignal");
	  fclose(fd);
	  return(1);
	}
      if (strncmp(line,"def Cmovie",10) == 0) 
	{
	  strcpy(ftype,"A_CMOVIE");
	  strcpy(mtype,"cmovie");
	  fclose(fd);
	  return(1);
	}
      if (strncmp(line,"def Fmovie",10) == 0) 
	{
	  strcpy(ftype,"A_FMOVIE");
	  strcpy(mtype,"fmovie");
	  fclose(fd);
	  return(1);
	}
      if (strncmp(line,"def Ccmovie",10) == 0) 
	{
	  strcpy(ftype,"A_CCMOVIE");
	  strcpy(mtype,"ccmovie");
	  fclose(fd);
	  return(1);
	}
      if (strncmp(line,"def Cfmovie",10) == 0) 
	{
	  strcpy(ftype,"A_CFMOVIE");
	  strcpy(mtype,"cfmovie");
	  fclose(fd);
	  return(1);
	}
      if (strncmp(line,"def Wtrans1d",12) == 0) 
	{
	  strcpy(ftype,"A_WTRANS1D");
	  strcpy(mtype,"wtrans1d");
	  fclose(fd);
	  return(1);
	}
      if (strncmp(line,"def Wtrans2d",12) == 0) 
	{
	  strcpy(ftype,"A_WTRANS2D");
	  strcpy(mtype,"wtrans2d");
	  fclose(fd);
	  return(1);
	}
      if (strncmp(line,"def Vchains_wmax",16) == 0) 
	{
	  strcpy(ftype,"A_VCHAINS_WMAX");
	  strcpy(mtype,"vchains_wmax");
	  fclose(fd);
	  return(1);
	}      
    }
  fclose(fd);
  return(0);
}

/* Get the file type and some other info associated to fname.

   The function returns 2 if a flip has to be performed, 1 else and 0 if 
   no type found. 
*/

int _mw_get_file_type(fname,ftype,mtype,hsize,version)

char *fname;
char *ftype;
char *mtype;
int *hsize;  /* Return the size of the header, in bytes (0=unknown) */
float *version;/* Return the version number of the file format (0=no number) */

{
  int ret;

  if ((ret=_mw_get_binary_file_type(fname,ftype,mtype,hsize,version)) == 0)
    ret=_mw_get_ascii_file_type(fname,ftype,mtype,hsize,version);
  return(ret);
}
