

#include "pendulum.h"


int main()
{
	/* getting the time for random number generation */
	srand(time(NULL));

	pthread_mutex_init(&mutexp, NULL);    

	graphics_Initializer();
	
	pendulum_thread_creator();

	while (!end);

	finalize();

	return 0;       
}

/***********************************************************
 *NAME:   graphics_Initializer                             *
 *PURPOSE:To initialize all neccessary allegro libraries   *
 *INPUT:  N/A                                              *
 *RETURN: N/A                                              *
 ***********************************************************/

void graphics_Initializer()
{
	/* initializing allegro */
	allegro_init();
	/* initializing the keyboard */
	install_keyboard();
	/* initializing the timer */
	install_timer();	
	/* setting the gfx mode */
	int ret = set_gfx_mode(GFX_SAFE, XWIN, YWIN, 0, 0); 
	
		if (ret != 0)
		{
			allegro_message("could not set video mode");
			allegro_exit();
			return;
		}    
	/* setting the colr depth */
	set_color_depth(16);
	clear_to_color(screen, 0);
	/* setting the window title */
	set_window_title("PENDULUM SIMULATION!"); 
	
}

/****************************************************************************
 *NAME:   read_PendulumFile                                                 *
 *PURPOSE:To initialize mass and legth by reading values from a text file   *
 ****************************************************************************/

int read_PendulumFile()
{
	FILE *fp = fopen("pendulum_input.txt", "r");
		   
	int	i;
	
		for (i = 0; i < MAX_PENDULUM; i++) {
			fscanf(fp, "%f %f", &pendulum[i].length, &pendulum[i].mass );
			printf("%f \n%f\n",  pendulum[i].length, pendulum[i].mass);
		}

	fclose(fp);
	return 0;
}

/****************************************************************************************************
 *NAME: init_pendulum                                                                               *
 *PURPOSE:To initialize the pendulum structure members for the first appearnce position of pendulum *
                                                                                                    *
 *param   index i of the pendulum structure to be initialized in the array of pendulum structure    *
 *RETURN: N/A                                           	                                    	*
 ***************************************************************************************************/

 void init_pendulum(int	i)
{
	pthread_mutex_lock (&mutexp);
	
	read_PendulumFile();                                /* calls this function to initialize mass and length of each pendulum	*/
	
	pendulum[i].c = 2 + i % 14 ;                        /* color in [2,15]	*/
	pendulum[i].r = pendulum[i].mass;                   /* radius of the bob	*/

		if( i == 0) {
		   pendulum[i].origin_x = MAR_X + 130 + pendulum[i].r; }           /* initializing x_origin for the first pendulum	*/
		else if ( i==1 ) {
		   pendulum[i].origin_x = XWIN/2;}                                 /* initializing x_origin for the second pendulum	*/
		else{
		   pendulum[i].origin_x = LBOX - (130 + pendulum[i].r); }          /* initializing x_origin for the third pendulum	*/

	pendulum[i].origin_y = BL_Y + 200;                                    /* initializing y_origin for all pendulum		*/	
	pendulum[i].end_x = pendulum[i].origin_x;                             /* initializing x_end for all pendulum		*/
	pendulum[i].end_y = pendulum[i].origin_y + pendulum[i].length ;       /* initializing y_end for all pendulum		*/
	pendulum[i].theta = 0;                                                /* setting up initial angle*/
	pendulum[i].omega = 0;                                                /* setting initial angular velocity		*/
	pendulum[i].alpha = 0;                                                /* setting initial angular acceleration		*/
        
	pthread_mutex_unlock (&mutexp);
    
}


/****************************************************************************************************
 *NAME:   handle_collision                                                                          *
 *PURPOSE:To handle collision between adjecent pendulums and by analyzing the reaction              *
 *         provides the corresponding new appearnce position of pendulum                            *
                                                                                                    *
 *param  index i of the pendulum structure to be analyzed in the array of pendulum structure        *
 *RETURN: N/A                                           	                                    *
 ***************************************************************************************************/

 void handle_collision(int i)
{        
	float xdiff, ydiff, radii;

		if (i == 0) {
			pthread_mutex_lock (&mutexp);

			xdiff = pendulum[i].end_x - pendulum[i+1].end_x;
		
			ydiff = pendulum[i].end_y - pendulum[i+1].end_y;

			radii = pendulum[i].r + pendulum[i+1].r + 5;

				if	( ( (xdiff * xdiff) + (ydiff * ydiff) ) <= (radii * radii) ) {
					pendulum[i].omega -= Acc_Collision ;
					printf("\n collision between  pendulum 1 and 2 occurs");
				}

			pthread_mutex_unlock (&mutexp);
		}
		else if	(i == sizeof(pendulum)-1) {
					 
			pthread_mutex_lock (&mutexp);

			xdiff = pendulum[i].end_x - pendulum[i-1].end_x;
		
			ydiff = pendulum[i].end_y - pendulum[i-1].end_y;

			radii = pendulum[i].r + pendulum[i-1].r + 5;

				if	( ( (xdiff * xdiff) + (ydiff * ydiff) ) <= (radii * radii) ) {
					pendulum[i].omega += Acc_Collision ;
					printf("\n collision between 3 and 2 occurs");
				}

			pthread_mutex_unlock (&mutexp);
		}

		else{   
			pthread_mutex_lock (&mutexp);

			/* first collision detection with previous pend */
			xdiff = pendulum[i].end_x - pendulum[i-1].end_x;
		
			ydiff = pendulum[i].end_y - pendulum[i-1].end_y;

			radii = pendulum[i].r + pendulum[i-1].r + 5;

				if	( ( (xdiff * xdiff) + (ydiff * ydiff) ) <= (radii * radii) ) {
					pendulum[i].omega += Acc_Collision ;
					printf("\n collision between 2 and 1 occurs");
				}

			/* second collision detection with next pend */
			xdiff = pendulum[i].end_x - pendulum[i+1].end_x;
		
			ydiff = pendulum[i].end_y - pendulum[i+1].end_y;

			radii = pendulum[i].r + pendulum[i+1].r + 5;

				if	( ( (xdiff * xdiff) + (ydiff * ydiff) ) <= (radii * radii) ) {
					pendulum[i].omega -= Acc_Collision  ;
					printf("\n collision  between 2 and 3 occurs");
				}

			pthread_mutex_unlock (&mutexp);
		}         
}

/**************************************************************************
 *NAME:pendulum_update                                                    *
 *PURPOSE:To update the pendulum position and redraw in updated position  *
 **************************************************************************/

 void *pendulum_update(void *arg)
{

	struct task_par	*tp = (struct task_par *)arg;

	int i = tp->arg;
	int pen_period = tp->period;
	float dt;
	
	set_period(tp);

	init_pendulum(i);
       
           
		while (!end) { 

			erase_pendulum(i);

			calculate_position(i);

			handle_collision(i);

			draw_pendulum(i);

			wait_for_period(tp);  
		}
}


/**************************************************************************************************************************
 *NAME:    input task                                                                                                     *
 *PURPOSE: task which detects  and interprates key press and changes the parameter value accordingly every 100ms          *
 *INPUT: N/A													                														          *
 *RETURN: N/A                                           	                                                          *
 **************************************************************************************************************************/

void *input_task(void *arg)
{
        
	char	scan;
	int		i = 0;
	struct 	task_par *ti;
	ti = (struct task_par *)arg;
	
	set_period(ti);
      
		while (!end) {
				  

			if (keypressed()) {
				scan = readkey() >> 8;
				pthread_mutex_lock (&mutexp);

				switch (scan) {
					case KEY_1:
								 
						pendulum[i].omega += Initial_Velocity;
						break;
					case KEY_2:
								 
						pendulum[i].omega -= Initial_Velocity;
						break;
					case KEY_3:
					
						pendulum[i+1].omega += Initial_Velocity;
						break;
					case KEY_4:
					
						pendulum[i+1].omega -= Initial_Velocity;
						break;
					case KEY_5:
					
						pendulum[i+2].omega += Initial_Velocity;
						break;
					case KEY_6:
					
						pendulum[i+2].omega -= Initial_Velocity;
						break;
				
					case KEY_ESC:
						end = 1;
						break;
					default:
						break;
				}
				pthread_mutex_unlock (&mutexp);

				wait_for_period(ti);
					  
			
			}
		}

}


/************************************************************************************
 *NAME: display_task	                              		                    *
 *PURPOSE: To display basic information on the screen                               *
 *RETURN: N/A								            *
 ************************************************************************************/

 void *display_task(void *arg)
{

	struct task_par *td;
	td = (struct task_par *)arg;
	
	set_period(td);
  
		while (!end) {
			display_info();
			wait_for_period(td);
		}
}

/********************************************************************************
 * Shutdown allegro and the other subsystems                                    *
 * return	0 on success; != 0 on failure;                                  *
*********************************************************************************/

static int finalize(void)
{
	allegro_exit();
	pthread_exit(NULL);  /* must be here otherwise it makes the program freez */
	                     
	return 0;
}

/***********************************************************************
 *NAME:pendulum_thread_creator                                         *
 *PURPOSE:To create the pendulums and the key interprator thread       *
 *INPUT:  N/A                                                          *
 *RETURN: N/A                                                          *
 ***********************************************************************/
void pendulum_thread_creator()
{
	struct task_par		tp[MAX_PENDULUM], tid;

	struct sched_param	mypar, keypar;

	pthread_attr_t 	 	att[MAX_PENDULUM], attkey;
    
	pthread_t		pendulum_t[MAX_PENDULUM], key_interprator, display_update; /*Declaration of pendulums tasks */

	int  pen_created_err, key_interp_created_err, display_err, i;

	   /*creating pendulum tasks */
		for	(i=0; i<MAX_PENDULUM; i++) {
			tp[i].arg = i;
			tp[i].period = PERIOD_TO_UPDATE;	
			tp[i].deadline = 70;
			tp[i].priority = 20;
			tp[i].dmiss = 0;

			pthread_attr_init(&att[i]);
			pthread_attr_setinheritsched(&att[i], PTHREAD_EXPLICIT_SCHED);
			pthread_attr_setschedpolicy(&att[i], SCHED_FIFO);
			mypar.sched_priority = tp[i].priority;
			pthread_attr_setschedparam(&att[i], &mypar);

			pen_created_err = pthread_create(&pendulum_t[i], &att[i], pendulum_update, &tp[i]);

				if	(pen_created_err) {		
					allegro_message("ERROR on creating Pendulum!");
					allegro_exit();
					exit(-1);
				}
		}

	tid.arg = 0;
	tid.period = PERIOD_TO_DETECT_KEY;	
	tid.deadline = 80;
	tid.priority = 19;
	tid.dmiss = 0;

	pthread_attr_init(&attkey);
	pthread_attr_setinheritsched(&attkey, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attkey, SCHED_FIFO);
	keypar.sched_priority = tid.priority;
	pthread_attr_setschedparam(&attkey, &keypar);

	/* creating key interprator task*/
	key_interp_created_err = pthread_create(&key_interprator, &attkey, input_task, &tid); 
		if	(key_interp_created_err) {		
			allegro_message("ERROR on creating Key Interprator thread!");
			allegro_exit();
			exit(-1);
		}

	/* creating Display task*/
	display_err = pthread_create(&display_update, &attkey, display_task, &tid); 
		if	(display_err) {		
			allegro_message("ERROR on Display updating!");
			allegro_exit();
			exit(-1);
		}

		/* joining all threads that are created above */
		for	(i = 0; i < MAX_PENDULUM; i++) {
			pthread_join(pendulum_t[i], NULL);
		}
		pthread_join(key_interprator, NULL);
		pthread_join(display_update, NULL);

		/* Deleting all thread attributes used that are created above */
		for	(i = 0; i < MAX_PENDULUM; i++)	{
			pthread_attr_destroy(&att[i]);
		}
		pthread_attr_destroy(&attkey);
		
	pthread_mutex_destroy(&mutexp);
	       
 }

/*************************************************************************************
 *NAME: calculate_position	                              		             *
 *PURPOSE:To calculate the new position                                              *
 *RETURN: N/A								             *
 *************************************************************************************/

 void calculate_position(int i)
{
	pthread_mutex_lock (&mutexp);

	/*	The three lines below are fundamental role in updating the new value of theta	*/      
	pendulum[i].alpha = (-1 * g/pendulum[i].length) * sin(pendulum[i].theta ) ; 	/* 	Calculate acceleration 	*/
	pendulum[i].omega += pendulum[i].alpha;                                     	/* 	Increment angular velocity	*/
	pendulum[i].omega *= DUMP;                                                  	/*	Applying air friction	*/
	pendulum[i].theta += pendulum[i].omega;                                     	/* 	Increment theta	*/

	/* converting the polar coordinate to x,y cartesian coordinate*/
	pendulum[i].end_x = pendulum[i].origin_x + pendulum[i].length * sin(pendulum[i].theta); 
	pendulum[i].end_y = pendulum[i].origin_y + pendulum[i].length * cos(pendulum[i].theta);

	pthread_mutex_unlock (&mutexp);
}

/********************************************************************************************
 *NAME: display_info	                              		                            *
 *PURPOSE:To Display Basic information and Guideline to User                                *
 *RETURN: N/A								                    * 							*                                                                                           *
 ********************************************************************************************/

 void display_info()
{             
	pthread_mutex_lock (&mutexp);

	char    tbuf[32];
	char    sbuf[100];
	char    sbuf1[100];
	char    sbuf2[100];

	/* draw UL frame */
	rect(screen, UL_X, UL_Y, UL_X + UL_W, UL_Y + UL_H, MCOL);

	/* draw BL frame */
	rect(screen, BL_X, BL_Y, BL_X + BL_W, BL_Y + BL_H, MCOL);


	sprintf(tbuf, "%-12s: %d", "Number of Pendulums", MAX_PENDULUM);
	textout_ex(screen, font, tbuf, UL_X + 10, UL_Y + 10,
			NCOL, BKG);

	sprintf(tbuf, "%-12s: %-2.1f m/s²", "Gravity", g);
	textout_ex(screen, font, tbuf, UL_X + 10, UL_Y + 20,
			NCOL, BKG);

	sprintf(sbuf, "%-12s: %-2.1f ", "Length of Pendulum 1",  pendulum[0].length);
	textout_ex(screen, font, sbuf, UL_X + 200, UL_Y + 10,
			NCOL, BKG);
	sprintf(sbuf1, "%-12s: %-2.1f ", "Length of Pendulum 2", pendulum[1].length);
	textout_ex(screen, font, sbuf1, UL_X + 200, UL_Y + 20,
			NCOL, BKG);
	sprintf(sbuf2, "%-12s: %-2.1f ", "Length of Pendulum 3", pendulum[2].length);
	textout_ex(screen, font, sbuf2, UL_X + 410, UL_Y + 10,
			NCOL, BKG);


	textout_ex(screen, font, "Key 1: push to Right ", pendulum[0].origin_x-100, BL_Y + 5,
			NCOL+2, BKG);
	textout_ex(screen, font, "Key 2: push to Left  ", pendulum[0].origin_x-100, BL_Y + 15,
			NCOL+1, BKG);

	textout_centre_ex(screen, font, "Key 3: push to Right ", pendulum[1].origin_x-10, BL_Y + 5,
			NCOL+2, BKG);
	textout_centre_ex(screen, font, "Key 4: push to Left  ", pendulum[1].origin_x-10, BL_Y + 15,
			NCOL+1, BKG);

	textout_centre_ex(screen, font, "Key 5: push to Right ", pendulum[2].origin_x+25, BL_Y + 5,
			NCOL+2, BKG);
	textout_centre_ex(screen, font, "Key 6: push to Left  ", pendulum[2].origin_x+25, BL_Y + 15,
			NCOL+1, BKG);


	textout_centre_ex(screen, font, "Pendulum 1", pendulum[0].origin_x, pendulum[0].origin_y-15,
			NCOL, BKG);
	textout_centre_ex(screen, font, "Pendulum 2", pendulum[1].origin_x, pendulum[1].origin_y-15,
			NCOL, BKG);
	textout_centre_ex(screen, font, "Pendulum 3", pendulum[2].origin_x, pendulum[2].origin_y-15,
			NCOL, BKG);

	pthread_mutex_unlock (&mutexp);

}

/*************************************************************************************
*NAME: draw_pendulum	                              		  		     *
 *PURPOSE:To draw the pendulum in new position                                       *
 *RETURN: N/A								             *
 *************************************************************************************/

 void draw_pendulum(int i)
{
	pthread_mutex_lock (&mutexp);

	/* Redrawing the pendulum in the updated position*/
	line(screen, pendulum[i].origin_x, pendulum[i].origin_y, pendulum[i].end_x, pendulum[i].end_y, pendulum[i].c);
	circlefill(screen, pendulum[i].end_x, pendulum[i].end_y, pendulum[i].r, pendulum[i].c);

	pthread_mutex_unlock (&mutexp);
}

/*************************************************************************************
 *NAME: erase_pendulum	                              		  		     *
 *PURPOSE:To erase the perivious position of the drawn pendulum                      *
 *RETURN: N/A								             *
 *************************************************************************************/

 void erase_pendulum(int i)
{
	pthread_mutex_lock (&mutexp);

	/* erasing the previously drawn pendulum*/  
	line(screen, pendulum[i].origin_x, pendulum[i].origin_y, pendulum[i].end_x, pendulum[i].end_y, BKG);
	circlefill(screen, pendulum[i].end_x, pendulum[i].end_y, pendulum[i].r, BKG);

	pthread_mutex_unlock (&mutexp);
}


/*************************************************************************************
 *NAME: time_copy	                              		  		     *
 *PURPOSE: copies a source time variable ts in a destination variable pointed by td  *
 *INPUT: td - destination time variable and ts - source time variable		     *
 *RETURN: N/A								             *
 *************************************************************************************/

 void time_copy(struct timespec *td, struct timespec ts)
{
	td->tv_sec = ts.tv_sec;
	td->tv_nsec = ts.tv_nsec;
}

/***************************************************************************************
 *NAME:time_add_ms		                              		  	       *
 *PURPOSE: Adds a value ms expressed in milliseconds to the time variable pointed by t *
 *INPUT: t - destination time variable and ms - millisecond to be added		       *
 *RETURN: N/A								               *
 ***************************************************************************************/
void time_add_ms(struct timespec *t, int ms)
{
	t->tv_sec += ms/1000;
	t->tv_nsec += (ms%1000)*1000000;
	if (t->tv_nsec > 1000000000) {
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}

/**************************************************************************************
 *NAME:time_cmp	                              		  			      *
 *PURPOSE:compares two time variables t1 and t2  				      *
 *INPUT: t1 and t2 - first and second time variable 				      *
 *RETURN: 0 if they are equal, 1 if t1 > t2, ‐1 if t1 < t2  			      *
 **************************************************************************************/

 int time_cmp(struct timespec t1, struct timespec t2)
{
	if (t1.tv_sec > t2.tv_sec) return 1;
	if (t1.tv_sec < t2.tv_sec) return -1;
	if (t1.tv_nsec > t2.tv_nsec) return 1;
	if (t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
}

/**************************************************************************************
 *NAME:set_period                              		  			      *
 *PURPOSE:To computes the next activation time and the absolute deadline of the task. *
 *INPUT: pointer of structure task_par				                      *
 **************************************************************************************/

 void set_period(struct task_par *tp)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_copy(&(tp->at), t);
	time_copy(&(tp->dl), t);
	time_add_ms(&(tp->at), tp->period);
	time_add_ms(&(tp->dl), tp->deadline);
}

/**************************************************************************************
 *NAME:wait_for_period                              		  		      *
 *PURPOSE:To Suspends the calling thread until the next activation.                   *
 *INPUT: pointer of structure task_par				                      *
 **************************************************************************************/

void wait_for_period(struct task_par *tp)
{
	clock_nanosleep(CLOCK_MONOTONIC,
		 TIMER_ABSTIME, &(tp->at), NULL);
	time_add_ms(&(tp->at), tp->period);
	time_add_ms(&(tp->dl), tp->period);
}

/******************************************************************************
* FRAND: returns a random float in [xmi, xma)                                 *
/******************************************************************************/

float frand(float xmi, float xma)
{
	float r;
	r = rand()/(float)RAND_MAX; /* rand in [0,1) */
	return xmi + (xma - xmi)*r;
}





















