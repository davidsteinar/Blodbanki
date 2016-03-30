
#include "simlib.h"            

//Define constants

#define EVENT_BLOOD_ARRIVAL        1  /* Event type for arrival of blood */
#define EVENT_BLOOD_USED	2		/* Event type for blood consumption (when blood is used) */
#define EVENT_END_SIMULATION    3  /* Event type simulation end */

#define STREAM__BLOOD_INTERARRIVAL  1  /* Random-number stream for interarrivals of blood. */

/* Non-simlib */

FILE  *infile, *outfile;

/* Bloodbank simulation related functions declared */

void simulate(void);
void readInput(void);
void eventLoop(void);
void arrive(void);
void bloodArrival(void);
void bloodUsed(void);
void report(void);