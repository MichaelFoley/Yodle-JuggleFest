/*
 *	Michael Foley, jugglefest circuit assignment easy version
 *
 *	it's easier to have the makefile rip the circuit and juggler information into 
 *	structure array initializations	than to write the parser for the file.
 *	I can also have the Makefile rip the 3 circuit 12 juggler sample for testing.
 *	I'm assuming someone at Yodle wrote the program to generate the output in the sample.
 *	if that's a bad assumption then I have no way to validate my answers.
 *	Another assumption, the LEAST popular circuits must still be one of the preferences of C/J jugglers 
 *
 *	it's working, hope it's the right answer 28762@yodle.com here I come
 *
 *	double checking, it has many failures, which I'll also submit.
 *
 *	Leaving debugging printf statements, #defining them out with OBSOLETE
 *
 *	when you run, redirect stderr to /dev/null or a file, it's not empty, there are errors
 *
 *	After bouncing the problem with assignment failures off Sam we've decided the assignment
 *	failure message is inaccurate, only the un-line-numbered message is true, the others are
 *	successful reassignments.
 * 	
 *	My code also currently fails to properly account for ties in the dot product when recurring.
 *	That will now be rectified
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

static int verbose[256]={0};	/* verbose modes off by default, turn on with -v */
static int silence[256]={0};	/* if non zero DISABLES some feature that is on by default, -s arg */
static int counter[256]={0};	/* counts of verbose message occurences */

struct Circuit {
	char *name;
	unsigned short h,e,p;
	int preferred,jugglers;
	union ToSort {
		unsigned long long ull;
		struct {
			unsigned short used,juggler,weight,dot_product;
			} us;
		} *uTS;
	/* pointer to allocated juggler/dot_product array, sized by jugglers/circuits */
	} 
	circuit[] = {
#ifdef LIL
#include "lil_circuits.c"
#else	/* LIL */
#ifdef BIG
#include "big_circuits.c"
#else	/* BIG */
#include "new_circuits.c"	/* if not lil example or big test data, better tweak Makefile */
#endif	/* BIG */
#endif	/* LIL */
	};
 
#ifndef PREFERENCES
#define PREFERENCES 11		/* we only need 10+1 for null for test data */
#endif /* PREFERENCES */
struct Juggler {
	char *name;		/* name has to be the first item in struct for initialization */
	unsigned short h,e,p;	/* h,e,p has to be the second,third,fourth itemin struct for initialization */
	char *prefstring[PREFERENCES];	/* prefstring has to be the fifth item in struct for initialization */
	short pindex;	/* index into preference array for next attempted circuit assignment */
	} 
	juggler[] = {
#ifdef LIL
#include "lil_jugglers.c"
#else	/* LIL */
#ifdef BIG
#include "big_jugglers.c"
#else	/* BIG */
#include "new_jugglers.c"	/* if not lil example or big test data, better tweak Makefile */
#endif	/* BIG */
#endif	/* LIL */
	};

#define JUGGLERS (sizeof(juggler)/sizeof(juggler[0]))

#define Dot_Product(c,j) (c.h*j.h+c.e*j.e+c.p*j.p)

static int
qsort_uTS(const void *a, const void *b)
	{
	union ToSort *uTSa,*uTSb;
	int rc;
	uTSa=(union ToSort *)a;
	uTSb=(union ToSort *)b;
#ifndef OBSOLETE
	if (uTSa->ull<uTSb->ull)
	   rc=1;
	  else
	   if (uTSa->ull>uTSb->ull)
	      rc=-1;
	     else
	      rc=0;
#else /* OBSOLETE */
	if (uTSa->us.dot_product<uTSb->us.dot_product)
	   rc=1;
	  else
	   if (uTSa->us.dot_product>uTSb->us.dot_product)
	      rc=-1;
	     else
	      rc=0;
#endif /* OBSOLETE */
	return(rc);
	}

static short JperC;	/* Jugglers per Circuit, calculated from array sizes in main() */

static short maxpref=0;	/* maximum preference index */
static FILE *output=NULL;	/* so I can hijack stderr calls */

static int
assign_juggler(unsigned short ji,unsigned int depth)
	{
	unsigned int calls=0;
	unsigned short ties;			/* iterations through assignment/reassignment loop */
	unsigned short *weight=NULL; 		/* need to keep weights at each level of recursion */
	unsigned short ci,reassign,rc;		/* circuit index, reassign this juggler, return code */
	int i;	/* short term index variables */
	calls++;	/* counting recursions */
	if (juggler[ji].pindex>maxpref)
	   maxpref=juggler[ji].pindex;
	if (juggler[ji].prefstring[juggler[ji].pindex]==NULL)
	   {
	/* TODO do we report this here or after we return from recursion, if we make it to top is true failure ? */
	   if (output && (verbose['f']))
	      fprintf(output,"%5u: %u -vf Juggler[%hu] %s preference list exhausted, festival failure\n", __LINE__,
			++counter['f'],ji,juggler[ji].name);
	   return(1);	/* failure to assign */
	   }
	ci=(unsigned short)atoi(&juggler[ji].prefstring[juggler[ji].pindex][1]);
	if (circuit[ci].uTS==NULL)	/* first visit to this circuit */
	   {
	   if ((circuit[ci].uTS=calloc(sizeof(union ToSort),JperC+1))==NULL)
	      return(output?fprintf(output,"%5u: unable to allocate union circuit[%d].u[X=%d] array \n",
				__LINE__,ci,JperC+1):__LINE__);	/* not -v, important failure */
	   }

#define ShowAssignments(line,trigger,label) \
	if ((output) && (verbose[trigger])) \
	   { \
	   fprintf(output,"%5u: %5u -v%c %5u %2u %-5s %-6s @%s ", \
			line,++counter[trigger],trigger,calls,depth,circuit[ci].name,juggler[ji].name,label); \
	   for (i=0;i<=JperC;i++) \
		fprintf(output,"%u:%04hu:%1hu:%05hu ",i, \
			circuit[ci].uTS[i].us.dot_product, \
			circuit[ci].uTS[i].us.weight, \
			circuit[ci].uTS[i].us.juggler, \
			circuit[ci].uTS[i].us.used); \
	   fprintf(output,"\n"); \
	   }
	circuit[ci].uTS[JperC].us.dot_product=Dot_Product(circuit[ci],juggler[ji]);
	circuit[ci].uTS[JperC].us.juggler=ji;
	circuit[ci].uTS[JperC].us.used=0xabba;
	for (i=0;i<=JperC;i++)
	    circuit[ci].uTS[i].us.weight=0;	/* before first qsort at this level of recursion all weights are 0 */
	if ((output) && (verbose['a']))
	   fprintf(output,"%5u: %u -va Circuit %s added %s:%hu\n",__LINE__,++counter['a'],
			circuit[ci].name,juggler[ji].name,circuit[ci].uTS[JperC].us.dot_product);
	ShowAssignments(__LINE__,'1',"entry")
	for (rc=0,ties=0;;ties++)	/* by default we expect to succeed */
	    {
	    qsort(circuit[ci].uTS,JperC+1,sizeof(circuit[ci].uTS[0]),qsort_uTS); 
	    if (circuit[ci].uTS[JperC].us.weight)	/* if true all lowest dp jugglers tried */
	       {
	       for (i=0;i<=JperC;i++)				/* run through this circuits assignments */
		   if (circuit[ci].uTS[i].us.juggler==ji) 	/* assigning juggler ji will have been the only change */
		      circuit[ci].uTS[i].ull=0;		/* remove juggler ji from circuit ci, restoring to original list */
	       qsort(circuit[ci].uTS,JperC+1,sizeof(circuit[ci].uTS[0]),qsort_uTS); 	/* restore list to original order */
	       juggler[ji].pindex++;			/* try next circuit preference */
	       if (assign_juggler(ji,depth+1))
	          rc=__LINE__;	/* tell previous recursion to try again, only failure case for recursion */
		 else
		  rc=0;		/* succeeded in assigning him further down his preference list */
	       ShowAssignments(__LINE__,'2',"exit ")
	       break;		/* out of for (;;) loop */
	       }
	    if (ties)
		counter['t']++;
	    if (circuit[ci].uTS[JperC].ull!=0)	/* if we have a juggler to reassign */
	       {
	       if ((output) && (verbose['r']))
	          fprintf(output,"%5u: %u -vr Circuit %s overfull, reassigning %s:%hu\n",__LINE__,++counter['r'],
			circuit[ci].name,juggler[circuit[ci].uTS[JperC].us.juggler].name,circuit[ci].uTS[JperC].us.dot_product);
	       if (weight==NULL)
		  if ((weight=calloc(sizeof(weight[0]),JperC+1))==NULL)		/* need JperC array at each recursion level */
		     return(fprintf(stderr,"%5u: Unable to calloc unsigned short weight[%u] array during reassignment\n",__LINE__,JperC+1));
				/* above error may need hook to only print once no matter how many times hit */
				/* although if I'm out of heap that instance of the program is done anyway */
	       for (i=0;i<=JperC;i++)
		   weight[i]=circuit[ci].uTS[i].us.weight;
	       reassign=circuit[ci].uTS[JperC].us.juggler;	/* done with arg juggler, reuse */
	       circuit[ci].uTS[JperC].ull=0;
	       juggler[reassign].pindex++;			/* try next circuit preference */
	       if (ties)
	          ShowAssignments(__LINE__,'3',"ties")
	       if (assign_juggler(reassign,depth+1))		/* non zero return is failure to reassign */
		  {	/* failure to reassign juggler */
	          ShowAssignments(__LINE__,'4',"faild")
		  circuit[ci].uTS[JperC].us.dot_product=Dot_Product(circuit[ci],juggler[reassign]);
		  circuit[ci].uTS[JperC].us.weight=1;		/* this juggler be further up qsorted list for same dot_product */
		  circuit[ci].uTS[JperC].us.juggler=reassign;
		  circuit[ci].uTS[JperC].us.used=0xabba;
		  juggler[reassign].pindex--;			/* failed assignment, double check this decrements properly */
		  if (output && verbose['w'])
		       {
		       fprintf(output, "%5u: %u -vw %s %s dp=%hu weight=1 (%s %hx)\n" ,__LINE__,++counter['w'],
				circuit[ci].name,
				juggler[circuit[ci].uTS[JperC].us.juggler].name,
				circuit[ci].uTS[JperC].us.dot_product, /* weight=1 */
				juggler[circuit[ci].uTS[JperC-1].us.juggler].name,
				circuit[ci].uTS[JperC-1].us.dot_product);
		       }
		  /* for (rc=0;;) loop will iterate here */
		  }	/* end of failure to reassign juggler */
	         else	/* if assign_juggler(reassign) succeeds */
		  {
		  circuit[ci].uTS[JperC].ull=0;	/* only required so array looks good in debugging */
	          break;	/* out of for(;;) loop, rc should still be =0 */
		  }
	       ShowAssignments(__LINE__,'5',"iter8")
	       /* loop will iterate here */
	       }	/* end of last slot not empty logic */
	      else	/* list not overfull, no need to reassign */
	       break;	/* out of for(;;) loop, rc should still be =0 */
	   }	/* end of for (rc=0,ties=0;;ties++) */
	if (weight)
	   free(weight);
	return(rc);	/* done recurring to this level and circuit */
	}	/* end of assign_juggler() */

static int
count_preferences(unsigned short ji)
	{
	unsigned short ci;
	if (juggler[ji].prefstring[juggler[ji].pindex]==NULL)
	   return(0);
	ci=(unsigned short)atoi(&juggler[ji].prefstring[juggler[ji].pindex][1]);
	circuit[ci].preferred++;
	juggler[ji].pindex++;
	return(count_preferences(ji));
	}

int
main(int argc, char **argv, char **arge)	
	{
	int j,i,ci,ji,pi;	/* index variables through circuit, juggler, and preferences */
	char *cptr,*s;
	int c,*distribution,email_target_juggler,email_name,reverse,bump,unassigned,*pdist;
	int favoritism[JUGGLERS];

	output=NULL;
	email_target_juggler=1970;
	reverse=0;
	bump=0;

	for (ji=0;ji<JUGGLERS;ji++)
	    favoritism[ji]=ji;
	
	for (j=0;(c=getopt(argc,argv,"B:b:EeF:f:HhOoRrS:s:V:v:?"))!=-1;)
	    switch (c)
		   {
	      case '?': case 'H': case 'h':
		   printf("Usage: %s <-jJuggler> -e -o -s<flags> -v<flags>\n",argv[0]); break;
	      case 'J': case 'j': email_target_juggler=atoi(optarg); break;
	      case 'B': case 'b': bump=atoi(optarg); break;
	      case 'F': case 'f': 
		   if ((ji=atoi(optarg))<JUGGLERS)
		      {
		      i=favoritism[ji];
		      favoritism[ji]=favoritism[j];
		      favoritism[j++]=i;
		      }
		     else
		      fprintf(stderr,"%5u: ignoring -j%d, not in range of jugglers %u\n",__LINE__,ji,JUGGLERS);
		   break;
	      case 'E': case 'e': output=stderr; break;
	      case 'O': case 'o': output=stdout; break;
	      case 'R': case 'r': reverse=1-reverse; break;
	
	      case 'S': case 's':
		   for (s=optarg;*s;s++)
			silence[*s]++;
		   break;
	      case 'V': case 'v':
		   for (s=optarg;*s;s++)
			verbose[*s]++;
		   break;
	      default: fprintf(stderr,"Unknown option %c\n",c); return(__LINE__);
		   }
	

	JperC=(sizeof(juggler)/sizeof(juggler[0]))/(sizeof(circuit)/sizeof(circuit[0]));

	if ((distribution=calloc(sizeof(distribution[0]),JperC+2))==NULL)
	      return(fprintf(output,"%5u: unable to allocate int distribution[%d] array \n",
				__LINE__,JperC));	/* not -v, important failure */

	for (unassigned=i=0;i<JUGGLERS;i++)
	    {
	    j=(i+bump)%JUGGLERS;
	    ji=favoritism[reverse?JUGGLERS-j:j];
	    juggler[ji].pindex=0;
#ifdef PREF_AUDIT
	    count_preferences(ji);
#else
	    if (assign_juggler(ji,0))	/* true (non zero) return */
		{
		unassigned++;
		if (output && verbose['#'])	
		   fprintf(output,"%5u: %u unable to assign %s\n",__LINE__,unassigned,juggler[ji].name);
		}
#endif
	    }
/* turn next 10 lines into macro function */
#define CHECK_STATS(line) \
	if (output && verbose['a']) fprintf(output,"%5u: Assignments:\t %5u\n",line,counter['a']); \
	if (output && verbose['f']) fprintf(output,"%5u: Failures:\t %5u\n",line,counter['f']); \
	if (output && verbose['r']) fprintf(output,"%5u: Reassignments:\t %5u\n",line,counter['r']); \
	if (output && verbose['s']) fprintf(output,"%5u: Ties:\t %5u\n",line,counter['t']); \
	if (output && verbose['d']) \
	   { \
	   for (i=0;i<=JperC;i++) \
	       distribution[i]=0; \
	   for (ci=0;ci<(sizeof(circuit)/sizeof(circuit[0]));ci++) \
	       { \
	       circuit[ci].jugglers=0; \
	       if (circuit[ci].uTS) \
		  { \
	          for (i=0;i<JperC;i++) \
		      if (circuit[ci].uTS[i].ull!=0) \
			 circuit[ci].jugglers++; \
		  distribution[circuit[ci].jugglers]++; \
		  } \
		 else \
		  distribution[0]++; \
	       } \
	   fprintf(output,"Juggler Circuit distribution:\n"); \
	   for (i=0;i<=JperC;i++) \
	       fprintf(output,"%5u:\t%5u: %5u\n",line,i,distribution[i]); \
	   }
	CHECK_STATS(__LINE__)
	
	for (ci=0;ci<(sizeof(circuit)/sizeof(circuit[0]));ci++)
	    {
#ifdef PREF_AUDIT
	    printf("%u %s ",circuit[ci].preferred,circuit[ci].name);
#else
	    printf( "%s%c",circuit[ci].name,verbose['x']?'\n':' ');	/* Circuit name, if -vx newline else space */
	    if (circuit[ci].uTS)		/* if we tried to assign any jugglers ... */
	       for (i=0;i<JperC;i++)
		   {
		   if (verbose['x'])
		      {
		      if (circuit[ci].uTS[i].ull)	/* non zero uTS elements are assigned jugglers */
		         {
			 circuit[ci].jugglers++;
		         ji=circuit[ci].uTS[i].us.juggler;
		         printf("\t%016llx\t%s\t dpju %4hx %4hx %4hx %4hx\n",
				circuit[ci].uTS[i].ull,
				juggler[ji].name,
				circuit[ci].uTS[i].us.dot_product,
				circuit[ci].uTS[i].us.weight,
				circuit[ci].uTS[i].us.juggler,
				circuit[ci].uTS[i].us.used);
		         }
		      }
		     else
		      {
		      if (circuit[ci].uTS[i].ull)	/* non zero uTS elements are assigned jugglers */
		         {
		         if (i)
		            printf(", ");
			 circuit[ci].jugglers++;
		         ji=circuit[ci].uTS[i].us.juggler;
		         printf("%s", juggler[ji].name);
		         for (pi=0;juggler[ji].prefstring[pi];pi++)
			     {
			     cptr=juggler[ji].prefstring[pi];
			     j=atoi(&cptr[1]);
			     if (!silence[':'])
		                printf(" %s:%hu",cptr,Dot_Product(circuit[j],juggler[ji]));
			     }
		         }
		      }
		   }
#endif
	    printf("\n");
	    }
	
	if (output && verbose['p'])
	   {
	   if ((pdist=calloc(sizeof(pdist[0]),maxpref+1))==NULL)
	      return(fprintf(output,"%5u: unable to allocate int pdist[%d] array \n",__LINE__,pdist+1));

	   for (ji=0;ji<JUGGLERS;ji++)
	       pdist[juggler[ji].pindex]++;

	   fprintf(output,"%5u: Juggler preference index distribution\n",__LINE__);
	   for (i=0;i<=maxpref;i++)
	       fprintf(output,"%5u: %2u: %5u\n",__LINE__,i,pdist[i]);
	   }
	    
	    
	if (output && verbose['@'])	/* calculate email address */
	   {
	   ci=email_target_juggler;
	   for (email_name=i=0;i<JperC;i++)
	       if (circuit[ci].uTS[i].ull)	/* non zero uTS elements are assigned jugglers */
		  {
		  if (verbose['+'])
		     fprintf(output,"%u+%hu\n",email_name,circuit[ci].uTS[i].us.juggler);
	          email_name+=(int)circuit[ci].uTS[i].us.juggler;
		  }
	   fprintf(output,"%u unassigned of %u, email the results to %u@yodle.com\n",unassigned,JUGGLERS,email_name); 
	   }
	return(0);
	}
