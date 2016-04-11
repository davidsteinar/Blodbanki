
#include "simlib.h"

//Define constants

#define ENABLE_DEBUGPRINTF

#define EVENT_BLOOD_ARRIVAL     1  /* Event type for arrival of blood */
#define EVENT_BLOOD_DONATION	2  /* Event type for blood departure (when blood is used) */
#define EVENT_BLOOD_DEMAND		3  /* Event type for blood demand */
#define EVENT_BLOOD_EXPIRATION		4	/* Event type for blood expiration */
#define EVENT_END_SIMULATION    5  /* Event type simulation end */

#define STREAM_BLOOD_ARRIVAL   1  /* Random-number stream for interarrivals of blood. */
#define STREAM_BLOOD_DEMAND	2  /*  Random-number stream for demands of blood. */

#define MAXNBATCH 1000
#define MAXITEM  4
#define MAXBLOODGROUP 4 /* only the postive blood groups are considered for the simlation since negative ones are rare */

#define ITEM    3  //til að setja í transfer fylki fyrir Expire event
#define BLOODGROUP  4

#ifdef ENABLE_DEBUGPRINTF
#define DEBUGPRINTF(msg, ...)						\
	do {											\
		printf("At sim_time = %.1f: ", sim_time);	\
		printf(msg, __VA_ARGS__);					\
	}												\
	while (0)
#else
#define DEBUGPRINTF(msg) 
#endif

extern int N[MAXITEM][MAXBLOODGROUP]; /* Total number of batches of item “i” of blood group “g” */
extern float Stock[MAXNBATCH][MAXITEM][MAXBLOODGROUP]; /* Stock level of the nth batch of item “i” of blood group “g” */
extern float Texpiry[MAXNBATCH][MAXITEM][MAXBLOODGROUP]; /* The time of expiry of the nth batch of item “i” of blood group “g" */
extern float waste[MAXITEM][MAXBLOODGROUP];
extern float shortage[MAXITEM][MAXBLOODGROUP];
extern float Tcamp; /* The time of next blood donation camp */
extern float collectionPolicy, componentizePolicy;
extern float max_sim_time;
extern float perc_fail, Tc, Sc, SBB, Dig;
extern float Perc[MAXBLOODGROUP]; /* A+ B+ O+ AB+ and 6.04% are of negative type */
extern char *bloodGroupTypes[MAXBLOODGROUP]; // index to blood types char*
extern char *bloodItemTypes[MAXITEM];
extern FILE  *infile, *outfile;

/* Bloodbank simulation related functions declared */

void simulate(void);

void readInput(void);
void eventLoop(void);

void bloodArrival(int collected);
void bloodCamp(void);
void bloodDonation(void);
void bloodExpiration(void);
void bloodDemand(void);

void report(void);

