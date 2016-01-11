#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

typedef struct Triangles{
    XPoint a;
    XPoint b;
    XPoint c;
} Triangle;

int orientation(XPoint a, XPoint b, XPoint c){
  return (a.x*b.y + b.x*c.y + c.x*a.y - a.y*b.x - b.y*c.x - c.y*a.x);
}

int intersect_test(XPoint p,XPoint q, XPoint r, XPoint s){
  if( (orientation(p,q,r) * orientation(p,q,s) < 0) && (orientation(r,s,p) * orientation(r,s,q) < 0)){
    return 1;
  }
  else{
    return 0;
  }
}

int main(){
  XPoint p,q,a,b;
  p.x = 200;
  p.y = 110;
  q.x = 170;
  q.y = 130;
  a.x = 20;
  a.y = 20;
  b.x = 25;
  b.y = 320;

  printf("orientation pqa*pqb %d\n", orientation(p,q,a)*orientation(p,q,b));
  printf("orientation abp*abq %d\n", orientation(a,b,p)*orientation(a,b,q)); 
}