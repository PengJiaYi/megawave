/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   cimage_io.c
  
   Vers. 1.12
   (C) 1993-2002 Jacques Froment
   Input/Output private functions for the Cimage structure

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
#include <sys/stat.h>
#include <string.h>
#include <math.h>
#ifdef Linux
#include <sys/types.h>
#include <unistd.h>
#endif
#include "mw.h"


long fsize(fd)  /* Return the size of the file */

int fd; /* file descriptor given by open */

{
  struct stat  stbuf;

  if (fstat(fd, &stbuf) < 0)
    mwerror(INTERNAL,0,"[fsize] Cannot get stat from the opened file\n");
  return (stbuf.st_size);
}                        


/*~~~~~~ MegaWaveI formats ~~~~~*/

#define PROTECT_NORMAL 0644  /* rw- pour l'utilisateur, r-- pour les autres */


/*--- Cree l'entete d'un fichier <NomFic> au format ImageAction, ayant   ---*/
/*--- pour dimension d'image <dx>,<dy> et pour commentaires <comment>. ---*/
/* Retourne 0 si OK, -1 si creation impossible ou erreur d'ecriture */

#ifdef __STDC__
static short _MAKEHEADER_IMG(char *NomFic, short dx, short dy,char *comment)
#else
static short _MAKEHEADER_IMG(NomFic,dx,dy,comment)
char *NomFic,*comment;
short dx,dy;
#endif

{ short fic;
  unsigned short BufferEntete[160];       /* Buffer de lecture de l'entete */
  short i;
  unsigned short byteswrite;
  unsigned short header = 64;             /* Nbre de bytes entete du fichier */
  short lencomm;

  fic = open(NomFic,O_WRONLY | O_CREAT | O_TRUNC,PROTECT_NORMAL);
  if (fic == -1) return(-1);

  BufferEntete[0] = 0x494D;
  lencomm = strlen(comment);
  BufferEntete[1] = _mw_get_flip_b2(lencomm);
  BufferEntete[2] = _mw_get_flip_b2(dx);
  BufferEntete[3] = _mw_get_flip_b2(dy);
  for (i=4;i<32; i++) BufferEntete[i] = 0;
  memcpy(&BufferEntete[32],comment,lencomm);
  byteswrite = write(fic,BufferEntete,header+lencomm);
  close(fic);
  if (byteswrite != header+lencomm) return(-1);
  return(0);
}
 

/*--- Cree l'entete d'un fichier <NomFic> au format Inrimage, ayant    ---*/
/*--- pour dimension d'image <dx>,<dy>.                                ---*/
/* Retourne 0 si OK, -1 si creation impossible ou erreur d'ecriture   */
#ifdef __STDC__
static short MAKE_INR_HEADER(char *NomFic, short dx, short dy)
#else
static short MAKE_INR_HEADER(NomFic,dx,dy) 

char *NomFic;
short dx,dy;
#endif
{ short fic;
  unsigned short BufferEntete[128];       /* Buffer de lecture de l'entete */
  unsigned short byteswrite;
  unsigned short header = 256;            /* Nbre de bytes entete du fichier */
  unsigned short i;

  fic = open(NomFic,O_WRONLY | O_CREAT | O_TRUNC, PROTECT_NORMAL);
  if (fic == -1) return(-1);

  for (i=0;i<header/2;i++) BufferEntete[i] = 0;

  BufferEntete[0] = BufferEntete[5] = dx;
  BufferEntete[1] = dy;
  BufferEntete[2] = 256;
  BufferEntete[3] = BufferEntete[4] = 1;
  BufferEntete[12] = 200;

  byteswrite = write(fic,BufferEntete,header);
  close(fic);
  if (byteswrite != header) return(-1);
  return(0);
}

/*--- Cree l'entete d'un fichier <NomFic> au format MultImage, ayant   ---*/
/*--- pour dimension d'image <dx>,<dy>.                                ---*/
/* Retourne 0 si OK, -1 si creation impossible ou erreur d'ecriture   */

#ifdef __STDC__
static short MAKE_MTI_HEADER(char *NomFic, short dx, short dy)
#else
static short MAKE_MTI_HEADER(NomFic,dx,dy) 
char *NomFic;
short dx,dy;
#endif
{ short fic;
  unsigned short BufferEntete[4];       /* Buffer de lecture de l'entete */
  unsigned short byteswrite;
  unsigned short header = 8;             /* Nbre de bytes entete du fichier */

  fic = open(NomFic,O_WRONLY | O_CREAT | O_TRUNC, PROTECT_NORMAL);
  if (fic == -1) return(-1);


  BufferEntete[0] = dx;
  BufferEntete[1] = dy;
  BufferEntete[2] = 8;
  BufferEntete[3] = 0;
  byteswrite = write(fic,BufferEntete,header);
  close(fic);
  if (byteswrite != header) return(-1);
  return(0);
}
 


Cimage _mw_cimage_load_megawave1(NomFic,Type)

char  *NomFic;                        /* Nom du fichier image */
char  *Type;                          /* Type de format du fichier */

{ short   fic;                            /* fichier */
  unsigned short BufferEntete[4];         /* Buffer de lecture de l'entete */
  unsigned short bytesread;               /* Nbre de bytes lus */
  unsigned short minheader = 8; /* Nbre de bytes entete minimale du fichier */
  unsigned short header;        /* Nbre de bytes entete du format du fichier */
  long  TailleBuffer;                   /* Taille du Buffer en octets */
  unsigned short dx,dy;                 /* Taille de l'image du fichier */
  unsigned short taillezc;              /* Taille de la zone de commentaires */
  unsigned short n;                       /* Nbre de bytes a lire */
  long i,L;                               /* Indices courants sur l'image */
  double sqrt_fsize;

  Cimage image;                /* Buffer memoire */
  char  Comment[mw_cmtsize];   /* Commentaires */
  unsigned char *ptr;

  image = NULL;
  strcpy(Type,"?");  /* Type a priori inconnu */

       /* Ouverture du fichier */

  fic = open(NomFic, /* O_BINARY | */ O_RDONLY);
  if (fic == -1) mwerror(FATAL, 0,"File \"%s\" not found or unreadable\n",NomFic);

         /* Lecture entete */
  bytesread = read(fic,BufferEntete,minheader);
  if (bytesread != minheader) 
    mwerror(FATAL, 0,"Error while reading header of file \"%s\" (may be corrupted)\n",NomFic);

         /* Tests entete */

 if ((bytesread == minheader) && 
     ((BufferEntete[0] == 0x494D)||(BufferEntete[0] == 0x4D49)))
    {  /* IMG */
      strcpy(Type,"IMG");
      if (BufferEntete[0] == 0x494D)
	{
	  taillezc = _mw_get_flip_b2(BufferEntete[1]);
	  dx = _mw_get_flip_b2(BufferEntete[2]);
	  dy = _mw_get_flip_b2(BufferEntete[3]);
	}
      else
	{
	  taillezc = BufferEntete[1];
	  dx = BufferEntete[2];
	  dy = BufferEntete[3];
	}
      header=64+taillezc;

      /* Read the comments */
      if (taillezc > 0)
	{
	  if (lseek(fic,64,L_SET) == -1L) 
	    mwerror(FATAL, 0,"IMG image header file \"%s\" is corrupted\n",NomFic);
	  bytesread = read(fic,Comment,taillezc);
	  if (bytesread != taillezc) 
	    mwerror(FATAL, 0,"IMG image header file \"%s\" is corrupted\n",NomFic);
	  Comment[taillezc] = '\0';
	}
      else Comment[0] = '\0';
    }

 else
   if ((BufferEntete[3] == 1) && (BufferEntete[2] == 256))
     {  /* INR */
       strcpy(Type,"INR");
       dx = BufferEntete[0];
       dy = BufferEntete[1];
       header=256;
       Comment[0] = '\0';
     }

   else
     if ((BufferEntete[2] == 8) && (BufferEntete[3] == 0))
       {  /* MTI */
	 strcpy(Type,"MTI");
	 dx = BufferEntete[0];
	 dy = BufferEntete[1];
	 header=8;
	 Comment[0] = '\0';
       }
     else 
       {  /* May be BIN */
	 sqrt_fsize = sqrt((double) fsize(fic));
	 L = (int) sqrt_fsize;
	 if ( ((double) L - sqrt_fsize) == 0.0 ) /* Size is a square */
	   {
	     /* Assume to be BIN */
	     strcpy(Type,"BIN");
	     dx = dy = L;
	     header = 0;
	     Comment[0] = '\0';
	     mwerror(WARNING,0,
		     "Unrecognized header for the file \"%s\": assume BINary 8 bpp format.\n",NomFic);
	   }
	 else
	   mwerror(FATAL, 0,"Format of the image file \"%s\" unknown\n",NomFic);
       }

  /* Reservation memoire */

  image = mw_change_cimage(NULL,dy,dx);
  if (image == NULL)
    mwerror(FATAL,0,"Not enough memory to load the image \"%s\"\n",NomFic);

  strcpy(image->cmt,Comment);

  /* On se positionne sur le debut de la zone pixel (0,0) */

  if (lseek(fic,header,L_SET) == -1L) 
    {
      mw_delete_cimage(image);
      image = NULL;
      mwerror(ERROR, 0,"Format of the image file \"%s\" unknown or file corrupted\n",NomFic);
      return(image);
    }

  ptr = image->gray;
  i = 0;
  L = ( (long) dx ) * dy;
  while (L>0) {
      if (L > 65000L) n = 65000; else n = L;
      L -= n;
      bytesread = read(fic,ptr+i,n);
      if (bytesread != n) 
	{
	  mw_delete_cimage(image);
	  image = NULL;
	  mwerror(ERROR, 0,"Error while reading file \"%s\" (file may be corrupted)\n",NomFic); 
	  return(image);
	}
      i += bytesread;
    }

  close(fic);

  _mw_flip_image((unsigned char *) ptr,sizeof(char),dx,dy, FALSE);
  return(image);
}

short _mw_cimage_create_megawave1(NomFic,image,Type)

     char  *NomFic;                        /* Nom du fichier image */
     Cimage image;
     char  *Type;                          /* Type de format du fichier */

{
  int   fic;                            /* fichier */
  unsigned short byteswrite;            /* Nbre de bytes ecrits */
  unsigned short header;                /* Nbre de bytes entete du fichier */
  unsigned short dx,dy;                          /* Taille de l'image */
  unsigned short taillezc;              /* Taille de la zone de commentaires */
  unsigned short n;                     /* Nbre de bytes a lire */
  long i,l;                             /* Indices courants sur l'image */
  unsigned char *ptr; 

  if ((image->ncol>=65536)||(image->nrow>=65536))
    {
      mwerror(FATAL,1,"Image too big to be saved using %s external type !\n",
	      Type);
      return(-1);
    }
  dx = image->ncol;
  dy = image->nrow;

  if (Type[0] == '?') _mw_put_range_type(Type,"cimage",1);
  if (strcmp(Type,"IMG") == 0) 
    {
      if (_MAKEHEADER_IMG(NomFic,dx,dy,image->cmt) != 0) 
	{
	  mwerror(FATAL,1,"Unable to write in the file \"%s\"\n",NomFic);
	  return(-1);
	}
      taillezc = strlen(image->cmt);
      header = 64;
    }
  else if (strcmp(Type,"INR") == 0) 
    {
      if (MAKE_INR_HEADER(NomFic,dx,dy) != 0) 
	{
	  mwerror(FATAL,1,"Unable to write in the file \"%s\"\n",NomFic);
	  return(-1);
	}
      taillezc = 0;
      header = 256;
    }
  else if (strcmp(Type,"MTI") == 0) 
    {
      if (MAKE_MTI_HEADER(NomFic,dx,dy) != 0) 
	{
	  mwerror(FATAL,1,"Unable to write in the file \"%s\"\n",NomFic);
	  return(-1);
	}
      taillezc = 0;
      header = 8;
    }
  else if (strcmp(Type,"BIN") == 0)
    {
      taillezc = 0;
      header = 0;
    }
  else
    mwerror(INTERNAL,1,"[_mw_cimage_create_megawave1] Unknown format \"%s\"\n",
	    Type);

  if (taillezc + header <= 0) /* No header at all */
    {
      if ((fic=open(NomFic,O_WRONLY | O_CREAT | O_TRUNC, 
		    PROTECT_NORMAL)) == -1)
	mwerror(FATAL,1,"Unable to create the file \"%s\"\n",NomFic);
    }
  else
    /* Header already written */
    {
      if ( ((fic = open(NomFic,O_WRONLY)) == -1) || 
	(lseek(fic,(long)taillezc+header,L_SET) != taillezc+header) )
	mwerror(FATAL,1,"Unable to write in the file \"%s\"\n",NomFic);
    }

  l = ((long) dx) * dy;
  
  i=0;
  ptr = image->gray;
  while (l>0) 
    {
      if (l > 65000L) n = 65000; else n = l;
      l -= n;
      byteswrite = write(fic,ptr+i,n);
      if (byteswrite != n) 
	{
	  mwerror(FATAL,1,"Error while writing in the file \"%s\"\n",NomFic);
	  return(-1);
	}
      i += n;
  }
  close(fic);
  return(0);
}


/*~~~~~~ MegaWave2 formats ~~~~~*/

/* Native formats (without conversion of the internal type) */

/* Return != NULL if load OK */

Cimage _mw_cimage_load_native(NomFic,Type)

char  *NomFic;                        /* Nom du fichier image */
char  *Type;                          /* Type de format du fichier */

{
  if ((strcmp(Type,"IMG") == 0) || (Type[0] == '?'))
    /* Includes INR,MTI and BIN types (not supported by _mw_get_file_type()) */
    return(_mw_cimage_load_megawave1(NomFic,Type));

  if (strcmp(Type,"TIFF") == 0)
     /* TIFF format with 1 gray-level plane */
    return((Cimage) _mw_cimage_load_tiff(NomFic));

  if (strcmp(Type,"PM_C") == 0)
    /* PM format with pm_form=PM_C and pm_np = pm_nband = 1 */
    return((Cimage) _mw_cimage_load_pm(NomFic));

  if (strcmp(Type,"GIF") == 0)
    /* GIF87 format with a conversion of colors to grey scales */
    return((Cimage) _mw_cimage_load_gif(NomFic));

  if (strcmp(Type,"BMP") == 0)
    /* BMP format with 8 bpp */
    return((Cimage) _mw_cimage_load_bmp(NomFic));

  if (strcmp(Type,"PS") == 0)
    /* Read PostScript file */
    return((Cimage) _mw_cimage_load_ps(NomFic));

  if (strcmp(Type,"EPSF") == 0)
    /* Read Encapsulated PostScript file */
    return((Cimage) _mw_cimage_load_epsf(NomFic));

  if (strcmp(Type,"PGMA") == 0)
    /* Read PGM Ascii file */
    return((Cimage) _mw_cimage_load_pgma(NomFic));

  if (strcmp(Type,"PGMR") == 0)
    /* Read PGM Raw bits file */
    return((Cimage) _mw_cimage_load_pgmr(NomFic));

  if (strcmp(Type,"JFIF") == 0)
    /* JPEG/JFIF format with 8-bit gray level plane */
    return((Cimage) _mw_cimage_load_jpeg(NomFic));

  return(NULL);
}


/* Return 0 if create OK */

short _mw_cimage_create_native(NomFic,image,Type)

     char  *NomFic;                        /* Nom du fichier image */
     Cimage image;
     char  *Type;                          /* Type de format du fichier */

{
  if ((_mw_is_of_ftype(Type,"IMG")) || (_mw_is_of_ftype(Type,"INR")) || 
      (_mw_is_of_ftype(Type,"MTI")) || (_mw_is_of_ftype(Type,"BIN")))
    {
      if ((image->ncol>=65536)||(image->nrow>=65536))
	{
	  mwerror(WARNING,1,"Image too big to be saved using %s external type. Using PM_C...\n",Type);
	  strcpy(Type,"PM_C");
	}
      else return(_mw_cimage_create_megawave1(NomFic,image,Type));
    }

  if (_mw_is_of_ftype(Type,"GIF"))
    /* Write GIF87 file with a grey scales colormap */
    {
      if ((image->ncol>=65536)||(image->nrow>=65536))
	{
	  mwerror(WARNING,1,"Image too big to be saved using %s external type. Using PM_C...\n",Type);
	  strcpy(Type,"PM_C");
	}
      else return(_mw_cimage_create_gif(NomFic,image));
    }


  if (_mw_is_of_ftype(Type,"TIFF"))
     /* TIFF format with 1 gray-level plane */
    return(_mw_cimage_create_tiff(NomFic,image));

  if (_mw_is_of_ftype(Type,"BMP"))
     /* BMP format with 8 bpp */
    return(_mw_cimage_create_bmp(NomFic,image));

  if (_mw_is_of_ftype(Type,"PM_C"))
    /* PM format with pm_form=PM_C and pm_np = pm_nband = 1 */
    return(_mw_cimage_create_pm(NomFic,image));

  if (_mw_is_of_ftype(Type,"PS"))
    /* Write PostScript file */
    return(_mw_cimage_create_ps(NomFic,image));

  if (_mw_is_of_ftype(Type,"EPSF"))
    /* Write Encapsulated PostScript file */
    return(_mw_cimage_create_epsf(NomFic,image));

  if (_mw_is_of_ftype(Type,"PGMA"))
    /* Write PGM Ascii file */
    return(_mw_cimage_create_pgma(NomFic,image));

  if (_mw_is_of_ftype(Type,"PGMR"))
    /* Write PGM Raw bits file */
    return(_mw_cimage_create_pgmr(NomFic,image));

  if (_mw_is_of_ftype(Type,"JFIF"))
     /* JPEG/JFIF format with 1 gray-level plane */
    return(_mw_cimage_create_jpeg(NomFic,image,_mw_get_ftype_opt(Type)));
  
  return(-1);
}

/* All available formats */

Cimage _mw_cimage_load_image(NomFic,Type)

char  *NomFic;                        /* Nom du fichier image */
char  *Type;                          /* Type de format du fichier */

{ 
  Cimage im;
  char mtype[mw_mtype_size];
  int hsize;  /* Size of the header, in bytes */
  float version;/* Version number of the file format */

  _mw_get_file_type(NomFic,Type,mtype,&hsize,&version);

  /* First, try native formats */
  im = _mw_cimage_load_native(NomFic,Type);
  if (im != NULL) return(im);

  /* If failed, try other formats with memory conversion */
  im = (Cimage) _mw_load_etype_to_itype(NomFic,mtype,"cimage",Type);
  if (im != NULL) return(im);

  if (Type[0]=='?')
    mwerror(FATAL, 1,"Unknown external type for the file \"%s\"\n",NomFic);
  else
    mwerror(FATAL, 1,"External type of file \"%s\" is %s. I Don't know how to load such external type into a Cimage !\n",NomFic,Type);
}


short _mw_cimage_create_image(NomFic,image,Type)

char  *NomFic;                        /* Nom du fichier image */
Cimage image;
char  *Type;                          /* Type de format du fichier */

{
  short ret;

  /* First, try native formats */
  ret = _mw_cimage_create_native(NomFic,image,Type);
  if (ret == 0) return(0);

  /* If failed, try other formats with memory conversion */
  ret = _mw_create_etype_from_itype(NomFic,image,"cimage",Type);
  if (ret == 0) return(0);

  /* Invalid Type should have been detected before, but we can arrive here because
     of a write failure (e.g. the output file name is write protected).
  */
  mwerror(FATAL, 1,"Cannot save \"%s\" : all write procedures failed !\n",NomFic);  
}

