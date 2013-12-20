/*
 *	Michael Foley, jugglefest circuit assignment easy version
 *
 *	it's easier to have the makefile rip the circuit and juggler information into 
 *	structure array initializations	than to write the parser for the file.
 *	I can also have the Makefile rip the 3 circuit 12 juggler sample for testing.
 *	I'm assuming someone at Yodle wrote the program to generate the output in the sample.
 *	if that's a bad assumption then I have no way to validate my answers.
 *	Another assumption, the LEAST popular circuits must still be one of the preferences of C/J jugglers 
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

struct Circuit {
	char *name;
	unsigned short h,e,p;
	int preferred;
	union ToSort {
		unsigned long long ull;
		struct {
			unsigned short used,filler,juggler,dot_product;
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

#define Dot_Product(c,j) (c.h*j.h+c.e*j.e+c.p*j.p)

static int
qsort_uTS(const void *a, const void *b)
	{
	union ToSort *uTSa,*uTSb;
	int rc;
	uTSa=(union ToSort *)a;
	uTSb=(union ToSort *)b;
	if (uTSa->ull<uTSb->ull)
	   rc=1;
	  else
	   if (uTSa->ull>uTSb->ull)
	      rc=-1;
	     else
	      rc=0;
#ifdef NEVER
	printf("qsort: a 0x%016llx b 0x%016x rc=%2d\n",uTSa->ull,uTSb->ull,rc);
#endif
	return(rc);
	}

static short JperC;	/* Jugglers per Circuit, calculated from array sizes in main() */

static FILE *output;

static int
assign_juggler(unsigned short ji)
	{
	unsigned short ci;
	if (juggler[ji].prefstring[juggler[ji].pindex]==NULL)
	   return(fprintf(output,"Juggler[%hu] %s preference list exhausted, festival failure\n",
				ji,juggler[ji].name));
	ci=(unsigned short)atoi(&juggler[ji].prefstring[juggler[ji].pindex][1]);
	if (circuit[ci].uTS==NULL)	/* first visit to this circuit */
	   {
	   if ((circuit[ci].uTS=calloc(sizeof(union ToSort),JperC+1))==NULL)
	      return(fprintf(output,"%5u: unable to allocate union circuit[%d].u[X=%d] array \n",
				__LINE__,ci,JperC+1));
	   }
	circuit[ci].uTS[JperC].us.dot_product=Dot_Product(circuit[ci],juggler[ji]);
	circuit[ci].uTS[JperC].us.juggler=ji;
	circuit[ci].uTS[JperC].us.used=0xbeef;
	circuit[ci].uTS[JperC].us.filler=0xdead;
#ifdef OBSOLETE
	printf("Circuit %s added %s:%hu\n",
		circuit[ci].name,juggler[ji].name,circuit[ci].uTS[JperC].us.dot_product);
#endif
	qsort(circuit[ci].uTS,JperC+1,sizeof(circuit[ci].uTS[0]),qsort_uTS); 
	if (circuit[ci].uTS[JperC].ull!=0)	/* if we have a juggler to reassign */
	   {
	   ji=circuit[ci].uTS[JperC].us.juggler++;	/* done with arg juggler, reuse */
#ifdef OBSOLETE
	   printf("Circuit %s overpopulated, reassigning %s:%hu\n",
			circuit[ci].name,juggler[ji].name,circuit[ci].uTS[JperC].us.dot_product);
#endif
	   juggler[ji].pindex++;
	   if (assign_juggler(ji))				/* non zero return is failure, report */
	      return(fprintf(output,"%5u: Unable to reassign Juggler[%hu] %s\n",
				__LINE__,ji,juggler[ji].name));
	   }
	return(0);	/* successful assignment and any recursive reassignments */
	}

int
main()	/* no arguments needed, input is #included at compile time, output to stdout */
	{
	int j,i,ci,ji,pi;	/* index variables through circuit, juggler, and preferences */
	char *cptr;

	JperC=(sizeof(juggler)/sizeof(juggler[0]))/(sizeof(circuit)/sizeof(circuit[0]));

	output=stdout;	/* change to stderr for release */

	for (ji=0;ji<(sizeof(juggler)/sizeof(juggler[0]));ji++)
	    {
	    juggler[ji].pindex=0;
	    if (assign_juggler(ji))	/* true (non zero) return */
		fprintf(output,"%5u: unable to assign juggler[%hu] %s\n",__LINE__,ji,juggler[ji].name);
	    }
	for (ci=0;ci<(sizeof(circuit)/sizeof(circuit[0]));ci++)
	    {
	    printf("%s ",circuit[ci].name);
	    if (circuit[ci].uTS)		/* if we tried to assign any jugglers ... */
	       for (i=0;i<JperC;i++)
		   {
		   if (i)
		     printf(", ");
		   if (circuit[ci].uTS[i].ull)	/* non zero uTS elements are assigned jugglers */
		      {
		      ji=circuit[ci].uTS[i].us.juggler;
		      printf("%s", juggler[ji].name);
		      for (pi=0;juggler[ji].prefstring[pi];pi++)
			  {
			  cptr=juggler[ji].prefstring[pi];
			  j=atoi(&cptr[1]);
		          printf(" %s:%hu",cptr,Dot_Product(circuit[j],juggler[ji]));
			  }
		      }
		   }
	    printf("\n");
	    }
	return(0);
	}
