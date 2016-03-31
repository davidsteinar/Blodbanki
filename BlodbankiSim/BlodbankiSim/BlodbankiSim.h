
#include "simlib.h"            

//Define constants

#define EVENT_BLOOD_ARRIVAL     1  /* Event type for arrival of blood */
#define EVENT_BLOOD_DONATION	2  /* Event type for blood departure (when blood is used) */
#define EVENT_BLOOD_DEMAND		3  /* Event type for blood demand */
#define EVENT_BLOOD_EXPIRATION		4	/* Event type for blood expiration */
#define EVENT_END_SIMULATION    5  /* Event type simulation end */

#define LIST_BLOOD_STORAGE		1
#define LIST_BLOOD_DEMAND		2  

#define STREAM_BLOOD_ARRIVAL   1  /* Random-number stream for interarrivals of blood. */
#define STREAM_BLOOD_DEMAND	2  /*  Random-number stream for demands of blood. */

#define MAXNBATCH 1000
#define MAXITEM  1000
#define MAXBLOODGROUP 4 /* only the postive blood groups are considered for the simlation since negative ones are rare */
int N[MAXITEM][MAXBLOODGROUP]; /* Total number of batches of item “i” of blood group “g” */
float Stock[MAXNBATCH][MAXITEM][MAXBLOODGROUP]; /* Stock level of the nth batch of item “i” of blood group “g” */
float Texpiry[MAXNBATCH][MAXITEM][MAXBLOODGROUP]; /* The time of expiry of the nth batch of item “i” of blood group “g" */
float Tcamp; /* The time of next blood donation camp */
float perc_fail, Tc, Sc, SBB, Dig;
static float Perc[MAXBLOODGROUP] = { 21.19, 29.24, 37.09, 6.44 }; /* A+ B+ O+ AB+ and 6.04% are of negative type */
FILE  *infile, *outfile;

/* Bloodbank simulation related functions declared */

void simulate(void);

void readInput(void);
void eventLoop(void);

void bloodArrival(void);
void bloodDonation(void);
void bloodExpiration(void);
void bloodDemand(void);


void report(void);