/*
 *	Michael Foley, jugglefest circuit assignment 
 *
 *	moved progress comments from here to README file
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

static int verbose[256]={0};	/* verbose modes off by default, turn on with -v */
static int counter[256]={0};	/* counts of verbose message occurences */
static int silence[256]={0};	/* if non zero DISABLES some feature that is on by default, -s arg */

struct Circuit {
	char *name;
	unsigned short h,e,p;
	int preferred,jugglers;
	union ToSort {
		unsigned long long ull;
		struct {
			unsigned short used,juggler,tried,dot_product;
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

static unsigned short currJuggler=0;
static unsigned int currJugglerDepth=0;

#define JUGGLERS (sizeof(juggler)/sizeof(juggler[0]))

static unsigned short jugglers=JUGGLERS;
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

static short maxpref=0;	/* maximum preference index */
static FILE *output=NULL;	/* so I can hijack stderr calls */

static int strict_rule=1;
static unsigned long long calls_report=0;
static unsigned long long calls_interval=1000;
static unsigned long long sum=0,total=0;

static int fail_jugglers=0;	/* switched by -f argument */
static short failed_juggler=-1;	/* only meaningful if assign_juggler returns error to main */
static int email_target_circuit=-1;

static int
assign_juggler(unsigned short ji,int depth)
	{
	static int lowestTieDepth;
	static unsigned long long calls=0;
	union ToSort *uTS=NULL,*temp_uTS;
	unsigned short ties;			/* iterations through assignment/reassignment loop */
	unsigned short *tried=NULL; 		/* need to keep tried at each level of recursion */
	unsigned short ci,reassign,rc;		/* circuit index, reassign this juggler, return code */
	int i;	/* short term index variables */
	calls++;	/* counting recursions */
	
	if (ji==jugglers)	/* how we detect and end total recursion */
	   return(0);		/* ALL jugglers have been successfully assigned */
	if (juggler[ji].pindex>maxpref)
	   maxpref=juggler[ji].pindex;
	if (juggler[ji].prefstring[juggler[ji].pindex]==NULL)
	   {
	   failed_juggler=ji;
	   if (output && (verbose['f']))
	      fprintf(output,"%5u: %5u -vf %s preference list exhausted, festival failure\n", __LINE__,
			++counter['f'],juggler[ji].name);
	   if (lowestTieDepth==0)
	      lowestTieDepth=depth;	/* don't start checking until we have our first exhausted juggler */
	   if (fail_jugglers)
	      currJuggler++;
	   return(1-fail_jugglers);	/* return failure to assign, or success regarless if fail_jugglers==1 */
	   }
	ci=(unsigned short)atoi(&juggler[ji].prefstring[juggler[ji].pindex][1]);
	if (calls_report && (calls>calls_report))
	   {
	   fprintf(stdout,"%5u: calls=%llu depth=%d %s %hu %s\n",__LINE__,calls,depth,
			juggler[ji].name,juggler[ji].pindex,circuit[ci].name);
	   calls_report+=calls_interval;
	   }
	if (circuit[ci].uTS==NULL)	/* first visit to this circuit */
	   {
	   if ((circuit[ci].uTS=calloc(sizeof(union ToSort),JperC+1))==NULL)
	      return(output?fprintf(output,"%5u: unable to allocate union circuit[%d].uTS[X=%d] array \n",
				__LINE__,ci,JperC+1)
			   :__LINE__);	/* not -v, important failure */
	   }

	if (uTS==NULL)	/* first visit to this circuit */
	   {
	   if ((uTS=calloc(sizeof(union ToSort),JperC+1))==NULL)
	      return(output?fprintf(output,"%5u: unable to allocate union uTS[X=%d] array at depth %d\n",
				__LINE__,JperC+1,depth)
			   :__LINE__);	/* not -v, important failure */
	   }
	for (i=0;i<=JperC;i++)
	    {
	    circuit[ci].uTS[i].us.tried=0;	/* before first qsort at this level of recursion all tried are 0 */
	    uTS[i].ull=circuit[ci].uTS[i].ull;
	    }
#ifdef NEVER	/* removed from Show Assignments macro function below */
	   for (total=0,i=0;i<JUGGLERS;i++) \
	       total+=juggler[i].pindex; \
	%10llu %10llu	between -v%c and %7llu
	sum,total	between trigger and calls
#endif /* NEVER */
#define ShowAssignments(line,trigger,label) \
	if ((output) && (verbose[trigger])) \
	   { \
	   fprintf(output,"%5u: %7u -v%c %18llu %5u %6d %5hu %-5s %2hd %-6s @%s ", \
			line,++counter[trigger],trigger,calls,depth,depth-currJugglerDepth,currJuggler,circuit[ci].name,juggler[ji].pindex,juggler[ji].name,label); \
	   for (i=0;i<=JperC;i++) \
		fprintf(output,"%u:%04hu:%1hu:%05hu ",i, \
			circuit[ci].uTS[i].us.dot_product, \
			circuit[ci].uTS[i].us.tried, \
			circuit[ci].uTS[i].us.juggler, \
			circuit[ci].uTS[i].us.used); \
	   fprintf(output,"\n"); \
	   }
	circuit[ci].uTS[JperC].us.juggler=ji;
	circuit[ci].uTS[JperC].us.dot_product=Dot_Product(circuit[ci],juggler[ji]);
	circuit[ci].uTS[JperC].us.used=0xabba;
	for (rc=0,ties=0;;ties++)	/* by default we expect to succeed */
	    {
	    qsort(circuit[ci].uTS,JperC+1,sizeof(circuit[ci].uTS[0]),qsort_uTS); 
	    ShowAssignments(__LINE__,'0',"qsort")
	    if (circuit[ci].uTS[JperC].ull==0)	/* if we have successfully assigned ji continue down juggler list */
	       {
	       currJuggler++;	/* increment to the next Juggler needing assignment */
	       currJugglerDepth=depth;
	       ShowAssignments(__LINE__,'1',"nextj")
	       if ((rc=assign_juggler(currJuggler,depth+1))==0)	/* try to assign the rest of the jugglers */
		  break;	/* total success assigning all jugglers, break out of loop to clean up allocations and return rc */
	       currJuggler--;	/* decrement because we failed to assign next juggler */ /* TODO: May want to report verbose['f'] here */
	       circuit[ci].uTS[JperC].us.tried=1;	/* tried the empty overflow slot so we go into undo code below */
	       ShowAssignments(__LINE__,'4',"flJgr")
	       }
	    if (circuit[ci].uTS[JperC].us.tried)	/* if true we're failing, have to undo what we did here and return failure up recursion */
	       {
	       temp_uTS=uTS;
	       uTS=circuit[ci].uTS;
	       circuit[ci].uTS=temp_uTS;	/* restored assignment list saved when we entered this level of recursion. */
	       ShowAssignments(__LINE__,'7',"faild")
	       if (strict_rule)
	          rc=__LINE__;
		 else
	          {
	          if (Dot_Product(circuit[ci],juggler[ji])>circuit[ci].uTS[JperC-1].us.dot_product)	/* if ji not tied for last dp here */
		     {
	             juggler[ji].pindex++;			/* try next circuit preference */
		     sum++;
	             ShowAssignments(__LINE__,'2',"nxPrf")
	             if (rc=assign_juggler(ji,depth+1))
		        {
		        juggler[ji].pindex--;			/* if we failed, decrement on way back up to retry */
		        sum--;
		        }
		     }
		    else
		     rc=__LINE__;	/* ji was tied for dp AND we tried all ties, report failure */
		  }
	       ShowAssignments(__LINE__,'5',"flPrf") /* loop will iterate here */
	       break;		/* out of for (rc=0,ties=0;;ties++) loop */
	       }
	    if (ties)
	       {
	       counter['t']++;
	       ShowAssignments(__LINE__,'3',"tryti")
	       if (lowestTieDepth>=depth)
		  {
	          lowestTieDepth=depth;
	          ShowAssignments(__LINE__,'l',"lowti")
		  }
	       }
	    if ((output) && (verbose['r']))
	       fprintf(output,"%5u: %u -vr Circuit %s overfull, reassigning %s:%hu\n",__LINE__,++counter['r'],
			circuit[ci].name,juggler[circuit[ci].uTS[JperC].us.juggler].name,circuit[ci].uTS[JperC].us.dot_product);
	    if (tried==NULL)	/* do this inside the for loop only if juggler to reassign, don't churn and fragment the heap */
	       if ((tried=calloc(sizeof(tried[0]),JperC+1))==NULL)		/* need JperC array at each recursion level */
		  return(fprintf(stderr,"%5u: Unable to calloc unsigned short tried[%u] array during reassignment\n",__LINE__,JperC+1));
				/* above error may need hook to only print once no matter how many times hit */
				/* although if I'm out of heap that instance of the program is done anyway */
	    reassign=circuit[ci].uTS[JperC].us.juggler;	/* done with arg juggler, reuse */
	    for (i=0;i<=JperC;i++) tried[i]=circuit[ci].uTS[i].us.tried;	/* save the tried values in case we come through this circuit again at deepter level of recursion */
	    circuit[ci].uTS[JperC].ull=0;
	    juggler[reassign].pindex++;			/* try next circuit preference */
	    sum++;
	    if (rc=assign_juggler(reassign,depth+1))		/* non zero return is failure to reassign */
	       {	/* failure to reassign juggler */
	       sum--;
	       juggler[reassign].pindex--;			/* failed assignment, double check this decrements properly */
	       for (i=0;i<=JperC;i++) circuit[ci].uTS[i].us.tried=tried[i];	/* restore tried values for this level of recursion */
	       circuit[ci].uTS[JperC].us.juggler=reassign;
	       circuit[ci].uTS[JperC].us.tried=1;		/* this juggler be further up qsorted list for same dot_product */
	       circuit[ci].uTS[JperC].us.dot_product=Dot_Product(circuit[ci],juggler[reassign]);
	       circuit[ci].uTS[JperC].us.used=0xabba;
	       ShowAssignments(__LINE__,'6',"tried")
	       if (output && verbose['w'])
		  {
		  fprintf(output, "%5u: %u -vw %s %s dp=%hu tried=1 (%s %hx)\n" ,__LINE__,++counter['w'],
				circuit[ci].name,
				juggler[circuit[ci].uTS[JperC].us.juggler].name,
				circuit[ci].uTS[JperC].us.dot_product, /* tried=1 */
				juggler[circuit[ci].uTS[JperC-1].us.juggler].name,
				circuit[ci].uTS[JperC-1].us.dot_product);
		  }
		  /* for (rc=0;;) loop will iterate here */
	       }	/* end of failure to reassign juggler */
	      else	/* if rc=assign_juggler(reassign,depth+1) succeeds */
	       break;	/* out of for(;;) loop, rc should still be =0 because successfull reassignment of last uTS */
	   ShowAssignments(__LINE__,'8',"iter8") /* loop will iterate here */
	   }	/* end of for (rc=0,ties=0;;ties++) */
	if (tried)
	   free(tried);	/* free tried array created IF we hit reassignment logic above */
	if (uTS)
	   free(uTS);	/* free whichever assignment array is NOT hanging off circuit */
	ShowAssignments(__LINE__,'9',"rturn")
	if (ci==email_target_circuit)
	   ShowAssignments(__LINE__,'_',"email")
	return(rc);	/* done recurring to this level and circuit */
	}	/* end of assign_juggler(unsigned short ji, unsigned int depth) */

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
	short j,i,ci,ji,pi;	/* index variables through circuit, juggler, and preferences */
	char *cptr,*s;
	int c,*distribution,email_name,unassigned,*pdist;

	setbuf(stdin,NULL);
	setbuf(stdout,NULL);
	setbuf(stderr,NULL);

	output=NULL;
	email_target_circuit=1970;
	jugglers=JUGGLERS;

	for (optarg=NULL,j=0;(c=getopt(argc,argv,"C:c:EeFfHhJ:j:K:k:LlOoRrS:s:T:t:V:v:?"))!=-1;)
	    {
	    if (output && verbose['.'])
	       fprintf(output,optarg?"%5u:\t -%c %s\n":"%5u: -%c\n",__LINE__,c,optarg);
	    switch (c)
		   {
	      case '?': case 'H': case 'h':
		   printf("Usage: %s -t<Circuit> -e -o -f -c<interval> -s<flags> -v<flags>\n",argv[0]);
		   printf("\t\t-l\t\tLoosens strict assignment rule, tries to push jugglers down preference list to fit all jugglers in festival, incomplete\n");
		   printf("\t\t-f\t\tFails jugglers who exhaust preference lists instead of using total recursion to try and fit all by resolving ties, see README\n");
		   printf("\t\t-o\t\tPuts verbose output messages on stdout, messages go nowhere by default, conflicts with -e\n");
		   printf("\t\t-e\t\tPuts verbose output messages on stderr, messages go nowhere by default, conflicts with -o\n");
		   printf("\t\t-e\t\tPuts verbose output messages on stderr, messages go nowhere by default, conflicts with -o\n");
		   printf("\t\t-t<Circuit>\tChanges circuit used to calculate submission email address. Defaults to 1970\n");
		   printf("\t\t-c<interval>\tInterval at which to report recursive call statistics. Defaults to 0 and doesn't report\n");
		   printf("\t\t-j<Jugglers>\tHow many of the jugglers to attemp to assign. Defaults to ALL\n");
		   printf("\t\t-s<flags>\tSilence output optionally, sort of the opposite of verbose.\n");
		   printf("\t\t\t-s:\tTerse output, just comma separated juggler names.\n");
		   printf("\t\t-v<flags>\tVerbose output.\n");
		   printf("\t\t\t-v@\tDisplay calculated email address for target circuit\n");
		   printf("\t\t\t-v+\tDisplay calculation for email address for target circuit\n");
		   printf("\t\t\t-v_\tDebugging data for email address target circuit\n");
		   printf("\t\t\t-vr\tReassignment message\n");
		   printf("\t\t\t-vw\tWeighted ties tried message\n");
		   printf("\t\t\t-vu\tUnable to assign Juggler message from main\n");
		   printf("\t\t\t-vd\tDisplay Circuit distribution, how many jugglers end up on each circuit\n");
		   printf("\t\t\t-vp\tDisplay preference index distribution, counts for how many jugglers ended up how far down preference list, or exhausting it.\n");
		   printf("\t\t\t-v,\tPrefixes output with count of how many jugglers preferred each circuit.\n");
		   printf("\t\t\t-vl\tDisplay lowest recursion level debugging data\n");
		   printf("\t\t\t-v0-9\tDebugging data at decision points in recursion logic\n");
		   break;
	      case 'C': case 'c': calls_report=calls_interval=(unsigned long long)atol(optarg); break;
	      case 'T': case 't': email_target_circuit=atoi(optarg); break;
	      case 'J': case 'j': jugglers=(unsigned short)atoi(optarg); break;
	      case 'L': case 'l': strict_rule=0; break;	/* loosen strict rule, after failure, retry pushing juggler down preference list */
	      case 'F': case 'f': fail_jugglers=1; break;	/* fail jugglers who exhaust their preference list, instead of backing up to retry ties */
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
	    optarg=NULL;
	    }
	

	JperC=JUGGLERS/(sizeof(circuit)/sizeof(circuit[0]));

	if ((distribution=calloc(sizeof(distribution[0]),JperC+2))==NULL)
	      return(fprintf(output,"%5u: unable to allocate int distribution[%d] array \n",
				__LINE__,JperC));	/* not -v, important failure */

	for (ji=0;ji<jugglers;ji++)
	    {
	    juggler[ji].pindex=0;
	    count_preferences(ji);
	    juggler[ji].pindex=0;
	    }
	for (unassigned=0,failed_juggler=-1;currJuggler<jugglers;)
	    if (assign_juggler(currJuggler,0))	/* true (non zero) return */
	       {
	       unassigned++;
	       if (output && verbose['u'])	
	          fprintf(output,"%5u: %u unable to assign %s\n",__LINE__,unassigned,juggler[failed_juggler].name);
	       }
	      else
	       {
	       if (failed_juggler!=-1)
		  {
		  unassigned++;
	          if (output && verbose['u'])	
		     fprintf(output?output:stderr,"%5u: %u unable to assign %s\n",__LINE__,unassigned,juggler[failed_juggler].name);
	          if (failed_juggler==currJuggler)
		     currJuggler++;
	          failed_juggler=-1;
		  }
	       }
	/* macro function because I may want to use this again */
#define CHECK_STATS(line) \
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
	    if (output && verbose[','])
	       fprintf(output,"%u ",circuit[ci].preferred);
	    printf( "%s%c",circuit[ci].name,verbose['x']?'\n':' ');	/* Circuit name, if -vx newline else space */
	    if ((!silence[',']) && circuit[ci].uTS)		/* if we tried to assign any jugglers ... */
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
				circuit[ci].uTS[i].us.tried,
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
	    printf("\n");
	    }
	
	if (output && verbose['p'])
	   {
	   if ((pdist=calloc(sizeof(pdist[0]),maxpref+1))==NULL)
	      return(fprintf(output,"%5u: unable to allocate int pdist[%d] array \n",__LINE__,maxpref+1));

	   for (ji=0;ji<jugglers;ji++)
	       pdist[juggler[ji].pindex]++;

	   fprintf(output,"%5u: Juggler preference index distribution\n",__LINE__);
	   for (i=0;i<=maxpref;i++)
	       fprintf(output,"%5u: %2u: %5u\n",__LINE__,i,pdist[i]);
	   }
	    
	if (output && verbose['@'])	/* calculate email address */
	   {
	   ci=email_target_circuit;
	   for (email_name=i=0;i<JperC;i++)
	       if (circuit[ci].uTS[i].ull)	/* non zero uTS elements are assigned jugglers */
		  {
		  if (verbose['+'])
		     fprintf(output,"%u+%hu\n",email_name,circuit[ci].uTS[i].us.juggler);
	          email_name+=(int)circuit[ci].uTS[i].us.juggler;
		  }
	   fprintf(output,"%u unassigned of %u, email the results to %u@yodle.com\n",unassigned,jugglers,email_name); 
	   }
	return(0);
	}
