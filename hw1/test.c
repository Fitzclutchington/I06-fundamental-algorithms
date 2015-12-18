/* compiles with command line  gcc test.c -lX11 -lm -L/usr/X11R6/XSetLineAttributes */

/* 
  Compiles with command line  gcc -o assign1 hw1.c -lX11 -lm -L/usr/X11R6/lib 
  Run : ./assign input.txt
  Homework #1
  Wan Kim Mok
  Due: September 30, 2015
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(){
  int i,j;
  for(j=0;j<10;j++){
    printf("%d \n",j);
    for(i=0;i<40;i+=2){
      if(i==10){
        break;
    }
    printf("%d \n",i);
  }
}
}