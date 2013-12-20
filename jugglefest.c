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
 
#ifundef PREFERENCES
#define PREFERENCES 11		/* we only need 10+1 for null for test data */
#endif /* PREFERENCES */
struct Juggler {
	char *name;		/* name has to be the first item in struct for initialization */
	unsigned short h,e,p;	/* h,e,p has to be the second,third,fourth itemin struct for initialization */
	char *prefstring[PREFERENCES];	/* prefstring has to be the fifth item in struct for initialization */
	int *pref;
	short prefindex;	/* index into preference array for next attempted circuit assignment */
	} 
#ifdef LIL
	juggler[] = {
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

static int
assign_juggler(unsigned short ji)
	{
	unsigned short ci;
	if (juggler[ji].prefstring[juggler[ji.pindex]==NULL)
	   return(fprintf(stderr,"Juggler[%hu] %s preference list exhausted, festival failure\n",
				ji,juggler[ji].name));
	ci=(unsigned short(atoi(&juggler[ji].prefstring[juggler[ji].pindex][1]);
	if (circuit[ci].uTS==NULL)	/* first visit to this circuit */
	   {
	   if ((circuit[ci].uTS=calloc(sizeof(union ToSort),JperC+1))==NULL)
	      return(fprintf(stderr,"%5u: unable to allocate union circuit[%d].u[X=%d] array \n",
				__LINE__,ci,JperC+1));
	   }
	circuit[ci].uTS[JperC].us.dot_product=Dot_Product(circuit[ci],juggler[ji]);
	circuit[ci].uTS[JperC].us.juggler=ji;
	circuit[ci].uTS[JperC].us.used=0xbeef;
	circuit[ci].uTS[JperC].us.filler=0xdead;
	printf("Circuit %s added %s:%hx\n",
		circuit[ci].name,juggler[ji].name,circuit[ci].uTS[i].us.dot_product);
	qsort(circuit[ci].uTS,JperC+1,sizeof(circuit[ci].uTS[0]),qsort_uTS); 
	if (circuit[ci].uTS[JperC].ull!=0)	/* if we have a juggler to reassign */
	   {
	   ji=circuit[ci].uTS[JperC].us.juggler++;	/* done with arg juggler, reuse */
	   printf("Circuit %s added %s:%hx\n",
			circuit[ci].name,juggler[ji].name,circuit[ci].uTS[i].us.dot_product);
	   juggler[ji].pindex++;
	   if (assign_juggler(ji))				/* non zero return is failure, report */
	      return(fprintf(stderr,"%5u: Unable to reassign Juggler[%hu] %s\n",
				__LINE__,ji,juggler[ji].name));
	   }
	return(0);	/* successful assignment and any recursive reassignments */
	}

int
main()	/* no arguments needed, input is #included at compile time, output to stdout */
	{
	union ToSort *uTS;
	int ii,i,ci,ji,pi;	/* index variables through circuit, juggler, and preferences */
	int X,rc;	/* and X is preference list size, atoi return code */

#ifdef SANITY
	/* Sanity check circuit names */
	for (ci=0; ci<(sizeof(circuit)/sizeof(circuit[0]));ci++)
	    if ((rc=atoi(&circuit[ci].name[1]))!=ci)
	       return(fprintf(stderr,"%5u: Circuit index %d does not match name %d\n",__LINE__,ci,rc));

	/* Sanity check juggler names */
	for (ji=0; ji<(sizeof(juggler)/sizeof(juggler[0]));ji++)
	    if ((rc=atoi(&juggler[ji].name[1]))!=ji)
	       return(fprintf(stderr,"%5u: Juggler index %d does not match name %d\n",__LINE__,ji,rc));
#endif /* SANITY */

	JperC=ji/ci;

	for (X=0; juggler[0].prefstring[X];X++) ;
	
	if ((uTS=calloc(sizeof(union ToSort),(JperC+1)))==NULL)
	   return(fprintf(stderr,"%5u: unable to allocate union ToSort[X=%d] array \n",__LINE__,JperC+1));

	for (ci=0;ci<(sizeof(circuit)/sizeof(circuit[0]));ci++)
	    if ((circuit[ci].uTS=calloc(sizeof(union ToSort),JperC))==NULL)
	       return(fprintf(stderr,"%5u: unable to allocate union circuit[%d].u[X=%d] array \n",
				__LINE__,ci,JperC));
	
	for (ji=0;ji<(sizeof(juggler)/sizeof(juggler[0]));ji++)
	    {
		if ((juggler[ji].pref=malloc(sizeof(int)*X))==NULL)
		   return(fprintf(stderr,"unable to allocate int[X=%d] array for juggler # %d of %d\n",
					X,ji,(sizeof(juggler)/sizeof(juggler[0]))));
		for (pi=0;pi<X;pi++)	/* depend on X calculated above */
		    {
		    juggler[ji].pref[pi]=ci=atoi(&juggler[ji].prefstring[pi][1]);
		    circuit[ci].preferred++;
		    for (i=0;i<JperC;i++)
			{
#ifdef NEVER
		        printf("%5u: J%d C%d %2d dot_product=%4x juggler=%4x used=%4x: %016x\n",__LINE__,
				ji,ci,i,
				uTS[i].us.dot_product,
				uTS[i].us.juggler,
				uTS[i].us.used,
				uTS[i].ull);
#endif
			uTS[i].ull=circuit[ci].uTS[i].ull;
			}
		    uTS[i].us.dot_product=Dot_Product(circuit[ci],juggler[ji]);
		    uTS[i].us.juggler=ji;
		    uTS[i].us.used=0xbeef;
		    uTS[i].us.filler=0xdead;
		    printf("Circuit %s added %s:%hx\n",
				circuit[ci].name,juggler[ji].name,uTS[i].us.dot_product);
#ifdef NEVER
		    printf("%5u: J%d C%d %2d dot_product=%4hx juggler=%4hx used=%4hx: %016llx\n",__LINE__,
				ji,ci,i,
				uTS[i].us.dot_product,
				uTS[i].us.juggler,
				uTS[i].us.used,
				uTS[i].ull);
#endif
		    qsort(uTS,JperC+1,sizeof(uTS[0]),qsort_uTS); 
		    for (i=0;i<JperC;i++)
			{
#ifdef NEVER
		        printf("%5u: J%d C%d %2d dot_product=%4hx juggler=%4hx used=%4hx: %016x\n",__LINE__,
				ji,ci,i,
				uTS[i].us.dot_product,
				uTS[i].us.juggler,
				uTS[i].us.used,
				uTS[i].ull);
#endif
			circuit[ci].uTS[i].ull=uTS[i].ull;
			}
#ifdef NEVER
		    printf("%5u: J%d C%d %2d dot_product=%4hx juggler=%4hx used=%4hx: %016x\n",__LINE__,
				ji,ci,i,
				uTS[i].us.dot_product,
				uTS[i].us.juggler,
				uTS[i].us.used,
				uTS[i].ull);
#endif
		    if (uTS[i].us.used)
		       {
		       printf("Circuit %s has >%d Jugglers, ", circuit[ci].name,JperC);
		       for (ii=0;ii<JperC+1;ii++)
		           printf("%s:%hx ",juggler[uTS[ii].us.juggler].name,uTS[ii].us.dot_product);
		       printf("%s being dropped: %016llx\n", juggler[uTS[i].us.juggler].name,uTS[i].ull);
		       circuit[ci].preferred--;
		       }
		    }
	    }
	for (ci=0;ci<(sizeof(circuit)/sizeof(circuit[0]));ci++)
	    {
	    printf("%s %d ",circuit[ci].name,circuit[ci].preferred);
	    for (i=0;i<JperC;i++)
		printf("%s:%hx ",
			juggler[circuit[ci].uTS[i].us.juggler].name,
			circuit[ci].uTS[i].us.dot_product);
	    printf("\n");
	    }
	return(0);
	}
