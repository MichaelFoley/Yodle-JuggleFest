/*
 *	Juggle Festival Circit assignments
 *	
 *	Writing a specific solution to the 2000 Circuit 12000 Juggler puzzle
 *	Neglecting the more general solution that would calculate the array sizes
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

struct Circuit {
	int used;
	char *name;
	int index;	/* in case the ordinal of the names aren't contiguous */
	int ordinal;
	int hand_to_hand;
	int endurance;
	int pizzaz;
	} *circuit;

struct Juggler {
	int used;
	char *name;
	int index;	/* in case the ordinal of the names aren't contiguous */
	int ordinal;
	int hand_to_hand;
	int endurance;
	int pizzaz;
	char *prefptr;	/* pointer to list of preferences */
	int prefcount;	/* count of preferences */
	int prefcircuits;
	} *juggler;

static int cindex=0;
static int jindex=0;

static void
parse_buffer(char *buffer)
	{
	enum { undecided, crct, cname, ch, ce, cp, jglr, jname, jh, je, jp, prefs } state;
	register int i;
	int number=0;
	char *token=null;
	state=undecided;
	for (i=0; buffer[i]; i++)
	    switch (buffer[i])
		   {
	      case '0': case '1': case '2': case '3': case '4':
	      case '5': case '6': case '7': case '8': case '9':
		   number*=10;
		   number+=buffer[i]-'0';
		   break;
	      case 'C':
		   state=crct;
		   token=&buffer[i];
	           break;
	      case 'J':
		   state=crct;
		   token=&buffer[i];
	           break;
	      case '\n': 
		   if (state==cp)
		      circuit[cindex++].pizzaz=number;
		     else
		      juggler[jindex++].prefptr=token;
		   state=undecided;
		   break;
	      case ',': commas++;
	      case ' ':
		   switch (state)
			  {
		     case undecided: state=(*token=='C')?cname:jname; break;
		     case crct: 
			  }
	      default:	/* 
		   }
	return(0);
	}

int
jugglefest(char *filename, int circuits, int jugglers, char *output)
	{
	FILE *f;
	char *buffer,*s;
	size_t len;	/* size of input file, so we can allocate the buffer */
	if ((f=fopen(filename,"r"))==NULL)
	   return(fprintf(stderr,"Unable to fopen(%s,r) errno=%d, %s\n",filename,(int)errno,strerror(errno)));
	fseek(0,SEEK_END,f);
	len=ftell(f);
	if ((buffer=malloc(len+1))==NULL)
	   return(fprintf(stderr,"Unable to malloc(%d) to read the contents of %s\n",len,filename,fclose(f)));
	if ((rc=fread(buffer,1,len,f))!=len)
	   return(fprintf(stderr,"Unable to fread %d bytes from %s, rc=%d\n",len,filename,rc,fclose(f)));
	buffer[len]='\0';	/* mark end of file with null char */
	fclose(f);	/* done with file, close it */
	if ((circuit=calloc(circuits,sizeof(circuit[0])))==NULL)
	   return(fprintf(stderr,"Unable to calloc(%d,circuit) for the circuit array\n",circuits,filename));
	if ((juggler=calloc(jugglers,sizeof(juggler[0])))==NULL)
	   return(fprintf(stderr,"Unable to calloc(%d,juggler) for the juggler array\n",jugglers,filename));
	if (parse_buffer(buffer))
	   return((__LINE__);
	return(0);
	}

