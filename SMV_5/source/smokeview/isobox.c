// $Date$ 
// $Revision$
// $Author$

#include "options.h"
#include <stdlib.h>
#ifdef pp_DRAWISO
#ifdef pp_OSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#endif
#include <math.h>
#include <stdio.h>
#include "MALLOC.h"
#define IN_ISOBOX
#include "isodefs.h"

// svn revision character string
char isobox_revision[]="$Revision$";

unsigned short *vertices=NULL;
int *rank=NULL,*sortedlist=NULL;
int *closestnodes=NULL;

/* ------------------ GetIsobox ------------------------ */

void GetIsobox(const float *x, 
               const float *y, 
               const float *z, 
               const float *vals, 
               const float *tvals, 
               const int *nodeindexes, 
               float level,
               float *xvert, 
               float *yvert, 
               float *zvert, 
               float *tvert, int *closestnodes, int *nvert,
               int *triangles, int *ntriangles){
       /*
       INPUT
       -----
       x - x[0] = min x value
           x[1] = max x value
       y - y[0] = min y value
           y[1] = max y value
       z - z[0] = min z value
           z[1] = max z value

      vals - values at box formed by x,y,z
      tvals- values at box formed by x,y,z
      level - desired iso-surface level

       OUTPUT
       -----
       xvert, yvert, zvert - array of x,y,z coordinates that have iso-surface value 
       nvert - number of vertices
       triangles - set of 3 integer indices for each triangle pointing into xvert, yvert, zvert arrays
       ntriangles - number of indices in triangles

       */

  float vmin, vmax;
  float xxval[8],yyval[8],zzval[8];
  int bigger, casenum, type, sign;
  int ixmin[4]={0,1,4,5}, ixmax[4]={2,3,6,7};
  int iymin[4]={0,3,4,7}, iymax[4]={1,2,5,6};
  int izmin[4]={0,1,2,3}, izmax[4]={4,5,6,7};
  int n;
  int edge,*edges=NULL, nedges, *path=NULL, *case2=NULL, npath;
  int v1, v2;
  float val1, val2, denom, factor;
  float xx, yy, zz;
  int outofbounds;
  int prods[]={1,2,4,8,16,32,64,128};
  int thistype;

int compcase[]={0,0,0,-1,0,0,-1,-1,0,0,0,0,-1,-1,0};


int edge2vertex[12][2]={
  {0,1},{1,2},{2,3},{0,3},
  {0,4},{1,5},{2,6},{3,7},
  {4,5},{5,6},{6,7},{4,7}
};




int cases[256][10]={
{0,0,0,0,0,0,0,0, 0,  0},{0,1,2,3,4,5,6,7, 1,  1},{1,2,3,0,5,6,7,4, 1,  2},
{1,2,3,0,5,6,7,4, 2,  3},{2,3,0,1,6,7,4,5, 1,  4},{0,4,5,1,3,7,6,2, 3,  5},
{2,3,0,1,6,7,4,5, 2,  6},{3,0,1,2,7,4,5,6, 5,  7},{3,0,1,2,7,4,5,6, 1,  8},
{0,1,2,3,4,5,6,7, 2,  9},{3,7,4,0,2,6,5,1, 3, 10},{2,3,0,1,6,7,4,5, 5, 11},
{3,0,1,2,7,4,5,6, 2, 12},{1,2,3,0,5,6,7,4, 5, 13},{0,1,2,3,4,5,6,7, 5, 14},
{0,1,2,3,4,5,6,7, 8, 15},{4,0,3,7,5,1,2,6, 1, 16},{4,5,1,0,7,6,2,3, 2, 17},
{1,2,3,0,5,6,7,4, 3, 18},{5,1,0,4,6,2,3,7, 5, 19},{2,3,0,1,6,7,4,5, 4, 20},
{4,5,1,0,7,6,2,3, 6, 21},{2,3,0,1,6,7,4,5, 6, 22},{3,0,1,2,7,4,5,6,14, 23},
{4,5,1,0,7,6,2,3, 3, 24},{7,4,0,3,6,5,1,2, 5, 25},{2,6,7,3,1,5,4,0, 7, 26},
{3,0,1,2,7,4,5,6, 9, 27},{2,6,7,3,1,5,4,0, 6, 28},{4,0,3,7,5,1,2,6,11, 29},
{0,1,2,3,4,5,6,7,12, 30},{0,0,0,0,0,0,0,0, 0,  0},{5,4,7,6,1,0,3,2, 1, 32},
{0,3,7,4,1,2,6,5, 3, 33},{1,0,4,5,2,3,7,6, 2, 34},{4,5,1,0,7,6,2,3, 5, 35},
{2,3,0,1,6,7,4,5, 3, 36},{3,7,4,0,2,6,5,1, 7, 37},{6,2,1,5,7,3,0,4, 5, 38},
{0,1,2,3,4,5,6,7, 9, 39},{3,0,1,2,7,4,5,6, 4, 40},{3,7,4,0,2,6,5,1, 6, 41},
{5,6,2,1,4,7,3,0, 6, 42},{3,0,1,2,7,4,5,6,11, 43},{3,0,1,2,7,4,5,6, 6, 44},
{1,2,3,0,5,6,7,4,12, 45},{0,1,2,3,4,5,6,7,14, 46},{0,0,0,0,0,0,0,0, 0,  0},
{5,1,0,4,6,2,3,7, 2, 48},{1,0,4,5,2,3,7,6, 5, 49},{0,4,5,1,3,7,6,2, 5, 50},
{4,5,1,0,7,6,2,3, 8, 51},{4,7,6,5,0,3,2,1, 6, 52},{1,0,4,5,2,3,7,6,12, 53},
{4,5,1,0,7,6,2,3,11, 54},{0,0,0,0,0,0,0,0, 0,  0},{5,1,0,4,6,2,3,7, 6, 56},
{1,0,4,5,2,3,7,6,14, 57},{0,4,5,1,3,7,6,2,12, 58},{0,0,0,0,0,0,0,0, 0,  0},
{4,0,3,7,5,1,2,6,10, 60},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{6,7,3,2,5,4,0,1, 1, 64},{0,1,2,3,4,5,6,7, 4, 65},
{1,0,4,5,2,3,7,6, 3, 66},{0,4,5,1,3,7,6,2, 6, 67},{2,1,5,6,3,0,4,7, 2, 68},
{6,7,3,2,5,4,0,1, 6, 69},{5,6,2,1,4,7,3,0, 5, 70},{0,1,2,3,4,5,6,7,11, 71},
{3,0,1,2,7,4,5,6, 3, 72},{0,1,2,3,4,5,6,7, 6, 73},{7,4,0,3,6,5,1,2, 7, 74},
{2,3,0,1,6,7,4,5,12, 75},{7,3,2,6,4,0,1,5, 5, 76},{1,2,3,0,5,6,7,4,14, 77},
{1,2,3,0,5,6,7,4, 9, 78},{0,0,0,0,0,0,0,0, 0,  0},{4,0,3,7,5,1,2,6, 3, 80},
{0,3,7,4,1,2,6,5, 6, 81},{2,3,0,1,6,7,4,5, 7, 82},{5,1,0,4,6,2,3,7,12, 83},
{2,1,5,6,3,0,4,7, 6, 84},{0,1,2,3,4,5,6,7,10, 85},{5,6,2,1,4,7,3,0,12, 86},
{0,0,0,0,0,0,0,0, 0,  0},{0,1,2,3,4,5,6,7, 7, 88},{7,4,0,3,6,5,1,2,12, 89},
{3,0,1,2,7,4,5,6,13, 90},{0,0,0,0,0,0,0,0, 0,  0},{7,3,2,6,4,0,1,5,12, 92},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{5,4,7,6,1,0,3,2, 2, 96},{6,2,1,5,7,3,0,4, 6, 97},{2,1,5,6,3,0,4,7, 5, 98},
{2,1,5,6,3,0,4,7,14, 99},{1,5,6,2,0,4,7,3, 5,100},{1,5,6,2,0,4,7,3,12,101},
{1,5,6,2,0,4,7,3, 8,102},{0,0,0,0,0,0,0,0, 0,  0},{5,4,7,6,1,0,3,2, 6,104},
{0,4,5,1,3,7,6,2,10,105},{2,1,5,6,3,0,4,7,12,106},{0,0,0,0,0,0,0,0, 0,  0},
{5,6,2,1,4,7,3,0,11,108},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{7,6,5,4,3,2,1,0, 5,112},{0,4,5,1,3,7,6,2,11,113},
{6,5,4,7,2,1,0,3, 9,114},{0,0,0,0,0,0,0,0, 0,  0},{1,5,6,2,0,4,7,3,14,116},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{7,6,5,4,3,2,1,0,12,120},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{7,6,5,4,3,2,1,0, 1,128},
{0,1,2,3,4,5,6,7, 3,129},{1,2,3,0,5,6,7,4, 4,130},{1,2,3,0,5,6,7,4, 6,131},
{7,4,0,3,6,5,1,2, 3,132},{1,5,6,2,0,4,7,3, 7,133},{1,5,6,2,0,4,7,3, 6,134},
{3,0,1,2,7,4,5,6,12,135},{3,2,6,7,0,1,5,4, 2,136},{4,0,3,7,5,1,2,6, 5,137},
{7,4,0,3,6,5,1,2, 6,138},{2,3,0,1,6,7,4,5,14,139},{6,7,3,2,5,4,0,1, 5,140},
{2,3,0,1,6,7,4,5, 9,141},{1,2,3,0,5,6,7,4,11,142},{0,0,0,0,0,0,0,0, 0,  0},
{4,0,3,7,5,1,2,6, 2,144},{3,7,4,0,2,6,5,1, 5,145},{7,6,5,4,3,2,1,0, 6,146},
{1,0,4,5,2,3,7,6,11,147},{4,0,3,7,5,1,2,6, 6,148},{3,7,4,0,2,6,5,1,12,149},
{1,0,4,5,2,3,7,6,10,150},{0,0,0,0,0,0,0,0, 0,  0},{0,3,7,4,1,2,6,5, 5,152},
{4,0,3,7,5,1,2,6, 8,153},{0,3,7,4,1,2,6,5,12,154},{0,0,0,0,0,0,0,0, 0,  0},
{0,3,7,4,1,2,6,5,14,156},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{5,1,0,4,6,2,3,7, 3,160},{1,2,3,0,5,6,7,4, 7,161},
{1,0,4,5,2,3,7,6, 6,162},{4,5,1,0,7,6,2,3,12,163},{3,0,1,2,7,4,5,6, 7,164},
{0,1,2,3,4,5,6,7,13,165},{6,2,1,5,7,3,0,4,12,166},{0,0,0,0,0,0,0,0, 0,  0},
{3,2,6,7,0,1,5,4, 6,168},{4,0,3,7,5,1,2,6,12,169},{1,2,3,0,5,6,7,4,10,170},
{0,0,0,0,0,0,0,0, 0,  0},{6,7,3,2,5,4,0,1,12,172},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{6,5,4,7,2,1,0,3, 5,176},
{0,4,5,1,3,7,6,2, 9,177},{0,4,5,1,3,7,6,2,14,178},{0,0,0,0,0,0,0,0, 0,  0},
{6,5,4,7,2,1,0,3,12,180},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{5,4,7,6,1,0,3,2,11,184},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{7,3,2,6,4,0,1,5, 2,192},{6,5,4,7,2,1,0,3, 6,193},{7,3,2,6,4,0,1,5, 6,194},
{0,3,7,4,1,2,6,5,10,195},{3,2,6,7,0,1,5,4, 5,196},{3,2,6,7,0,1,5,4,12,197},
{3,2,6,7,0,1,5,4,14,198},{0,0,0,0,0,0,0,0, 0,  0},{2,6,7,3,1,5,4,0, 5,200},
{0,3,7,4,1,2,6,5,11,201},{2,6,7,3,1,5,4,0,12,202},{0,0,0,0,0,0,0,0, 0,  0},
{3,2,6,7,0,1,5,4, 8,204},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{5,4,7,6,1,0,3,2, 5,208},{3,7,4,0,2,6,5,1,14,209},
{5,4,7,6,1,0,3,2,12,210},{0,0,0,0,0,0,0,0, 0,  0},{4,7,6,5,0,3,2,1,11,212},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{6,7,3,2,5,4,0,1, 9,216},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{4,7,6,5,0,3,2,1, 5,224},
{4,7,6,5,0,3,2,1,12,225},{1,5,6,2,0,4,7,3,11,226},{0,0,0,0,0,0,0,0, 0,  0},
{7,6,5,4,3,2,1,0, 9,228},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{2,6,7,3,1,5,4,0,14,232},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{5,4,7,6,1,0,3,2, 8,240},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},{0,0,0,0,0,0,0,0, 0,  0},
{0,0,0,0,0,0,0,0, 0,  0}
};

int pathcclist[15][13]={
  { 0},
  { 3,0,1,2},
  { 6,0,1,2,2,3,0},
  { 6,0,1,2,3,4,5},
  { 6,0,1,2,3,4,5},
  { 9,0,1,2,2,3,4,0,2,4},
  { 9,0,1,2,2,3,0,4,5,6},
  { 9,0,1,2,3,4,5,6,7,8},
  { 6,0,1,2,2,3,0},
  {12,0,1,5,1,4,5,1,2,4,2,3,4},
  {12,0,1,2,0,2,3,4,5,6,4,6,7},
  {12,0,1,5,1,4,5,1,2,4,2,3,4},
  {12,0,1,2,3,4,5,3,5,6,3,6,7},
  {12,0,1,2,3,4,5,6,7,8,9,10,11},
  {12,0,1,5,1,4,5,1,2,4,2,3,4}
};

int pathcclist2[15][19]={
  { 0},
  { 0},
  { 0},
  { 12,0,1,2,0,2,3,4,5,6,4,6,7},
  { 0},
  { 0},
  { 15,0,1,2,0,2,3,4,5,6,7,8,9,7,9,10},
  { 15,0,1,2,3,4,5,3,5,7,3,7,8,5,6,7},
  { 0},
  { 0},
  { 12,0,1,2,0,2,3,4,5,6,4,6,7},
  { 0},
  { 12,0,1,2,3,4,6,3,6,7,4,5,6},
  { 12,0,1,2,3,4,5,6,7,8,9,10,11},
  { 0}
};



int pathccwlist[15][13]={
  { 0},
  { 3,0,2,1},
  { 6,0,2,1,0,3,2},
  { 6,0,2,1,3,5,4},
  { 6,0,2,1,3,5,4},
  { 9,0,2,1,2,4,3,0,4,2},
  { 9,0,2,1,0,3,2,4,6,5},
  { 9,0,2,1,3,5,4,6,8,7},
  { 6,0,2,1,0,3,2},
  {12,0,5,1,1,5,4,1,4,2,2,4,3},
  {12,0,2,1,0,3,2,4,6,5,4,7,6},
  {12,0,5,1,1,5,4,1,4,2,2,4,3},
  {12,0,2,1,3,5,4,3,6,5,3,7,6},
  {12,0,2,1,3,5,4,6,8,7,9,11,10},
  {12,0,5,1,1,5,4,1,4,2,2,4,3}
};

int pathccwlist2[15][19]={
  { 0},
  { 0},
  { 0},
  { 12,0,2,1,0,3,2,4,6,5,4,7,6},
  { 0},
  { 0},
  { 15,0,2,1,0,3,2,4,6,5,7,9,8,7,10,9},
  { 15,0,2,1,3,5,4,3,7,5,3,8,7,5,7,6},
  { 0},
  { 0},
  { 12,0,2,1,0,3,2,4,6,5,4,7,6},
  { 0},
  { 12,0,2,1,3,6,4,3,7,6,4,6,5},
  { 12,0,2,1,3,5,4,6,8,7,9,11,10},
  { 0}
};

int edgelist[15][13]={
  { 0                             },
  { 3,0,4, 3                      },
  { 4,0,4, 7, 2                   },
  { 6,0,4, 3, 7,11,10             },
  { 6,0,4, 3, 6,10, 9             },
  { 5,0,3, 7, 6, 5                },
  { 7,0,4, 7, 2, 6,10,9           },
  { 9,4,8,11, 2, 3, 7,6,10,9      },
  { 4,4,7, 6, 5                   },
  { 6,2,6, 9, 8, 4, 3             },
  { 8,0,8,11, 3,10, 9,1, 2        },
  { 6,4,3, 2,10, 9, 5             },
  { 8,4,8,11, 0, 3, 7,6, 5        },
  {12,0,4, 3, 7,11,10,2, 6,1,8,5,9},
  { 6,3,7, 6, 9, 8, 0             }
};

int edgelist2[15][16]={
  { 0                             },
  { 0},
  { 0},
  { 8,3,0,10,7,0,4,11,10},
  { 0},
  { 0},
  { 11, 7,10,9,4,0,4,9,0,9,6,2},
  { 9,7,10,11,3,4,8,9,6,2},
  { 0},
  { 0},
  { 8,0,8,9,1,3,2,10,11},
  { 0},
  { 8,0,3,4,8,11,7,6,5},
  { 12,4,11,8,0,5,1,7,3,2,9,10,6},
  { 0}
};

/* determine min and max solution values */

  vmin = vals[0]; 
  vmax = vals[0];
  for(n=0;n<12;n++){
    closestnodes[n]=0;
  }
  for(n=1;n<8;n++){
    if(vals[n]<vmin){vmin=vals[n];}
    if(vals[n]>vmax){vmax=vals[n];}
  }

/* if the iso-surface level is not bounded by the vals data then there is nothing to do */

  *nvert = 0;
  *ntriangles = 0;
  if(vmin>level||vmax<level)return;

/* determine which of 256 cases this is */

  casenum = 0; bigger = 0;
  sign = 1;
  for(n=0;n<8;n++){
    if(vals[n]>level){
      bigger++; 
      casenum |= prods[n];
    }
  }

/* there are more nodes greater than the iso-surface level than below, so 
   solve the complementary problem */

  if(bigger>4){
    sign=-1; casenum=0;
    for(n=0;n<8;n++){
      if(vals[n]<level)casenum |= prods[n];
    }
  }

/* stuff min and max grid data into a more convenient form 
  assuming the following grid numbering scheme

       5-------6
     / |      /| 
   /   |     / | 
  4 -------7   |
  |    |   |   |  
  Z    1---|---2
  |  Y     |  /
  |/       |/
  0--X-----3     

  */

  for(n=0;n<4;n++){
    xxval[ixmin[n]] = x[0];
    xxval[ixmax[n]] = x[1];
    yyval[iymin[n]] = y[0];
    yyval[iymax[n]] = y[1];
    zzval[izmin[n]] = z[0];
    zzval[izmax[n]] = z[1];
  }

  if(casenum<=0||casenum>=255)return; /* no iso-surface */

  case2 = &(cases[casenum][0]);
  type = case2[8];
  if(type==0)return;

  if(compcase[type]==-1){thistype=sign;}
  else{thistype=1;}
 
  if(thistype!=-1){
    edges = &(edgelist[type][1]);
    if(sign>0)path = &(pathcclist[type][1]);   /* construct triangles clock wise */
     else{path = &(pathccwlist[type][1]);}     /* construct triangles counter clockwise */
  }
  else{
    edges = &(edgelist2[type][1]);
    if(sign>0)path = &(pathcclist2[type][1]);   /* construct triangles clock wise */
     else{path = &(pathccwlist2[type][1]);}     /* construct triangles counter clockwise */
  }
  npath = path[-1];
  nedges = edges[-1];

/* calculate where iso-surface level crosses each edge */

  outofbounds=0;
  for(n=0;n<nedges;n++){
    edge = edges[n];
    v1 = case2[edge2vertex[edge][0]];
    v2 = case2[edge2vertex[edge][1]];
    val1 = vals[v1]-level; val2 = vals[v2]-level;
    denom = val2 - val1;
    factor = 0.5;
    if(denom!=0.0)factor = -val1/denom;
    if(factor<0.5){
      closestnodes[n]=nodeindexes[v1];
    }
    else{
      closestnodes[n]=nodeindexes[v2];
    }
    if(factor>1.0){
      /*factor=1;*/
      outofbounds=1;
    }
    if(factor<0.0){
      /*factor=0.0;*/
      outofbounds=1;
    }
    xx = xxval[v1]*(1.0-factor) + xxval[v2]*factor;
    yy = yyval[v1]*(1.0-factor) + yyval[v2]*factor;
    zz = zzval[v1]*(1.0-factor) + zzval[v2]*factor;
    xvert[n] = xx;
    yvert[n] = yy;
    zvert[n] = zz;
    if(tvert!=NULL)tvert[n]= tvals[v1]*(1.0-factor) + tvals[v2]*factor;

  }
  if(outofbounds==1){
    printf("*** warning - computed isosurface vertices are out of bounds for :\n");
    printf("case number=%i level=%f\n",casenum,level);
    printf("values=");
    for(n=0;n<8;n++){
      printf("%f ",vals[n]);
    }
    printf("\n");
    printf("x=%f %f y=%f %f z=%f %f\n\n",x[0],x[1],y[0],y[1],z[1],z[2]);
  }

  /* copy coordinates to output array */

  *nvert = nedges;
  *ntriangles = npath;
  for(n=0;n<npath;n++){
    triangles[n] = path[n];
  }
}

/* ------------------ calcNormal2 ------------------------ */

void calcNormal2(const unsigned short *v1, 
                 const unsigned short *v2, 
                 const unsigned short *v3, 
                 float *out, float *area){
  float u[3], v[3];
  int i;


  for(i=0;i<3;i++){
    u[i]=v2[i]-v1[i];
    v[i]=v3[i]-v1[i];
  }


  out[0] = u[1]*v[2] - u[2]*v[1];
  out[1] = u[2]*v[0] - u[0]*v[2];
  out[2] = u[0]*v[1] - u[1]*v[0];
  *area = sqrt((double)(out[0]*out[0]+out[1]*out[1]+out[2]*out[2]))/2.0;

  ReduceToUnit(out);

}

/* ------------------ calcNormal ------------------------ */

void calcNormal(const float *xx, const float *yy, const float *zz, float *out){
  float u[3],v[3], p1[3], p2[3], p3[3];
  static const int x = 0;
  static const int y = 1;
  static const int z = 2;

  p1[x]=xx[0]; p1[y]=yy[0]; p1[z]=zz[0];
  p2[x]=xx[1]; p2[y]=yy[1]; p2[z]=zz[1];
  p3[x]=xx[2]; p3[y]=yy[2]; p3[z]=zz[2];


  u[x] = p2[x] - p1[x];
  u[y] = p2[y] - p1[y];
  u[z] = p2[z] - p1[z];

  v[x] = p3[x] - p1[x];
  v[y] = p3[y] - p1[y];
  v[z] = p3[z] - p1[z];

  out[x] = u[y]*v[z] - u[z]*v[y];
  out[y] = u[z]*v[x] - u[x]*v[z];
  out[z] = u[x]*v[y] - u[y]*v[x];

  ReduceToUnit(out);

}

/* ------------------ ReduceToUnit ------------------------ */

void ReduceToUnit(float *v){

  float length;

  length = (float)sqrt((double)(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]));

  if(length==0.0f)length=1.0f;

  v[0] /= length;
  v[1] /= length;
  v[2] /= length;
}

/* ------------------ GetIsosurface ------------------------ */

int GetIsosurface(isosurface *surface, 
                  const float *data, 
                  const float *tdata, 
                  const char *iblank_cell, 
                  float level,
                  const float *xplt, int nx, 
                  const float *yplt, int ny, 
                  const float *zplt, int nz,
                   int isooffset
                   ){
#define ijkcell(i,j,k) ((i)+(j)*ibar+(k)*ijbar)
#define ij(i,j) ((i)+(j)*nx)

  int ibar,ijbar;
  float xvert[12], yvert[12], zvert[12], tvert[12], *tvertptr=NULL;
  int triangles[18];
  int nvert, ntriangles;
  int i, j, k;
  float xxx[2], yyy[2], zzz[2], vals[8], tvals[8], *tvalsptr=NULL;
  int nodeindexes[8], closestnodes[18];
  float *xx, *yy, *zz;
  int ijkbase,ip1jk,ijkp1,ip1jkp1,ijp1k,ip1jp1k,ijp1kp1,ip1jp1kp1;
  int ijbase;
  int nxy;
  
  ibar = nx-1;
  ijbar = (nx-1)*(ny-1);
  xx = xxx;
  yy = yyy;
  zz = zzz;
  nxy = nx*ny;
  if(surface->defined==0)return 0;
  if(tdata!=NULL){
    tvalsptr=tvals;
    tvertptr=tvert;
  }
  for(i=0;i<nx-isooffset;){
    xx[0]=xplt[i];
    xx[1]=xplt[i+isooffset];
    for(j=0;j<ny-isooffset;){
      yy[0]=yplt[j];
      yy[1]=yplt[j+isooffset];
      ijbase = ij(i,j);
      for(k=0;k<nz-isooffset;){
        ijkbase = ijbase + k*nxy;
        ip1jk = ijkbase + isooffset;
        ijkp1 = ijkbase + isooffset*nxy;
        ip1jkp1 = ijkbase + isooffset*(1+nxy);
        ijp1k = ijkbase + isooffset*nx;
        ip1jp1k = ijkbase + isooffset*(1+nx);
        ijp1kp1 = ijkbase + isooffset*(nx+nxy);
        ip1jp1kp1 = ijkbase + isooffset*(1+nx+nxy);

        if(isooffset!=1||iblank_cell==NULL||iblank_cell[ijkcell(i,j,k)]!=0){
          zz[0]=zplt[k];
          zz[1]=zplt[k+isooffset];

          nodeindexes[0]=ijkbase;
          nodeindexes[1]=ijp1k;
          nodeindexes[2]=ip1jp1k;
          nodeindexes[3]=ip1jk;
          nodeindexes[4]=ijkp1;
          nodeindexes[5]=ijp1kp1;
          nodeindexes[6]=ip1jp1kp1;
          nodeindexes[7]=ip1jkp1;

          vals[0]=data[ijkbase];
          vals[1]=data[ijp1k];
          vals[2]=data[ip1jp1k];
          vals[3]=data[ip1jk];
          vals[4]=data[ijkp1];
          vals[5]=data[ijp1kp1];
          vals[6]=data[ip1jp1kp1];
          vals[7]=data[ip1jkp1];

          if(tdata!=NULL){
            tvals[0]=tdata[ijkbase];
            tvals[1]=tdata[ijp1k];
            tvals[2]=tdata[ip1jp1k];
            tvals[3]=tdata[ip1jk];
            tvals[4]=tdata[ijkp1];
            tvals[5]=tdata[ijp1kp1];
            tvals[6]=tdata[ip1jp1kp1];
            tvals[7]=tdata[ip1jkp1];
          }

          GetIsobox(xx, yy, zz, vals, tvalsptr, nodeindexes, level,
                    xvert, yvert, zvert, tvertptr, closestnodes, &nvert, triangles, &ntriangles);

          if(UpdateIsosurface(surface, xvert, yvert, zvert, tvertptr, 
                              closestnodes, nvert, triangles, ntriangles)!=0)return 1;

        }
        k+=isooffset;
      }
      j+=isooffset;
    }
    i+=isooffset;
  }
  surface->defined=1;
  return 0;
}

/* ------------------ compareisonodes ------------------------ */

int compareisonodes( const void *arg1, const void *arg2 ){
  int i, j;
  i=*(int *)arg1;
  j=*(int *)arg2;
  i *= 3;
  j *= 3;
  if(vertices[i]<vertices[j])return -1;
  if(vertices[i]>vertices[j])return 1;
  i++; j++;
  if(vertices[i]<vertices[j])return -1;
  if(vertices[i]>vertices[j])return 1;
  i++; j++;
  if(vertices[i]<vertices[j])return -1;
  if(vertices[i]>vertices[j])return 1;
  return 0;
}

/* ------------------ computerank ------------------------ */

int computerank( const void *arg1, const void *arg2 ){
  int i, j;
  i=*(int *)arg1;
  j=*(int *)arg2;
  if(sortedlist[i]<sortedlist[j])return -1;
  if(sortedlist[i]>sortedlist[j])return 1;
  return 0;
}

/* ------------------ order_closestnodes ------------------------ */

int order_closestnodes( const void *arg1, const void *arg2 ){
  int i, j;
  int ii,jj;
  i=*(int *)arg1;
  j=*(int *)arg2;
  ii = closestnodes[i];
  jj = closestnodes[j];
  if(ii<jj)return -1;
  if(ii>jj)return 1;
  return 0;
}

/* ------------------ CompressIsosurface ------------------------ */

int CompressIsosurface(isosurface *surface, int reduce_triangles,
                        float xmin, float xmax, 
                        float ymin, float ymax, 
                        float zmin, float zmax){
  int i,j,nn;
  float *x=NULL, *y=NULL, *z=NULL, *t=NULL;
  int *map=NULL,*map2=NULL,nmap2,*vertexmap=NULL,*inverse_vertexmap=NULL;
  int nvertices;
  unsigned short *newvertices=NULL;
  int *triangles=NULL,*newtriangles=NULL,*newtriangles2=NULL,ntriangles;
  float xyzmaxdiff;
  int *cs=NULL, *ordered_closestnodes=NULL;
  int v1, v2, v3;
  int ii,iim1;
  unsigned short vx, vy, vz;
  int sumx, sumy, sumz, sumt;
  int nnewvertices;
  float tmin, tmax, tmaxmin;
  int flag;
  unsigned short *tvertices,*newtvertices;

  nvertices=surface->nvertices;
  if(nvertices==0)return 0;
  vertices=surface->vertices;

  x=surface->xvert; 
  y=surface->yvert; 
  z=surface->zvert; 
  if(surface->tvert!=NULL)t=surface->tvert;

  sortedlist=surface->sortedlist;
  rank=surface->rank;
  triangles=surface->triangles;
  ntriangles=surface->ntriangles;
  
  if(surface->dataflag==1){
    tvertices=surface->tvertices;
    FREEMEMORY(tvertices);
    if(NewMemory((void **)&tvertices,nvertices*sizeof(unsigned short))==0){
      FREEMEMORY(tvertices);
      return 1;
    }
    surface->tvertices=tvertices;
  }

  FREEMEMORY(vertices); FREEMEMORY(rank);
  if(NewMemory((void **)&vertices,3*nvertices*sizeof(unsigned short))==0||
     NewMemory((void **)&rank,nvertices*sizeof(int))==0||
     NewMemory((void **)&map,nvertices*sizeof(int))==0||
     NewMemory((void **)&map2,nvertices*sizeof(int))==0||
     NewMemory((void **)&sortedlist,nvertices*sizeof(int))==0){
    FREEMEMORY(vertices);
    FREEMEMORY(rank);
    FREEMEMORY(map);
    FREEMEMORY(map2);
    FREEMEMORY(sortedlist);
    return 1;
  }

  surface->vertices=vertices;
  surface->rank=rank;
  xyzmaxdiff = xmax - xmin;
  if(ymax-ymin>xyzmaxdiff)xyzmaxdiff=ymax-ymin;
  if(zmax-zmin>xyzmaxdiff)xyzmaxdiff=zmax-zmin;
  surface->xmin=xmin;
  surface->ymin=ymin;
  surface->zmin=zmin;
  surface->xyzmaxdiff=xyzmaxdiff;

  if(t!=NULL&&surface->dataflag==1){
    tmin=t[0];
    tmax=tmin;
    for(i=1;i<nvertices;i++){
      if(t[i]<tmin)tmin=t[i];
      if(t[i]>tmax)tmax=t[i];
    }
    surface->tmin=tmin;
    surface->tmax=tmax;
    tmaxmin=tmax-tmin;
    if(tmaxmin>0.0){
      for(i=0;i<nvertices;i++){
        tvertices[i]=(unsigned short)(65535*(t[i]-tmin)/tmaxmin);
      }
    }
    else{
      for(i=0;i<nvertices;i++){
        tvertices[i]=0;
      }
    }
  }

  for(i=0;i<nvertices;i++){
    vertices[3*i+0]=(unsigned short)(65535*(x[i]-xmin)/xyzmaxdiff);
    vertices[3*i+1]=(unsigned short)(65535*(y[i]-ymin)/xyzmaxdiff);
    vertices[3*i+2]=(unsigned short)(65535*(z[i]-zmin)/xyzmaxdiff);
    sortedlist[i]=i;
    map[i]=i;
  	rank[i]=i;
  }

  qsort((int *)sortedlist,(size_t)nvertices,sizeof(int),compareisonodes);
  qsort((int *)rank,(size_t)nvertices,sizeof(int),computerank);

  j=0;
  map[0]=0;
  map2[0]=0;
  nmap2=1;
  for(i=1;i<nvertices;i++){
    if(compareisonodes(sortedlist+i-1,sortedlist+i)!=0){
  	  j++;
  	  map2[j]=i;
  	  nmap2++;
    }
    map[i]=j;
  }

  nvertices=nmap2;

  if(NewMemory((void **)&newvertices,3*nvertices*sizeof(unsigned short))==0||
     NewMemory((void **)&cs,nvertices*sizeof(int))==0){
    FREEMEMORY(newvertices);
    FREEMEMORY(cs);
    return 1;
  }
  if(surface->dataflag==1){
    if(NewMemory((void **)&newtvertices,nvertices*sizeof(unsigned short))==0){
      FREEMEMORY(newtvertices);
      return 1;
    }
  }

  closestnodes=surface->closestnodes;

  for(i=0;i<nvertices;i++){
	  j=sortedlist[map2[i]];
	  newvertices[3*i+0]=vertices[3*j+0];
	  newvertices[3*i+1]=vertices[3*j+1];
	  newvertices[3*i+2]=vertices[3*j+2];
    cs[i] = closestnodes[j];
  }
  if(surface->dataflag==1){
    for(i=0;i<nvertices;i++){
	    j=sortedlist[map2[i]];
	    newtvertices[i]=tvertices[j];
    }
  }

  FREEMEMORY(closestnodes);
  surface->closestnodes = cs;

  if(NewMemory((void **)&newtriangles,ntriangles*sizeof(int))==0){
    return 1;
  }
  for(i=0;i<ntriangles;i++){newtriangles[i]=map[rank[triangles[i]]];}
  surface->triangles=newtriangles;
  surface->vertices=newvertices;
  surface->nvertices=nvertices;
  FREEMEMORY(vertices);
  if(surface->dataflag==1){
    surface->tvertices=newtvertices;
    FREEMEMORY(tvertices);
  }
  FREEMEMORY(triangles);
  FREEMEMORY(map);FREEMEMORY(map2);FREEMEMORY(sortedlist);

  if(reduce_triangles!=1)return 0;

  /* phase II compression, reduce the number of triangles */

  /* first, eliminate triangles whose nodes (2 or more) are closest to the same grid point */

  if(NewMemory((void **)&newtriangles2,ntriangles*sizeof(int))==0){
    return 1;
  }
  nn=0;
  for(i=0;i<ntriangles/3;i++){
    v1=newtriangles[3*i];
    v2=newtriangles[3*i+1];
    v3=newtriangles[3*i+2];
    if(cs[v1]!=cs[v2]&&cs[v1]!=cs[v3]&&cs[v2]!=cs[v3]){
      newtriangles2[nn++]=v1;
      newtriangles2[nn++]=v2;
      newtriangles2[nn++]=v3;
    }
  }
  FREEMEMORY(newtriangles);
  surface->triangles=newtriangles2;
  ntriangles=nn;
  surface->ntriangles=ntriangles;

  /* sort the closestnodes list */

  closestnodes=surface->closestnodes;
  if(NewMemory((void **)&ordered_closestnodes,nvertices*sizeof(int))==0){
    return 1;
  }
  for(i=0;i<nvertices;i++){
    ordered_closestnodes[i]=i;
  }
  qsort((int *)ordered_closestnodes,(size_t)nvertices,sizeof(int),order_closestnodes);
  if(NewMemory((void **)&vertexmap,nvertices*sizeof(int))==0||
     NewMemory((void **)&inverse_vertexmap,nvertices*sizeof(int))==0){
    FREEMEMORY(vertexmap);
    FREEMEMORY(inverse_vertexmap);
    return 1;
  }

  for(i=0;i<nvertices;i++){vertexmap[i]=i;}
  nn=0;
  sumx=0; sumy = 0; sumz = 0; sumt=0;

  /* average nodes */

  nn=0;
  vertices = surface->vertices;
  if(surface->dataflag==1)tvertices = surface->tvertices;
  nnewvertices=0;
  for(i=1;i<nvertices+1;i++){
    iim1=ordered_closestnodes[i-1];
    if(i!=nvertices)ii=ordered_closestnodes[i];
    inverse_vertexmap[iim1] = nnewvertices;
    nn++;
    vx = vertices[3*iim1  ]; sumx += vx;
    vy = vertices[3*iim1+1]; sumy += vy;
    vz = vertices[3*iim1+2]; sumz += vz;
    if(surface->dataflag==1&&tvertices!=NULL)sumt += tvertices[iim1];

    flag=0;
    if(i!=nvertices&&closestnodes[ii]!=closestnodes[iim1])flag=1;
    if(i==nvertices||flag==1){
      vertexmap[nnewvertices++] = iim1;
      vertices[3*iim1  ]=sumx/nn;
      vertices[3*iim1+1]=sumy/nn;
      vertices[3*iim1+2]=sumz/nn;
      if(surface->dataflag==1&&tvertices!=NULL)tvertices[iim1]=sumt/nn;

      nn=0; 
      sumx = 0; sumy = 0;sumz = 0; sumt = 0;
    }
  }

  if(NewMemory((void **)&newvertices,3*nnewvertices*sizeof(unsigned short))==0){
    return 1;
  }
  if(surface->dataflag==1){
    if(NewMemory((void **)&newtvertices,nnewvertices*sizeof(unsigned short))==0){
      return 1;
    }
  }
  for(i=0;i<nnewvertices;i++){
    ii = vertexmap[i];
    newvertices[3*i  ] = vertices[3*ii  ];
    newvertices[3*i+1] = vertices[3*ii+1];
    newvertices[3*i+2] = vertices[3*ii+2];
    if(surface->dataflag==1&&tvertices!=NULL)newtvertices[i]=tvertices[ii];
  }
  FREEMEMORY(vertices);
  surface->vertices = newvertices;
  surface->nvertices = nnewvertices;
  if(surface->dataflag==1){
    FREEMEMORY(tvertices);
    surface->tvertices=newtvertices;
  }
  triangles = surface->triangles;
  for(i=0;i<ntriangles;i++){
    triangles[i] = inverse_vertexmap[triangles[i]];
  }
  FREEMEMORY(ordered_closestnodes);
  FREEMEMORY(vertexmap);
  FREEMEMORY(inverse_vertexmap);

  return 0;

}

/* ------------------ UpdateIsosurface ------------------------ */

int UpdateIsosurface(isosurface *surface, 
                      const float *xvert, 
                      const float *yvert, 
                      const float *zvert, 
                      const float *tvert, 
                      const int *closestnodes, 
                      int nvert, 
                      const int *triangles, 
                      int ntriangles){
  int n,ns, noldvert, *is;
  float *xs=NULL, *ys=NULL, *zs=NULL, *ts=NULL;
  int *cn=NULL;

  if(ResizeSurface(surface,nvert,ntriangles,0)!=0)return 1;

  /* copy vertex data */

  if(nvert>0){
    noldvert = surface->nvertices;
    xs = surface->xvert + noldvert;
    ys = surface->yvert + noldvert;
    zs = surface->zvert + noldvert;
    if(tvert!=NULL)ts = surface->tvert + noldvert;
    cn = surface->closestnodes + noldvert;

    for(n=0;n<nvert;n++){
      xs[n] = xvert[n];
      ys[n] = yvert[n];
      zs[n] = zvert[n];
      if(tvert!=NULL&&ts!=NULL)ts[n]=tvert[n];
      cn[n] = closestnodes[n];
    }
    surface->nvertices = noldvert + nvert;
  }

  /* copy triangle path data */

  if(ntriangles>0){
    ns = surface->ntriangles;
    is = surface->triangles + ns;
    for(n=0;n<ntriangles;n++){
      is[n] = triangles[n]+noldvert;
    }
    surface->ntriangles = ns + ntriangles;
  }
  return 0;
}

/* ------------------ ResizeSurface ------------------------ */

int ResizeSurface(isosurface *surfacedata, int incvert, int inctriangles, int incnorm){

  int maxnum, *itemp=NULL;
  float *temp=NULL;

  /* resize vertex data if necessary */

  maxnum = surfacedata->nvertices+incvert;
  if(maxnum>surfacedata->maxvertices){
    maxnum += INCPOINTS;
    surfacedata->maxvertices = maxnum;

    temp = surfacedata->xvert;
    if(temp==NULL){
      if(NewMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
    }
    else{
      if(ResizeMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
    }
    surfacedata->xvert = temp;

    temp = surfacedata->yvert;
    if(temp==NULL){
      if(NewMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
    }
    else{
      if(ResizeMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
    }
    surfacedata->yvert = temp;

    temp = surfacedata->zvert;
    if(temp==NULL){
      if(NewMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
    }
    else{
      if(ResizeMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
    }
    surfacedata->zvert = temp;

    if(surfacedata->dataflag==1){
      temp = surfacedata->tvert;
      if(temp==NULL){
        if(NewMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
      }
      else{
        if(ResizeMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
      }
      surfacedata->tvert = temp;
    }

    itemp = surfacedata->closestnodes;
    if(itemp==NULL){
      if(NewMemory((void **)&itemp,maxnum*sizeof(int))==0)return 1;
    }
    else{
      if(ResizeMemory((void **)&itemp,maxnum*sizeof(int))==0)return 1;
    }
    surfacedata->closestnodes = itemp;
  }

  /* resize norm data if necessary */

  maxnum = surfacedata->nnorm+incnorm;
  if(maxnum>surfacedata->maxnorm){
    maxnum += INCPOINTS;
    surfacedata->maxnorm = maxnum;

    temp = surfacedata->xnorm;
    if(temp==NULL){
      if(NewMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
    }
    else{
      if(ResizeMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
    }
    surfacedata->xnorm = temp;

    temp = surfacedata->ynorm;
    if(temp==NULL){
      if(NewMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
    }
    else{
      if(ResizeMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
    }
    surfacedata->ynorm = temp;

    temp = surfacedata->znorm;
    if(temp==NULL){
      if(NewMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
    }
    else{
      if(ResizeMemory((void **)&temp,maxnum*sizeof(float))==0)return 1;
    }
    surfacedata->znorm = temp;
  }

  /* resize triangles data if necessary */

  maxnum = surfacedata->ntriangles+inctriangles;
  if(maxnum>surfacedata->maxtriangles){
    maxnum += INCPOINTS;
    surfacedata->maxtriangles = maxnum;

    itemp = surfacedata->triangles;
    if(itemp==NULL){
      if(NewMemory((void **)&itemp,maxnum*sizeof(int))==0)return 1;
    }
    else{
      if(ResizeMemory((void **)&itemp,maxnum*sizeof(int))==0)return 1;
    }
    surfacedata->triangles = itemp;
  }
  return 0;

}

/* ------------------ InitIsosurface ------------------------ */

void InitIsosurface(isosurface *surfacedata, float level, float *color,int colorindex){
  surfacedata->compression_type=0;
  surfacedata->level = level;
  surfacedata->color = color;
  surfacedata->colorindex=colorindex;
  surfacedata->defined=1;
  surfacedata->nvertices=0;
  surfacedata->maxvertices=-1;
  surfacedata->ntriangles=0;
  surfacedata->maxtriangles=-1;
  surfacedata->nnorm=0;
  surfacedata->maxnorm=-1;
  surfacedata->normtype=0;
  surfacedata->xvert=NULL;
  surfacedata->yvert=NULL;
  surfacedata->zvert=NULL;
  surfacedata->xnorm=NULL;
  surfacedata->ynorm=NULL;
  surfacedata->znorm=NULL;
  surfacedata->vertices=NULL;
  surfacedata->tvertices=NULL;
  surfacedata->rank=NULL;
  surfacedata->sortedlist=NULL;
  surfacedata->triangles=NULL;
  surfacedata->plottype = 0;
  surfacedata->closestnodes=NULL;
  surfacedata->tvert=NULL;
  surfacedata->cullfaces=0;
}

/* ------------------ freesurface ------------------------ */

void freesurface(isosurface *surfacedata){
  if(surfacedata->defined==0)return;
  FREEMEMORY(surfacedata->xvert);
  FREEMEMORY(surfacedata->yvert);
  FREEMEMORY(surfacedata->zvert);
  FREEMEMORY(surfacedata->xnorm);
  FREEMEMORY(surfacedata->ynorm);
  FREEMEMORY(surfacedata->znorm);
  FREEMEMORY(surfacedata->triangles);
  FREEMEMORY(surfacedata->vertices);
  FREEMEMORY(surfacedata->tvertices);
  FREEMEMORY(surfacedata->sortedlist);
  FREEMEMORY(surfacedata->rank);
  FREEMEMORY(surfacedata->closestnodes);
  FREEMEMORY(surfacedata->tvert);
  surfacedata->defined=0;
}

/* ------------------ SmoothIsoSurface ------------------------ */

void SmoothIsoSurface(isosurface *surfacedata){
  int *triangles_i;
  unsigned short *vertices_i,*v1,*v2,*v3;
  int n,i1,i2,i3;
  int ntriangles_i,nvertices_i;
  float out[3],area;
  float *xyznorm;
  short *norm,*vertexnorm;

  triangles_i = surfacedata->triangles;
  vertices_i = surfacedata->vertices;
  ntriangles_i = surfacedata->ntriangles;
  nvertices_i = surfacedata->nvertices;

  if(nvertices_i==0||ntriangles_i==0)return;
  NewMemory((void **)&xyznorm,3*nvertices_i*sizeof(float));
  NewMemory((void **)&norm,ntriangles_i*sizeof(short));
  NewMemory((void **)&vertexnorm,3*nvertices_i*sizeof(short));
  surfacedata->norm=norm;
  surfacedata->vertexnorm=vertexnorm;
  for(n=0;n<3*nvertices_i;n++){
    xyznorm[n]=0.0;
  }
  for(n=0;n<ntriangles_i/3;n++){
    i1=3*triangles_i[3*n];
    i2=3*triangles_i[3*n+1];
    i3=3*triangles_i[3*n+2];
    v1=vertices_i+i1;
    v2=vertices_i+i2;
    v3=vertices_i+i3;
    calcNormal2(v1,v2,v3,out,&area);
    norm[3*n  ]=(short)(out[0]*32767);
    norm[3*n+1]=(short)(out[1]*32767);
    norm[3*n+2]=(short)(out[2]*32767);
	  xyznorm[i1  ] += out[0]*area;
    xyznorm[i1+1] += out[1]*area;
    xyznorm[i1+2] += out[2]*area;
    xyznorm[i2  ] += out[0]*area;
    xyznorm[i2+1] += out[1]*area;
    xyznorm[i2+2] += out[2]*area;
    xyznorm[i3  ] += out[0]*area;
    xyznorm[i3+1] += out[1]*area;
    xyznorm[i3+2] += out[2]*area;
  }
  for(n=0;n<nvertices_i;n++){
    ReduceToUnit(xyznorm+3*n);
    vertexnorm[3*n  ]=(short)(xyznorm[3*n  ]*32767);
    vertexnorm[3*n+1]=(short)(xyznorm[3*n+1]*32767);
    vertexnorm[3*n+2]=(short)(xyznorm[3*n+2]*32767);
  }
  FREEMEMORY(xyznorm);
}

/* ------------------ GetNormalSurface ------------------------ */

int GetNormalSurface(isosurface *surfacedata){

  float out[3];
  float vertx[3], verty[3], vertz[3];
  float *xnorm=NULL, *ynorm=NULL, *znorm=NULL;
  float *xvert=NULL, *yvert=NULL, *zvert=NULL;
  int *triangles=NULL;
  int ntriangles;
  int nn, n, index;
  
  ntriangles = (surfacedata->ntriangles)/3;
  if(ntriangles==0)return 0;

  xvert = surfacedata->xvert;
  yvert = surfacedata->yvert;
  zvert = surfacedata->zvert;

  triangles = surfacedata->triangles;
  FREEMEMORY(surfacedata->xnorm);
  FREEMEMORY(surfacedata->ynorm);
  FREEMEMORY(surfacedata->znorm);
  if(NewMemory((void **)&surfacedata->xnorm,ntriangles*sizeof(int))==0||
     NewMemory((void **)&surfacedata->ynorm,ntriangles*sizeof(int))==0||
     NewMemory((void **)&surfacedata->znorm,ntriangles*sizeof(int))==0){
    freesurface(surfacedata);
    return 1;
  }

  xnorm = surfacedata->xnorm;
  ynorm = surfacedata->ynorm;
  znorm = surfacedata->znorm;

  nn = 0;
  for(n=0;n<ntriangles;n++){
    index = triangles[nn++];
    vertx[0] = xvert[index]; 
    verty[0] = yvert[index]; 
    vertz[0] = zvert[index];

    index = triangles[nn++];
    vertx[1] = xvert[index]; 
    verty[1] = yvert[index]; 
    vertz[1] = zvert[index];

    index = triangles[nn++];
    vertx[2] = xvert[index]; 
    verty[2] = yvert[index]; 
    vertz[2] = zvert[index];

    calcNormal(vertx,verty,vertz,out);
    xnorm[n]=out[0];
    ynorm[n]=out[1];
    znorm[n]=out[2];
  }
  return 0;

}


/* ------------------ DrawIsosurface ------------------------ */

#ifdef pp_DRAWISO
void DrawIsosurface(const isosurface *surfacedata){

  float shiny=10.;
  GLfloat *color=NULL;
  GLfloat specular[]={0.5,0.5,0.5,1.0};


  float *xvert, *yvert, *zvert, *xnorm, *ynorm, *znorm;
  float x11, y11, z11;
  float x, y, z;
  int *triangles, ntriangles;
  int index,n,nn,nvert;

  xvert = surfacedata->xvert;
  yvert = surfacedata->yvert;
  zvert = surfacedata->zvert;
  nvert = surfacedata->nvertices;
  xnorm = surfacedata->xnorm;
  ynorm = surfacedata->ynorm;
  znorm = surfacedata->znorm;
  triangles = surfacedata->triangles;
  ntriangles = surfacedata->ntriangles;

  nn = 0;
  color = surfacedata->color;
  if(surfacedata->plottype==0){

    glPushAttrib(GL_LIGHTING_BIT);

#ifdef ISO_DEBUG
    glLineWidth(1.0);
    glBegin(GL_LINES);
    nn=0;
    for(n=0;n<ntriangles/3;n++){
      index = triangles[nn];
      x = xvert[index]; y = yvert[index]; z = zvert[index];
      glVertex3f(x,y,z);
      glVertex3f(x+xnorm[n],y+ynorm[n],z+znorm[n]);
      nn +=3;
    }
    glEnd(); 
#endif

    nn = 0;
    glEnable(GL_LIGHTING);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glBegin(GL_TRIANGLES);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,specular);
    for (n = 0; n < ntriangles/3; n++) {
      glNormal3f(xnorm[n],ynorm[n],znorm[n]);

      index = triangles[nn++];
      x = xvert[index]; y = yvert[index]; z = zvert[index];
      glVertex3f(x,y,z);

      index = triangles[nn++];
      x = xvert[index]; y = yvert[index]; z = zvert[index];
      glVertex3f(x,y,z);

      index = triangles[nn++];
      x = xvert[index]; y = yvert[index]; z = zvert[index];
      glVertex3f(x,y,z);
    }
    glEnd();
    glPopAttrib();

  }
  if(surfacedata->plottype==1){
    glColor3fv(color);
    glBegin(GL_LINES);
    for (n = 0; n < ntriangles/3; n++) {
      index = triangles[nn++];
      x = xvert[index]; 
      y = yvert[index]; 
      z = zvert[index];
      glVertex3f(x,y,z);
      x11 = x; 
      y11 = y; 
      z11 = z;

      index = triangles[nn++];
      x = xvert[index]; 
      y = yvert[index]; 
      z = zvert[index];
      glVertex3f(x,y,z);
      glVertex3f(x,y,z);

      index = triangles[nn++];
      x = xvert[index]; 
      y = yvert[index]; 
      z = zvert[index];
      glVertex3f(x,y,z);
      glVertex3f(x,y,z);
      glVertex3f(x11,y11,z11);
    }
    glEnd();
  }
  if(surfacedata->plottype==2){
    glColor3fv(color);
    glBegin(GL_POINTS);
    for (n = 0; n < nvert; n++) {
      x = xvert[n]; 
      y = yvert[n]; 
      z = zvert[n];
      glVertex3f(x,y,z);
    }
    glEnd();
  }

}
#endif

#ifndef pp_DRAWISO

/* ------------------ isoheader ------------------------ */

void CCisoheader(char *isofile, 
                 char *isolonglabel, char *isoshortlabel, char *isounits,
                 float *levels, int *nlevels, int *error){
  int version=1,option=1;
  FILE *isostream=NULL;
  int len[3];


  isostream=fopen(isofile,"wb");

  len[0]=strlen(isolonglabel)+1;
  len[1]=strlen(isoshortlabel)+1;
  len[2]=strlen(isounits)+1;

  fwrite(&version,1,4,isostream);
  fwrite(len,4,3,isostream);
  fwrite(isolonglabel, 1,len[0],isostream);
  fwrite(isoshortlabel,1,len[1],isostream);
  fwrite(isounits,     1,len[2],isostream);
  fwrite(&option,4,1,isostream);
  fwrite(nlevels,4,1,isostream);
  fwrite(levels,4,*nlevels,isostream);
  fclose(isostream);

}


/* ------------------ isoheader ------------------------ */

void CCtisoheader(char *isofile, 
                 char *isolonglabel, char *isoshortlabel, char *isounits,
                 float *levels, int *nlevels, int *error){
  int version=2,option=1;
  FILE *isostream=NULL;
  int len[3];
  int one=1;


  isostream=fopen(isofile,"wb");

  len[0]=strlen(isolonglabel)+1;
  len[1]=strlen(isoshortlabel)+1;
  len[2]=strlen(isounits)+1;

  fwrite(&one,4,1,isostream);  
  fwrite(&version,4,1,isostream);
  fwrite(len,4,3,isostream);
  fwrite(isolonglabel, 1,len[0],isostream);
  fwrite(isoshortlabel,1,len[1],isostream);
  fwrite(isounits,     1,len[2],isostream);
  fwrite(&option,4,1,isostream);
  fwrite(nlevels,4,1,isostream);
  fwrite(levels,4,*nlevels,isostream);
  fclose(isostream);

}

/* ------------------ isoout ------------------------ */

void isoout(FILE *isostream,float t, int timeindex, isosurface *surface, int *error){
	unsigned char czero=0,*trilist1=NULL;
	unsigned short szero=0,*trilist2=NULL;
	int i;
  unsigned short *vertices, *tvertices;
  int nvertices, *trilist, ntrilist;

  vertices=surface->vertices;
  tvertices=surface->tvertices;
  nvertices=surface->nvertices;
  trilist=surface->triangles;
  ntrilist=surface->ntriangles;

	if(timeindex==0)fwrite(&t,4,1,isostream);
	fwrite(&nvertices,4,1,isostream);
	fwrite(&ntrilist,4,1,isostream);
  if(nvertices>0){
    fwrite(vertices,2,3*nvertices,isostream);
    if(tvertices!=NULL){
      fwrite(&surface->tmin,4,1,isostream);
      fwrite(&surface->tmax,4,1,isostream);
      fwrite(tvertices,2,nvertices,isostream);
    }
  }
  if(ntrilist==0)return;
	if(nvertices<256){
    if(NewMemory((void **)&trilist1,sizeof(unsigned char)*ntrilist)==0){
      for(i=0;i<ntrilist;i++){
        fwrite(&czero,1,1,isostream);
      }
    }
    else{
	    for(i=0;i<ntrilist;i++){trilist1[i] = (unsigned char)trilist[i];}
	    fwrite(trilist1,1,ntrilist,isostream);
      FREEMEMORY(trilist1);
    }
	}
	else if(nvertices>=256&&nvertices<65536){
    if(NewMemory((void **)&trilist2,sizeof(unsigned short)*ntrilist)==0){
      for(i=0;i<ntrilist;i++){
        fwrite(&szero,2,1,isostream);
      }
    }
    else{
	    for(i=0;i<ntrilist;i++){trilist2[i] = (unsigned short)trilist[i];}
	    fwrite(trilist2,2,ntrilist,isostream);
      FREEMEMORY(trilist2);
    }
	}
	else{
	  fwrite(trilist,4,ntrilist,isostream);
	}
}

/* ------------------ CCisosurface2file ------------------------ */

void CCisosurface2file(char *isofile, float *t, float *data, int *iblank, 
						float *level, int *nlevels,
                   float *xplt, int *nx, 
                   float *yplt, int *ny, 
                   float *zplt, int *nz,
                   int *isooffset, int *reduce_triangles, int *error
                   ){
  isosurface surface;
  int i;
  FILE *isostream=NULL;

#ifdef _DEBUG
  printf("before surface creation:");
  PrintMemoryInfo;
#endif
  isostream=fopen(isofile,"ab");
  *error = 0;
  for(i=0;i<*nlevels;i++){
    InitIsosurface(&surface,level[i],NULL,i);
    surface.dataflag=0;
    if(GetIsosurface(&surface,data,NULL,iblank,level[i],xplt,*nx,yplt,*ny,zplt,*nz,*isooffset)!=0){
      *error=1;
      return;
    }
    if(GetNormalSurface(&surface)!=0){
      *error=1;
      return;
    }
    if(CompressIsosurface(&surface,*reduce_triangles,
      xplt[0],xplt[*nx-1],
      yplt[0],yplt[*ny-1],
      zplt[0],zplt[*nz-1]
      )!=0){
      *error=1;
      return;
    }

    isoout(isostream,*t,i,&surface,error);

    freesurface(&surface);
  }
  fclose(isostream);
#ifdef _DEBUG
  printf("after surface creation:");
  PrintMemoryInfo;
#endif
}

/* ------------------ CCisosurface2file ------------------------ */

void CCisosurfacet2file(char *isofile, float *t, float *data, int *data2flag, float *data2, int *iblank, 
						float *level, int *nlevels,
                   float *xplt, int *nx, 
                   float *yplt, int *ny, 
                   float *zplt, int *nz,
                   int *isooffset, int *reduce_triangles, int *error
                   ){
  isosurface surface;
  int i;
  FILE *isostream=NULL;
  int dataflag=0;
  float *tdata=NULL;

  if(*data2flag==1){
    dataflag=1;
    tdata=data2;
  }


#ifdef _DEBUG
  printf("before surface creation:");
  PrintMemoryInfo;
#endif
  isostream=fopen(isofile,"ab");
  *error = 0;
  for(i=0;i<*nlevels;i++){
    InitIsosurface(&surface,level[i],NULL,i);
    surface.dataflag=dataflag;
    if(GetIsosurface(&surface,data,tdata,iblank,level[i],xplt,*nx,yplt,*ny,zplt,*nz,*isooffset)!=0){
      *error=1;
      return;
    }
    if(GetNormalSurface(&surface)!=0){
      *error=1;
      return;
    }
    if(CompressIsosurface(&surface,*reduce_triangles,
      xplt[0],xplt[*nx-1],
      yplt[0],yplt[*ny-1],
      zplt[0],zplt[*nz-1]
      )!=0){
      *error=1;
      return;
    }

    isoout(isostream,*t,i,&surface,error);

    freesurface(&surface);
  }
  fclose(isostream);
#ifdef _DEBUG
  printf("after surface creation:");
  PrintMemoryInfo;
#endif
}
#endif

