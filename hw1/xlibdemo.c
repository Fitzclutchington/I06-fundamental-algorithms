/* compiles with command line  gcc -o hw1 xlibdemo.c -lX11 -lm -L/usr/X11R6/lib */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <math.h>

typedef struct Triangles{
    XPoint a;
    XPoint b;
    XPoint c;
} Triangle;

int orientation(XPoint a, XPoint b, XPoint c);
int intersect_test(XPoint p,XPoint q, XPoint r, XPoint s);
int euclid_distance(XPoint a, XPoint b);

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

GC gc, gc_yellow, gc_red, gc_grey;
unsigned long valuemask = 0;
XGCValues gc_values, gc_yellow_values, gc_red_values, gc_grey_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;


int main(int argc, char **argv)
{
  int i,j,t,point_count,triangle_count,click_count=0;
  int v1x, v1y, v2x, v2y, v3x, v3y;
  int max_x, max_y = 0;
  float boundary_x, boundary_y = 0;
  XPoint points[1000];
  XPoint a,b,c,p,q;
  Triangle triangle_points[1000];
  char* filename = argv[1];  
  FILE *fp;
  

  /* open file and scan for triangle vertices*/
  fp = fopen(filename,"r");
  
  if (fp == NULL){
    printf("There's no input file.\n");
    exit(0);
  }

  i = 0;
  point_count = 0;
  while(fscanf(fp, "T (%d,%d) (%d,%d) (%d,%d)\n", &v1x, &v1y, &v2x, &v2y, &v3x, &v3y) != EOF){
    points[i].x = v1x;
    points[i].y = v1y; 
    point_count++; i++;
    points[i].x = v2x;
    points[i].y = v2y;
    point_count++; i++; 
    points[i].x = v3x;
    points[i].y = v3y;
    point_count++; i++;
  }
  
  //determine maximum x and y values
  for(i=0;i<point_count;i++){
    if(points[i].x>max_x){
      max_x = points[i].x;
    }
    if(points[i].y>max_y){
      max_y = points[i].y;
    }
  }

  //add 10% of maximum to all points
  boundary_x = max_x * 0.1;
  boundary_y = max_y * 0.1;
  for(i=0;i<point_count;i++){
    points[i].x += boundary_x;
    points[i].y += boundary_y;
  }
  
  //create array of triangles
  triangle_count = 0;
  for(i=0;i<point_count;i+=3){
    triangle_points[i].a = points[i];
    triangle_points[i].b = points[i+1];
    triangle_points[i].c = points[i+2];
    triangle_count++;
  }
  
  int graph[point_count+2][point_count+2];
  
  printf("triange a:(%d,%d)", triangle_points[0].a.x, triangle_points[0].a.y );
  /* opening display: basic connection to X Server */
  if( (display_ptr = XOpenDisplay(display_name)) == NULL )
    { printf("Could not open display. \n"); exit(-1);}
  printf("Connected to X server  %s\n", XDisplayName(display_name) );
  screen_num = DefaultScreen( display_ptr );
  screen_ptr = DefaultScreenOfDisplay( display_ptr );
  color_map  = XDefaultColormap( display_ptr, screen_num );
  display_width  = DisplayWidth( display_ptr, screen_num );
  display_height = DisplayHeight( display_ptr, screen_num );

  printf("Width %d, Height %d, Screen Number %d\n", 
           display_width, display_height, screen_num);

  /* creating the window */
  border_width = 10;
  // win_x and win_y determine where the window is displayed on the screen
  // by default
  win_x = 100; win_y = 100;
  win_width = display_width/2;
  win_height = (int) (win_width / 1.7); /*rectangular window*/
  
  win= XCreateSimpleWindow( display_ptr, RootWindow( display_ptr, screen_num),
                            win_x, win_y, win_width, win_height, border_width,
                            BlackPixel(display_ptr, screen_num),
                            WhitePixel(display_ptr, screen_num) );

  /* now try to put it on screen, this needs cooperation of window manager */
  size_hints = XAllocSizeHints();
  wm_hints = XAllocWMHints();
  class_hints = XAllocClassHint();
  if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
    { printf("Error allocating memory for hints. \n"); exit(-1);}

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
  XSetLineAttributes( display_ptr, gc, 2, LineSolid, CapRound, JoinRound);

  gc_red = XCreateGC( display_ptr, win, valuemask, &gc_red_values);
  XSetLineAttributes( display_ptr, gc_red, 6, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "red", 
      &tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color red\n"); exit(-1);} 
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
  for(i=0;i<point_count;i+=3){
    XDrawLine(display_ptr, win, gc, points[i].x, points[i].y, points[i+1].x, points[i+1].y);
    XDrawLine(display_ptr, win, gc, points[i+1].x, points[i+1].y, points[i+2].x, points[i+2].y );
    XDrawLine(display_ptr, win, gc, points[i].x, points[i].y, points[i+2].x, points[i+2].y);
  }
    

          break;
        case ConfigureNotify:
          /* This event happens when the user changes the size of the window*/
          win_width = report.xconfigure.width;
          win_height = report.xconfigure.height;
          break;
        case ButtonPress:
          {  
             int x, y;
             XPoint clicked;
             x = report.xbutton.x;
             y = report.xbutton.y;
             clicked.x = x;
             clicked.y = y;
             if (report.xbutton.button == Button1 ){
              if(click_count == 0){
                XFillArc( display_ptr, win, gc_red, 
                          x -win_height/40, y- win_height/40,
                          win_height/40, win_height/40, 0, 360*64);
                points[point_count] = clicked;
                point_count++;
                click_count++;
              }
              else if(click_count == 1){
                XFillArc( display_ptr, win, gc_red, 
                          x -win_height/40, y- win_height/40,
                          win_height/40, win_height/40, 0, 360*64);
                points[point_count] = clicked;
                point_count++;
                click_count++;
                // build graph
                //  iterate through all pairs of points
                for(i=0;i<point_count;i++){
                  for(j=0;j<point_count;j++){
                    p = points[i];
                    q = points[j];
                    // iterate through triangles to see if there exists an intersection between the
                    // two points
                    for(t=0;t<triangle_count;t++){
                      a = triangle_points[t].a;
                      b = triangle_points[t].b;
                      c = triangle_points[t].c;
                      if(intersect_test(p,q,a,b) || intersect_test(p,q,a,c) || intersect_test(p,q,b,c)){
                        printf("Intersection \n");
                        break;
                      } 
                    }
                  // if no intersection exists, add edge to graph
                  if(t == triangle_count){

                  }
                  }
                }
              }

            }
          }
          break;
        default:
    /* this is a catch-all for other events; it does not do anything.
             One could look at the report type to see what the event was */ 
          break;
  }

    }
  exit(0);
}

int orientation(XPoint a, XPoint b, XPoint c){
  return (a.x*b.y + b.x*c.y + c.x*a.y - a.y*b.x - b.y*c.x - c.y*a.x);
}

int intersect_test(XPoint p,XPoint q, XPoint r, XPoint s){
  if( orientation(p,q,r) * orientation(p,q,s) < 0 && orientation(r,s,p) * orientation(r,s,q) < 0){
    return 1;
  }
  else{
    return 0;
  }
}

int euclid_distance(XPoint a, XPoint b){
  x_term = a.x - b.x;
  y_term = a.y - b.y;
  return (int) sqrt(x_term*x_term + y_term * y_term);  
}