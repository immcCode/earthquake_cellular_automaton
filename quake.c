# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <math.h>
# include <time.h>
# include <sys/time.h>
# include <unistd.h>

# define NX 50
# define NY 50
#define ANSI_COLOR_RED     "\x1b[41m"
#define ANSI_COLOR_GREEN   "\x1b[42m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_LTGRAY  "\x1b[100m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define CLEAR_SCREEN       "\e[1;1H\e[2J"

typedef struct {
  char STATE;
  double B,D;
  int T;
  int i,j;
} area;

void timestamp ( void );
void showquake (int, int, area**);
void showquake_persist (int, int, area**);
void p(const char *str);

int main ( void )
{
  int i;
  int j;
  int nx = NX;
  int ny = NY;
  int NSTEPS = 100;
  struct timeval start_time, stop_time, elapsed_time;

  area **t,**tnew;

  t = malloc(nx*sizeof(area *));
  *t = malloc(nx*ny*sizeof(area));
  for (i = 0; i < nx; i++)
    *(t+i) = *t + i*ny;

  tnew = malloc(nx*sizeof(area *));
  *tnew = malloc(nx*ny*sizeof(area));
  for (i = 0; i < nx; i++)
    *(tnew+i) = *tnew + i*ny;

  srand(time(NULL));
  double Dinit=1.;

  gettimeofday(&start_time,NULL);

  /* Initialize area -- no earthquake yet */
  for ( j = 0; j < ny; j++ )
  {
    for ( i = 0; i < nx; i++ )
    {
      // Cell sites are aware of their own location.
      tnew[i][j].i = i;
      tnew[i][j].j = j;

      // No rupture possibility around edge of area
      if( i == 0 || i == 1 || j == 0 || j == 1 || i == nx-1 || i == nx-2 || j == ny-1 || j == ny-2)
      {
        tnew[i][j].STATE = ' ';
      }
      else
      {
        // populate area with strain
        t[i][j].B = tnew[i][j].B = 0.98;// (double) rand() /RAND_MAX;
        t[i][j].D = tnew[i][j].D = (double) rand() / ((RAND_MAX + 1.)/t[i][j].B);
        t[i][j].T = tnew[i][j].T = -1;
        t[i][j].STATE = tnew[i][j].STATE = '_';
      }
    }
  }

  /* Start an earthquake in the middle of the grid */

  /* Constant source earthquake */ //   tnew[nx/2][ny/2].B = 1;
  tnew[nx/2][ny/2].STATE = '!';
  tnew[nx/2][ny/2].D = t[nx/2][ny/2].B;
  tnew[nx/2][ny/2].T = t[nx/2][ny/2].T + 1;


  showquake(nx,ny,tnew);

  /* Let it shake */
  int step,li,lj;
  int total,shaking,broken;

  for (step=0 ;step<NSTEPS ;step++ )
  {
    total = 0;
    shaking = 0;
    broken = 0;
    /*
     *   Save the current earthquake state.
     *
     */
    for ( j = 0; j < ny; j++ )
    {
      for ( i = 0; i < nx; i++ )
      {
        t[i][j].STATE = tnew[i][j].STATE;
      }
    }

    //          memcpy(&t,&tnew,sizeof(tnew));
    //memcpy(t,tnew,sizeof(tnew));
    /*
     *  Scan for neighbors to redistribute shocks
     *
     */
    for ( j = 2; j < ny-2; j++ )
    {
      for ( i = 2; i < nx-2; i++ )
      {
        if ( t[i][j].STATE == '!' )
        {
          // Only check neighbors
          for(lj = -2;lj < 3; lj++)
          {
            for(li = -2;li < 3; li++)
            {
              // If not the fracture cell
              if ( !(lj == 0 && li == 0) )
              {
                if ( t[i+li][j+lj].STATE == '_' )
                {
                  // If one of the 4 nearest neighbors (N, S, E, W) & time = 0
                  if(1 == abs(lj+li) && t[i][j].T == 0)
                  {
                    // Redistribute load to neighbors
                    tnew[i+li][j+lj].D = t[i+li][j+lj].D + (t[i+li][j+lj].B/4);
                    tnew[i+li][j+lj].STATE = '^';
                  }
                  // If one of the 4 neighbors(NE, NW, SE, SW) & time = 1
                  if(2 == abs(lj+li) && t[i][j].T == 1)
                  {
                    // Redistribute load to neighbors
                    tnew[i+li][j+lj].D =  t[i+li][j+lj].D + (t[i+li][j+lj].B/6);
                    tnew[i+li][j+lj].STATE = '^';
                  }
                  // If one of the 4 neighbors once removed (N, S, E, W) & time = 2
                  if(2 == abs(lj+li) && t[i][j].T == 2)
                  {
                    // Redistribute load to neighbors
                    tnew[i+li][j+lj].D =  t[i+li][j+lj].D + (t[i+li][j+lj].B/12);
                    tnew[i+li][j+lj].STATE = '^';
                  }
                  // If one of the 4 neighbors once removed (NE, NW, SE, SW) & time = 3
                  if(3 == abs(lj+li) && t[i][j].T == 3)
                  {
                    // Redistribute load to neighbors
                    tnew[i+li][j+lj].D =  t[i+li][j+lj].D + (t[i+li][j+lj].B/12);
                    tnew[i+li][j+lj].STATE = '^';
                  }
                }
              }
              // Assign new strain to fracture cell and update timer
              else
              {
                if(tnew[i+li][j+lj].T == 0)
                  {
                    tnew[i+li][j+lj].D = t[i+li][j+lj].D - t[i+li][j+lj].B;
                  }
                if(tnew[i+li][j+lj].T == 1)
                  {
                    tnew[i+li][j+lj].D = t[i+li][j+lj].D - ((2/3)*t[i+li][j+lj].B);
                  }
                if(tnew[i+li][j+lj].T == 2 || tnew[i+li][j+lj].T == 3)
                  {
                    tnew[i+li][j+lj].D = t[i+li][j+lj].D - (t[i+li][j+lj].B/3);
                  }
                tnew[i+li][j+lj].T = t[i+li][j+lj].T +1;
              }
            }
          }
          // Recheck neighbors to see if redistribution of strain caused any fractures
          for(lj = -2;lj < 3; lj++)
          {
            for(li = -2;li < 3; li++)
            {
              // If not the fracture cell
              if ( !(lj == 0 && li == 0) )
              {
                  // If the new strain is over the threshold, fracture and initiate timer
                  if(tnew[i+li][j+lj].D >= tnew[i+li][j+lj].B)
                  {
                    tnew[i+li][j+lj].STATE = '!';
                    tnew[i+li][j+lj].T = t[i+li][j+lj].T + 1;
                  }
                }
              }
            }
          }
        }

        total++;
        if ( t[i][j].STATE == '!' )
        {
          broken++;
        }
        if ( t[i][j].STATE == '!' || t[i][j].STATE == '^'  )
        {
          shaking++;
        }
      }
    }
    // Uncomment for testing
    printf("%d %d %d %d %f %f\n",step,total, broken, shaking,broken/(2.*step),1.*shaking/(step*step));

    showquake(nx,ny,tnew);
  }
  gettimeofday(&stop_time,NULL);

  showquake_persist(nx,ny,tnew);
  printf ( "\n" );
  printf ( "Castellaro et al.,\n \t A simple but effective cellular automaton for earthquakes \n \tGeophysical Journal International: \n\tVolume 144, Issue 3, 1 March 2001, \n\thttps://doi.org/10.1046/j.1365-246x.2001.01350.x\n" );

  printf ( "\n" );
  timestamp( );
  timersub(&stop_time, &start_time, &elapsed_time);

  free(*t);
  free(*tnew);
  free(t);
  free(tnew);
  return 0;
}

/******************************************************************************/

void timestamp ( void )
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );

  strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  printf ( "%s\n", time_buffer );

  return;
# undef TIME_SIZE
}

/******************************************************************************/
void showquake(int nx, int ny, area **tnew)
{
  int i,j;
  for ( j = 0; j < ny; j++ )
  {
    for ( i = 0; i < nx; i++ )
    {
      if (tnew[i][j].STATE == '!')
      {
        printf(ANSI_COLOR_RED"%c "ANSI_COLOR_RESET,tnew[i][j].STATE);
      }
      else if (tnew[i][j].STATE == '^')
      {
        printf(ANSI_COLOR_GREEN"%c "ANSI_COLOR_RESET,tnew[i][j].STATE);
      }
      else if (tnew[i][j].STATE == '_')
      {
        printf(ANSI_COLOR_LTGRAY"%c "ANSI_COLOR_RESET,tnew[i][j].STATE);
      }
      else
      {
        printf("%c ",tnew[i][j].STATE);
      }
    }
    printf("\n");
  }
  sleep(1);
  printf(CLEAR_SCREEN);
}

/******************************************************************************/
void showquake_persist(int nx, int ny, area **tnew)
{
  int i,j;
  for ( j = 0; j < ny; j++ )
  {
    for ( i = 0; i < nx; i++ )
    {
      if (tnew[i][j].STATE == '!')
      {
        printf(ANSI_COLOR_RED"%c "ANSI_COLOR_RESET,tnew[i][j].STATE);
      }
      else if (tnew[i][j].STATE == '^')
      {
        printf(ANSI_COLOR_GREEN"%c "ANSI_COLOR_RESET,tnew[i][j].STATE);
      }
      else if (tnew[i][j].STATE == '_')
      {
        printf(ANSI_COLOR_LTGRAY"%c "ANSI_COLOR_RESET,tnew[i][j].STATE);
      }
      else
      {
        printf("%c ",tnew[i][j].STATE);
      }
    }
    printf("\n");
  }
}

/******************************************************************************/
