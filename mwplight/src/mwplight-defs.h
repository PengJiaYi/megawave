/**
 * @file mwplight-defs.h
 *
 * common definitions for mwplight
 *
 * @author Jacques Froment <jacques.froment@univ-ubs.fr> (2005), \
 *         Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr> (2008)
 */

#ifndef _MWPLIGHT_DEFS_H
#define _MWPLIGHT_DEFS_H

/* FIXME: drop */
#include <sys/types.h>
#include <sys/stat.h>

#include "mwplight-config.h"

/*
 * CUSTOM TYPES/STRUCTURES
 */

/* an argument of a statement */
typedef struct s_argument
{
     int Atype;                  /* argument type (OPTION,NEEDEDARG,...)   */
     int IOtype;                 /* I/O type (READ, WRITE)                 */
     int ICtype;                 /* interval checking type (CLOSED, ...)   */

     char Flag;                  /* flag of the option ('c')               */
     char H_id[TREESTRSIZE];     /* name of the argument for Human being   */
     char C_id[TREESTRSIZE];     /* name of the argument in C body         */
     char Cmt[TREESTRSIZE];      /* comment's string                       */

     char Val[TREESTRSIZE];      /* default input value                    */
     char Min[TREESTRSIZE];      /* min value in case of interval checking */
     char Max[TREESTRSIZE];      /* man value in case of interval checking */

     struct s_variable * var;    /* pointer to the corresponding variable
				  * in the main function (t_varfunc)       */

     struct s_argument * previous;     /* pointer to the previous argument */
     struct s_argument * next;         /* pointer to the next argument     */
} t_argument;

typedef struct s_header
{
     char Name[TREESTRSIZE];     /* name of the C module
				  * = name of the main function            */
     char Author[TREESTRSIZE];   /* list of author(s)                      */
     char Version[TREESTRSIZE];  /* version number                         */
     char Function[TREESTRSIZE]; /* explain the function of the module     */
     char Labo[TREESTRSIZE];     /* name of the lab (optional)             */
     char Group[TREESTRSIZE];    /* group name (optional)                  */
     t_argument *usage;          /* pointer on the first argument in usage */
     t_argument *retmod;         /* pointer to the argument of
				  * the return main function, if any       */
     int NbOption;               /* number of option                       */
     int NbNeededArg;            /* number of needed arguments             */
     int NbVarArg;               /* number of variable arguments           */
     int NbOptionArg;            /* number of optional arguments           */
     int NbNotUsedArg;           /* number of notused arguments            */
} t_header;

/* a variable */
typedef struct s_variable
{
     char Name[TREESTRSIZE];     /* name of the variable in C body         */
     int Ctype;                  /* category of scalar C type of name
				  * (SHORT_T, MW2_T, ...)                  */
     char Stype [TREESTRSIZE];   /* type of the parameter in C body.
				  * examples : unsigned char, Cimage.
				  * beware : pointer is not indicated here
				  * v(if v is declared by char *v;
				  * the type is reported as char
				  * and not char *.
				  * Use PtrDepth to now
				  * that the full type is char *.)          */
     char Ftype [TREESTRSIZE];   /* (full) Type of the parameter in C body.
				  * examples : unsigned char *, Cimage *.   */
     int PtrDepth;               /* pointer depth (0 if not a pointer, 1
				  * if e.g. char *, 2 if e.g. char **).
				  * beware : do not count when a type is
				  * already a pointer (such as a megawave
				  * structure  or a function) : e.g. 0 if
				  * Cimage, 1 if Cimage *,...               */
     int DeclType;               /* variable declaration type in A-file.
				  * See DT_* constants.                     */
     char Cstorage[TREESTRSIZE]; /* C storage, if any                       */

     struct s_argument * arg;    /* variable of the main function only :
				  * pointer to the corresponding argument
				  * in the header                           */

     struct s_variable * previous; /* pointer to the previous variable       */
     struct s_variable * next;     /* pointer to the next variable           */
} t_variable;

/* a variable or a function */
typedef struct s_varfunc
{
     int Itype;                  /* instruction type                        */
     t_variable * v;             /* description of the variable or function */
     t_variable * param;         /* pointer on the first parameter
				  * (NULL if a variable)                    */
     long l0;                    /* location of the beginning of the
				  * varfunc declaration in the input module
				  * file                                    */
     long l1;                    /* location of the end of the varfunc
				  * declaration                             */

     struct s_varfunc *previous; /* pointer to the previous varfunc         */
     struct s_varfunc *next;     /* pointer to the next varfunc             */

} t_varfunc;


/* C body description of the module */
typedef struct s_body
{
     t_varfunc * varfunc;        /* Pointer on the first variable or function */
     t_varfunc * mfunc;          /* Pointer on the module's function          */
} t_body;

/*
 * a C word
 * Used during parsing of C body while exact meaning is not yet determined.
 */
typedef struct s_token
{
     char Name[TREESTRSIZE];     /* name (the word itself)                  */
     int Wtype;                  /* word type                               */

     struct s_token * previous;  /* pointer to the previous t_token         */
     struct s_token * next;      /* pointer to the next t_token             */
} t_token;

/* a C instruction (until next ;) as obtained during parsing of C body */
typedef struct s_statement
{
     char phrase[STRSIZE]; /* the instruction text                          */
     int Itype;            /* instruction type                              */
     int nparam;           /* number of parameters in the function
			    * (-1 if no function)                           */
     int ndatatype;        /* number of data type in the function
			    * (-1 if no function)                           */
     int nvar;             /* number of declared variables                  */
     t_token * wfirst;     /* pointer of the first word in the instruction  */

     /*
      * the following NOT NULL only if the instruction has been
      *falsely separated by getinstruction() and has to be continued.
      */
     struct s_statement * previous;
     struct s_statement * next;  /* pointer to the next instruction
				  * to be continued                       */

} t_statement;


/*
 * GLOBAL (EXTERN) VARIABLES
 */

FILE * source_file_global;

int debug_flag;
t_header * H;
t_body * C;
struct stat module_fstat;

/* 
 * - 1 if parsing inside a comment, 0 elsewhere
 * - 0 if inside C body, > 0 if inside header
 *     (the number being the ID number of the header)
 * - 1 if parsing optional arguments in the usage but the last one;
 *   2 if parsing the last one;
 *   0 if not yet inside optional arguments;
 *   -1 if no more inside but one list was encountered.
 */
int inside_comment;
int inside_header;
int inside_optionarg;

/*
 * names of
 * - the input module file (without path or extension)
 * - the group (from the current working dir.)
 */
char * module_name;
char * group_name;

/*
 * buffers storing
 * - the module's usage
 * - the module's main function prototype (in K&R format)
 */
char usagebuf[STRSIZE];
char protobuf[STRSIZE];

/*
 * MACROS
 */

/*
 * t_argument
 */

/*
 * is the arg
 * - an option
 * - a "FLAGARG", i.e. a flag option
 * - a needed argument
 * - a variable argument
 * - an optional argument
 * - a notused argument
 */
#define ISARG_OPTION(x)   ((x)->Atype == OPTION)
#define ISARG_FLAGOPT(x)  (((x)->Atype == OPTION) && ((x)->H_id[0] == '\0'))
#define ISARG_NEEDED(x)   ((x)->Atype == NEEDEDARG)
#define ISARG_VARIABLE(x) ((x)->Atype == VARARG)
#define ISARG_OPTARG(x)   ((x)->Atype == OPTIONARG)
#define ISARG_NOTUSED(x)  ((x)->Atype == NOTUSEDARG)

/*
 * is the arg
 * - an input
 * - an output
 */
#define ISARG_INPUT(x)  ((x)->IOtype == READ)
#define ISARG_OUTPUT(x) ((x)->IOtype == WRITE)

/*
 * does the arg haave
 * - a default value
 * - an interfal checking
 */
#define ISARG_DEFAULT(x)  ((x)->Val[0] != '\0')
#define ISARG_INTERVAL(x) ((x)->ICtype != NONE)

/*
 * is the arg
 * - of an implicit pointer type (such as char * or Cimage)
 *   -> may be set to NULL.
 *   (does not work with user's types that are pointer to something)
 * - of an explicit pointer type (such as char ** but not Cimage)
 *   -> may be set to NULL.
 */
#define ISARG_IMPLICITPOINTER(x) (((x)->var->Ctype == MW2_T) ||		\
				  ((x)->var->Ctype == QSTRING_T) ||	\
				  ((x)->var->PtrDepth > 0))
#define ISARG_EXPLICITPOINTER(x) ((x)->var->PtrDepth > 0)

/*
 * is the arg
 * - a "SCALARARG", i.e. of scalar type (such as float)
 * - a "SCALARARG" and not a flag
 * - of type pointer to a scalar (such as char *)
 * - a "FILEARG" (i.e. of megawave internal type)
 * - of type pointer to a megawave  internal type (such as Cimage *)
 * - the return function of the module
 */
#define ISARG_SCALAR(x)        (((x)->var->Ctype >= SCALARMIN_T) &&	\
				((x)->var->Ctype <= SCALARMAX_T))
#define ISARG_SCALARNOTFLAG(x) ((ISARG_SCALAR(x)) && (!(ISARG_FLAGOPT(x))))
#define ISARG_POINTERSCALAR(x) ((ISARG_SCALAR(x)) && ((x)->var->PtrDepth == 1))
#define ISARG_FILE(x)          ((x)->var->Ctype == MW2_T)
#define ISARG_POINTERFILE(x)   ((ISARG_FILE(x)) && ((x)->var->PtrDepth == 1))
#define ISARG_RETURNFUNC(x)    ((x) == H->retmod)

/*
 * t_token
 */

/*
 * is the cword a data type
 * does the  cword participate to
 * - the 'scalar' data type (i.e. not explicitely a pointer)
 * - the full data type
 */
#define ISCWORD_TYPE(x)       (((x)->Wtype == W_DATATYPE) ||		\
                               ((x)->Wtype == W_USERDATATYPE) ||	\
			       ((x)->Wtype == W_CMWDATATYPE))
#define ISCWORD_SCALARTYPE(x) ((ISCWORD_TYPE(x)) ||		\
			       ((x)->Wtype == W_CMODIFIER))
#define ISCWORD_FULLTYPE(x)   ((ISCWORD_SCALARTYPE(x)) ||	\
			       ((x)->Wtype == W_CPOINTER) )

/*
 * t_statement
 */

/*
 * is the cinstruction
 * - a function declaration
 * - a function prototype
 * - a function
 */
#define ISCI_FUNCDECL(x)  (((x)->Itype == I_FUNCDECL_ANSI) ||	\
			   ((x)->Itype == I_FUNCDECL_KR))
#define ISCI_FUNCPROTO(x) (((x)->Itype == I_FUNCPROTO_ANSI) ||	\
			   ((x)->Itype == I_FUNCPROTO_KR))
#define ISCI_FUNCTION(x)  (ISCI_FUNCDECL(x) || ISCI_FUNCPROTO(x))

#endif /* !_MWPLIGHT_DEFS_H */
