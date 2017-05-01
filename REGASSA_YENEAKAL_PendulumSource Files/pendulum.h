/****************************PENDULUM SIMULATION***********************************
 * Project Name: PENDULUM SIMULATION WITH COLLISION                               *
 * Author: REGASSA, YENEAKAL GIRMA 						  *
 * Purpose: Real Time Systems Course Project		                          *
 **********************************************************************************/

#ifndef PENDULUM_H_
#define PENDULUM_H_

#include <stdlib.h> 
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <allegro.h>
#include <time.h>

//-----------------------------------------------------
// GRAPHICS CONSTANTS
//-----------------------------------------------------
#define XWIN   640   /* window x resolution	*/
#define YWIN   480   /* window y resolution	*/
#define BKG    0     /* background color	*/
#define MCOL   14    /* menu color	*/
#define NCOL   7     /* numbers color	*/
//-----------------------------------------------------
#define LBOX   630  /* X length of the pendulum box	*/
#define HBOX   424  /* Y height of the pendulum box	*/
#define XBOX   5    /* left box X coordinate	*/
#define YBOX   50   /* upper box Y coordinate	*/
#define RBOX   635  /* right box X coordinate	*/
#define BBOX   475  /* bottom box Y coordinate	*/
#define MAR_X  5    /* horizontal margin from screen */
#define MAR_Y  5    /* vertical margin from screen */

#define	UL_X	MAR_X			/* UL top left corner x coordinates */
#define UL_Y	MAR_Y			/* UL top left corner y coordinates */
#define UL_W	XWIN			/* UL width */
#define UL_H	35			/* UL height */

#define BL_X	MAR_X			/* BL top left corner x coordinates */
#define BL_Y	(2 * MAR_Y + UL_H)	/* BL top left corner y coordinates */
#define BL_W	UL_W			/* BL width */
#define BL_H	(YWIN - MAR_Y - BL_Y)	/* BL height */


//-----------------------------------------------------
// PENDULUM CONSTANTS
//-----------------------------------------------------
#define MAX_PENDULUM          3             /* max number of balls	*/
#define g                     9.8           /* acceleration of gravity	*/
#define PI                    3.141592654
#define TSCALE                10            /* time scale factor	*/
#define MIN_MASS_OF_BOB       7             /* Minimum weight of the bob	*/
#define MAX_MASS_OF_BOB       15            /* Maximum weight of the bob	*/
#define DUMP                  0.99          /* Dumping factor as friction	*/
#define Acc_Collision         0.1           /* Amount of acceleretion after collision	*/
#define Initial_Velocity      0.2           /* Initial Angular Velocity user applies during key press	*/



struct task_par {
	int	arg;	        	/* task argument	*/
	long	wcet;	        	/* in microseconds	*/
	int  	period;	        	/* in milliseconds	*/
	int  	deadline;			/* relative (ms) 	*/
	int  	priority;			/* in [0,99] */
	int  	dmiss;				/* no. of misses	*/
	struct 	timespec at;		/* next activ. time */
	struct 	timespec dl;		/* abs deadline 	*/
};

/*************************************************************
 *NAME:pen_structure                                         *
 *PURPOSE: To define pendulum  structure                     *
 *                                                           *
 *************************************************************/
typedef struct  pen_structure 
{ 
	int   c;          /* color [1,15]	*/
	float length;     /* length of the string(m)	*/
	float mass;       /* bob mass	*/
	float r;          /* radius of the bob (m)	*/
	float origin_x;   /* x coordinate for the pivot origin	*/
	float origin_y;   /* y coordinate for the pivot origin	*/
	float end_x;      /* x coordinate for string end and bob center	*/
	float end_y;      /* y coordinate for string end and bob center	*/
	float theta;      /* pendulum angle (rad)	*/
	float omega;      /* pendulum angular velocity velocity (m/s)	*/
	float alpha;      /* pendulum angular acceleration	*/
}Mypendulum;

Mypendulum pendulum[MAX_PENDULUM];

pthread_mutex_t mutexp;



#define PERIOD_TO_UPDATE      70	/* in microseconds */
#define PERIOD_TO_DETECT_KEY  80	/* in microseconds */



struct timespec tq;       /* represents the starting time for pendulum updtae thread	*/
struct timespec tr;       /* represents the starting time for key interprator thread	*/
struct timespec td;       /* represents the starting time for key interprator thread	*/
int end =0 ;


/*	FUNCTION PROTOTYPES	*/

void graphics_Initializer();
void init_pendulum(int i);
int  read_PendulumFile();
static int finalize(void);

void  pendulum_thread_creator();
void  calculate_position(int i);
void  draw_pendulum(int i);
void  erase_pendulum(int i);
void  display_info();
void  *pendulum_update(void *arg);
void  *input_task();
void  *display_task();
float frand(float xmi, float xma);
void  handle_collision(int i);

void time_copy(struct timespec *td, struct timespec ts);
void time_add_ms(struct timespec *t, int ms);
int  time_cmp(struct timespec t1, struct timespec t2);
void set_period(struct task_par *tp);
void wait_for_period(struct task_par *tp);

#endif
