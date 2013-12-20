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
			unsigned short used,juggler,pindex,dot_product;
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
	return(rc);
	}

static short JperC;	/* Jugglers per Circuit, calculated from array sizes in main() */

static FILE *output=NULL;	/* so I can hijack stderr calls */

static int
assign_juggler(unsigned short ji)
	{
	unsigned short ci,rc;
	unsigned long long ull;
	int i;
	if (juggler[ji].prefstring[juggler[ji].pindex]==NULL)
	   return((output && (verbose['f']))
			?fprintf(output,
				"%5u: %u -vf Juggler[%hu] %s preference list exhausted, festival failure\n",
				__LINE__,++counter['f'],ji,juggler[ji].name)
			:__LINE__);
	ci=(unsigned short)atoi(&juggler[ji].prefstring[juggler[ji].pindex][1]);
	if (circuit[ci].uTS==NULL)	/* first visit to this circuit */
	   {
	   if ((circuit[ci].uTS=calloc(sizeof(union ToSort),JperC+1))==NULL)
	      return(output?fprintf(output,"%5u: unable to allocate union circuit[%d].u[X=%d] array \n",
				__LINE__,ci,JperC+1):__LINE__);	/* not -v, important failure */
	   }
	circuit[ci].uTS[JperC].us.dot_product=Dot_Product(circuit[ci],juggler[ji]);
	circuit[ci].uTS[JperC].us.pindex=PREFERENCES-juggler[ji].pindex;
	circuit[ci].uTS[JperC].us.juggler=ji;
	circuit[ci].uTS[JperC].us.used=0xabba;
	if ((output) && (verbose['a']))
	   fprintf(output,"%5u: %u -va Circuit %s added %s:%hu\n",__LINE__,++counter['a'],
			circuit[ci].name,juggler[ji].name,circuit[ci].uTS[JperC].us.dot_product);
	qsort(circuit[ci].uTS,JperC+1,sizeof(circuit[ci].uTS[0]),qsort_uTS); 
	if (circuit[ci].uTS[JperC].ull!=0)	/* if we have a juggler to reassign */
	   {
	   ji=circuit[ci].uTS[JperC].us.juggler;	/* done with arg juggler, reuse */
	if ((output) && (verbose['r']))
	   fprintf(output,"%5u: %u -vr Circuit %s overpopulated, reassigning %s:%hu\n",__LINE__,++counter['r'],
			circuit[ci].name,juggler[ji].name,circuit[ci].uTS[JperC].us.dot_product);
	   juggler[ji].pindex++;		/* try next circuit preference */
	   if (rc=assign_juggler(ji))		/* non zero return is failure, report */
		{
		juggler[ji].pindex--;		/* */
		for (i=JperC-1;i>=0;i--)
		    {
		    if (circuit[ci].uTS[i].us.dot_product!=circuit[ci].uTS[JperC].us.dot_product)
		       return(rc);
		    if (output && verbose['s'])
		       fprintf(output, "%5u: %u %s swapping twins %hx %d %s %s\n" ,__LINE__,++counter['s'],
				circuit[ci].name,circuit[ci].uTS[JperC].us.dot_product,i,
				juggler[circuit[ci].uTS[JperC].us.juggler].name,
				juggler[circuit[ci].uTS[i].us.juggler].name);
				
		    ull=circuit[ci].uTS[i].ull;		/* swap with twin */
		    circuit[ci].uTS[i].ull=circuit[ci].uTS[JperC].ull;
		    circuit[ci].uTS[JperC].ull=ull;    
	   	    ji=circuit[ci].uTS[JperC].us.juggler;	/* done with arg juggler, reuse */
		    juggler[ji].pindex++;
		    if ((rc=assign_juggler(ji))==0)
			return(circuit[ci].uTS[JperC].ull=0);	/* req'd for pretty debugging */
		    }
		return(rc);
		}
	       else
		circuit[ci].uTS[JperC].ull=0;	/* only required so array looks good in debugging */
	   }
	return(0);	/* successful assignment and any recursive reassignments */
	}

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
	int c,*distribution,email_target_juggler,email_name;

	output=NULL;
	email_target_juggler=1970;
	
	for (;(c=getopt(argc,argv,"EeHhOoS:s:V:v:?"))!=-1;)
	    switch (c)
		   {
	      case '?': case 'H': case 'h':
		   printf("Usage: %s <-jJuggler> -e -o -s<flags> -v<flags>\n",argv[0]); break;
	      case 'J': case 'j': email_target_juggler=atoi(optarg); break;
	      case 'E': case 'e': output=stderr; break;
	      case 'O': case 'o': output=stdout; break;

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

	for (ji=0;ji<(sizeof(juggler)/sizeof(juggler[0]));ji++)
	    {
	    juggler[ji].pindex=0;
#ifdef PREF_AUDIT
	    count_preferences(ji);
#else
	    assign_juggler(ji);
#ifdef OBSOLETE
	    if (assign_juggler(ji))	/* true (non zero) return */
		if (output)	
		   fprintf(output,"%5u: unable to assign juggler[%hu] %s\n",__LINE__,ji,juggler[ji].name);
#endif
#endif
	    }
	for (ci=0;ci<(sizeof(circuit)/sizeof(circuit[0]));ci++)
	    {
#ifdef PREF_AUDIT
	    printf("%u %s ",circuit[ci].preferred,circuit[ci].name);
#else
	    printf( "%s%c",circuit[ci].name,verbose['x']?'\n':' ');
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
				circuit[ci].uTS[i].us.pindex,
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
		   distribution[circuit[ci].jugglers]++;
		   }
#endif
	    printf("\n");
	    }
	if (output && verbose['a']) fprintf(output,"Assignments:\t %5u\n",counter['a']);
	if (output && verbose['f']) fprintf(output,"Failures:\t %5u\n",counter['f']);
	if (output && verbose['r']) fprintf(output,"Reassignments:\t %5u\n",counter['r']);
	if (output && verbose['s']) fprintf(output,"Swap Twins:\t %5u\n",counter['s']);
	if (output && verbose['d'])
	   {
	   fprintf(output,"Juggler Circuit distribution:\n");
	   for (i=0;i<=JperC;i++)
	       fprintf(output,"\t%5u: %5u\n",i,distribution[i]);
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
	   fprintf(output,"email the results to %u@yodle.com\n",email_name); 
	   }
	return(0);
	}
