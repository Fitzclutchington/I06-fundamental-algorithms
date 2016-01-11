/* 
Andrew Fitzgerald
compiles with command line  gcc -o hw4 homework4.c -lX11 -lm -L/usr/X11R6/lib 
run with ./hw1 <test_input_file>
*/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define MAXPOINTS 1000

Display *display_ptr;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
unsigned int display_width, display_height;

Window win;
int border_width;
unsigned int win_width, win_height;
int win_x, win_y;

XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char *win_name_string = "Example Window";
char *icon_name_string = "Icon for Example Window";

XEvent report;

GC gc, gc_red, gc_white;
unsigned long valuemask = 0;
XGCValues gc_values,  gc_red_values,gc_white_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;


int main(int argc, char **argv)
{
  int i,j,x,y,point_count=0,click_count=0,cmd_file=0;
  int v1x, v1y;
  XPoint clicked;
  XPoint points[MAXPOINTS];
  XPoint draw_points[MAXPOINTS];
  int graph[MAXPOINTS][MAXPOINTS];
  int parent[MAXPOINTS];
  char* filename;  
  FILE *fp;
  

  /* open file and scan for triangle vertices*/
  if(argc == 2){
    cmd_file = 1;
    filename = argv[1];
    fp = fopen(filename,"r");
    
    if (fp == NULL){
      printf("There's no input file.\n");
      exit(0);
    }
    
    //save vertices into array
    i = 0;
    point_count = 0;
    while(fscanf(fp, "%d %d\n", &v1x, &v1y) != EOF){
      points[i].x = v1x + 20;
      points[i].y = v1y + 20; 
      point_count++; i++;
    }
  }
  
  /* opening display: basic connection to X Server */
  if( (display_ptr = XOpenDisplay(display_name)) == NULL )
    { printf("Could not open display. \n"); exit(1);}
  printf("Connected to X server  %s\n", XDisplayName(display_name) );
  screen_num = DefaultScreen( display_ptr );
  screen_ptr = DefaultScreenOfDisplay( display_ptr );
  color_map  = XDefaultColormap( display_ptr, screen_num );
  display_width  = DisplayWidth( display_ptr, screen_num );
  display_height = DisplayHeight( display_ptr, screen_num );

  /* creating the window */
  border_width = 10;
  // win_x and win_y determine where the window is displayed on the screen
  // by default
  win_x = 100; win_y = 100;
  win_width = 600;
  win_height = 600; /*rectangular window*/
  
  win= XCreateSimpleWindow( display_ptr, RootWindow( display_ptr, screen_num),
                            win_x, win_y, win_width, win_height, border_width,
                            BlackPixel(display_ptr, screen_num),
                            WhitePixel(display_ptr, screen_num) );

  /* now try to put it on screen, this needs cooperation of window manager */
  size_hints = XAllocSizeHints();
  wm_hints = XAllocWMHints();
  class_hints = XAllocClassHint();
  if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
    { printf("Error allocating memory for hints. \n"); exit(1);}

  size_hints -> flags = PPosition | PSize | PMinSize  ;
  size_hints -> min_width = 60;
  size_hints -> min_height = 60;

  XStringListToTextProperty( &win_name_string,1,&win_name);
  XStringListToTextProperty( &icon_name_string,1,&icon_name);
  
  wm_hints -> flags = StateHint | InputHint ;
  wm_hints -> initial_state = NormalState;
  wm_hints -> input = False;

  class_hints -> res_name = "x_use_example";
  class_hints -> res_class = "examples";

  XSetWMProperties( display_ptr, win, &win_name, &icon_name, argv, argc,
                    size_hints, wm_hints, class_hints );

  /* what events do we want to receive */
  XSelectInput( display_ptr, win, 
            ExposureMask | StructureNotifyMask | ButtonPressMask );
  
  /* finally: put window on screen */
  XMapWindow( display_ptr, win );

  XFlush(display_ptr);

  /* create graphics context, so that we may draw in this window */
  gc = XCreateGC( display_ptr, win, valuemask, &gc_values);
  XSetForeground( display_ptr, gc, BlackPixel( display_ptr, screen_num ) );
  XSetLineAttributes( display_ptr, gc, 1, LineSolid, CapRound, JoinRound);
  
  gc_white = XCreateGC( display_ptr, win, valuemask, &gc_white_values);
  XSetForeground( display_ptr, gc_white, WhitePixel(display_ptr, screen_num) );
  XSetLineAttributes( display_ptr, gc_white, 1, LineSolid, CapRound, JoinRound);

  gc_red = XCreateGC( display_ptr, win, valuemask, &gc_red_values);
  XSetLineAttributes( display_ptr, gc_red, 1, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "red", 
      &tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color red\n"); exit(1);} 
  else
    XSetForeground( display_ptr, gc_red, tmp_color1.pixel );

  /* and now it starts: the event loop */
  while(1)
    { XNextEvent( display_ptr, &report );
      switch( report.type )
  {
  case Expose:
          /* (re-)draw the example figure. This event happens
             each time some part ofthe window gets exposed (becomes visible) */
    //XDrawLine(display_ptr,drawable,gc,x1,y1,x2,y2)
    if(cmd_file){
      for(i=0;i<point_count;i++){
        x = points[i].x;
        y = points[i].y;
        XFillArc( display_ptr, win, gc, 
                x -win_height/100, y- win_height/100,
                win_height/100, win_height/100, 0, 360*64);
      }
      if(point_count>= 2){
        for(i=0;i<point_count;i++){
          for(j=0;j<point_count;j++){
            if(i != j){
              XDrawLine(display_ptr, win, gc, points[i].x, points[i].y, points[j].x, points[j].y);
            }
          }
        }
      }
    }
    break;

  case ConfigureNotify:
    /* This event happens when the user changes the size of the window*/
    win_width = report.xconfigure.width;
    win_height = report.xconfigure.height;
    break;
  case ButtonPress: 
    {
    x = report.xbutton.x;
    y = report.xbutton.y;
    clicked.x = x;
    clicked.y = y;
    
    if (report.xbutton.button == Button1 && !cmd_file){
      points[point_count] = clicked;
      point_count++;
      XFillArc( display_ptr, win, gc, 
                x -win_height/100, y- win_height/100,
                win_height/100, win_height/100, 0, 360*64);

      if(point_count>= 2){
        for(i=0;i<point_count;i++){
          for(j=0;j<point_count;j++){
            if(i != j){
              XDrawLine(display_ptr, win, gc, points[i].x, points[i].y, points[j].x, points[j].y);
            }
          }
        }
      }
    }

    //if right mouse is clicked
    else{
      continue;
    }  
  }
    break;
  default:
    /* this is a catch-all for other events; it does not do anything.
    One could look at the report type to see what the event was */ 
    break;
  }
}
}