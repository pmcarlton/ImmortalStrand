/*
 Works, 20110218pmc
 can specify how many rounds of EdU labeling with EDUGEN.


TODO also figure out why I can only do 16 generations max (not that it's meaningful to do much more, but...?!
	 make graphviz output and TIFF output for quick visualization (or use CIMG?)
*/ 		 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define XSNUM 23 //n, not 2n
#define EDUGEN 0 //number of labeling rounds AFTER the first one that occurs in head_init
#define RAND ((rand()+0.0)/(RAND_MAX+0.0))

/* GLOBALS */

int ID=0;
int GENERATION=0;
int GENS;

typedef struct cell{
int id;	// the unique ID
int xs[2][XSNUM]; // the set of post-S chromosomes (G2 or M)
int potential; //if it was born in the last round, then schedule to replicate in the next round
int current; //1 if scheduled to replicate, 0 if already replicated
int gen; // birth generation number
int parent; //the parent's ID
int md; //Mother/Daughter: 0 for mother, 1 for daughter (only meaningful for bias not 0.5, but set anyway)
int oldnew; //the number of EdU (value=1) chromosomes, for easy outputting
struct cell *prev;
struct cell *next;
} cell;

struct cell* head;
struct cell* tail;

void cell_replicate(cell* theCell, float bias) {

int li;
int edu;		//1 if EdU is present, 0 if it's gone
cell* c1;
cell* c2;
c1 = (cell*)malloc(sizeof(cell));
c2 = (cell*)malloc(sizeof(cell));

edu=(GENERATION <= EDUGEN);
c1->prev=tail;
tail->next=c1;
tail=c1;
c2->prev=tail;
tail->next=c2;
tail=c2;
tail->next=NULL;

theCell->current=0; //because it's division, not budding
c1->current=0;
c2->current=0;
c1->potential=1;
c2->potential=1;

c1->id=(++ID); c2->id=(++ID);
c1->parent = theCell->id; c2->parent = theCell->id;
c1->gen=GENERATION; c2->gen=GENERATION;
c1->md=0; c2->md=1;
c1->oldnew=0;c2->oldnew=0;

for (li=0;li<XSNUM;li++) {
	if(RAND < bias) {
		c1->xs[0][li] = theCell->xs[0][li]; c1->xs[1][li] = edu;
		c2->xs[0][li] = theCell->xs[1][li]; c2->xs[1][li] = edu;
		} else {
		c1->xs[0][li] = theCell->xs[1][li]; c1->xs[1][li] = edu;
		c2->xs[0][li] = theCell->xs[0][li]; c2->xs[1][li] = edu;
		}
	c1->oldnew+=c1->xs[0][li];c1->oldnew+=c1->xs[1][li];
	c2->oldnew+=c2->xs[0][li];c2->oldnew+=c2->xs[1][li];
}
}	

cell* init_head (cell* ptr) {
  int li;
  ptr=(cell*)malloc(sizeof(cell));
  ptr->id=(++ID);
  ptr->parent=0;
  ptr->gen=GENERATION;
  ptr->md=0;
  ptr->next=NULL;
  ptr->prev=NULL;
  ptr->current=1;
  ptr->potential=0;
  ptr->oldnew=0;
  
  for (li=0;li<XSNUM;li++) {
	ptr->xs[0][li]=0;
	ptr->xs[1][li]=1; //assuming one round of EdU incorporation
    ptr->oldnew+=ptr->xs[0][li];ptr->oldnew+=ptr->xs[1][li];
  }
  return(ptr);
}

void dumpstat (cell* ptr, FILE* out) {
fprintf(out,"%i %i %i %i %i %i \n", ptr->id, ptr->parent, ptr->gen, ptr->md, ptr->current, ptr->oldnew);
if(ptr->next) dumpstat(ptr->next,out);
}

/*MAIN*/

int main(int argc, char** argv) {
float bias;
int li,li2;
cell* workingCell;
FILE* out;

srand(getpid());
bias = atof(argv[1]);	// from 0 (random) to 1 (full blown immortal strands)
GENS = atoi(argv[2]);	// number of generations

GENERATION=0;
head=init_head(head);
tail=head;
GENERATION=1;

out=fopen("./ImmortalStats.txt","w");

while (GENERATION <= GENS){
workingCell=head;
while(workingCell) {
if(workingCell->current) {
  cell_replicate(workingCell,bias);
}
else if (workingCell->potential) {
  workingCell->potential=0;workingCell->current=1;
}
workingCell=workingCell->next;
}
++GENERATION;
}

dumpstat(head,out);
close(out);
return(GENERATION);
}
