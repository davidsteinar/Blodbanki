//Disable visual studio warnings of unsafe FILE* oerations
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "BlodbankiSim.h"
#include "Distributions.h"

int N[MAXITEM][MAXBLOODGROUP]; /* Total number of batches of item “i” of blood group “g” */
float Stock[MAXNBATCH][MAXITEM][MAXBLOODGROUP]; /* Stock level of the nth batch of item “i” of blood group “g” */
float Texpiry[MAXNBATCH][MAXITEM][MAXBLOODGROUP]; /* The time of expiry of the nth batch of item “i” of blood group “g" */
float Tcamp; /* The time of next blood donation camp */
float perc_fail = 0.02;
float Tc; // Time between two succesive camps
float Sc;  // Donation at a camp (units)
float SBB; //Daily donation at the blood
float Dig;  //Daily demand for the item
float waste[MAXITEM][MAXBLOODGROUP];
float expireTimes[MAXITEM] = { 30.0, 365.0, 5.0, 30.0 }; //lifetime for { wholeblood, plasma, plateletes, RBC }
float Perc[MAXBLOODGROUP] = { 0.2119, 0.2924, 0.3709, 0.0644 }; /* A+ B+ O+ AB+ and 6.04% are of negative type */
FILE  *infile, *outfile;

void simulate()
{
	printf("Starting blood bank inventory management simulation.\n");
	if ((infile = fopen("bloodbank.in", "r")) == NULL) {
		printf("Can't find the file: bloodbank.in\n");
		exit(1);
	}
	if ((outfile = fopen("bloodbank.out", "w")) == NULL) {
		printf("Can't create the file: bloodbank.out\n");
		exit(1);
	}
	readInput();

	init_simlib();

	// Schedule first event

	event_schedule(sim_time + 0, EVENT_BLOOD_ARRIVAL);
	event_schedule(sim_time + 1, EVENT_BLOOD_DEMAND);



	event_schedule(40, EVENT_END_SIMULATION);

	eventLoop();
	report();

	fclose(infile);
	fclose(outfile);
}

void readInput()
{
	int i = 0;
	// Assign input from infile to blodbanki variables
	/* for (g = 0; g < MAXBLOODGROUP; g++)
		fscanf(infile, "%f ", &Perc[g]); */


	//Print input information to output file
	fprintf(outfile, "Percentage composition of blood group:\n");

	for (i = 0; i < MAXBLOODGROUP; i++)
		fprintf(outfile, "Perc[%d] = %16.3f \n", i, Perc[i]);
}

void eventLoop()
{
	while (list_size[LIST_EVENT] != 0) {

		/* Determine the next event. */
		timing();

		switch (next_event_type) {

		case EVENT_BLOOD_ARRIVAL:
			bloodArrival();
			break;
		case EVENT_BLOOD_DONATION:
			bloodDonation();
			break;
		case EVENT_BLOOD_DEMAND:
			bloodDemand();
			break;
		case EVENT_BLOOD_EXPIRATION:
			bloodExpiration();
			break;
		case EVENT_END_SIMULATION:
			return;
			break;
		}
	}
}

//Blood arrival from camp
void bloodArrival()
{
	/* the following is the cumulative empirical distribution for days beween camps, data taken from Excel data file (duration between camps) */
	float Fcamp[17] = { 0.09363296, 0.48689139, 0.65543071, 0.77528090, 0.86516854, 0.91011236, 0.95505618, 0.97003745, 0.98127341,
		0.98127341, 0.98876404, 0.99250936, 0.99625468, 0.99625468, 0.99625468, 0.99625468, 1.0 };
	/* the description in the book chapter uses an Empirical distribution for units of blood donated, then you would do the same as for Fcamp,
	However, see bloodbank.R script for analysis of this data, we find that we could us the negative binomial distribution */
	float size = 2.7425933;
	float mu = 83.5663430;  /* these are the parameters found in the R script for the negative binomial distribution */
	int collected = negativebinomrnd(size, mu, STREAM_BLOOD_ARRIVAL);
	int i;
    /*
	for (i = 0; i < 300; i++)
		printf("%d\n,", negativebinomrnd(size, mu, STREAM_BLOOD_ARRIVAL));
    */
    float collectedTypes[MAXBLOODGROUP];
    collected *= 1-perc_fail;
    for(i = 0; i < MAXBLOODGROUP; i++)
    {
        collectedTypes[i] = collected * Perc[i];
    }
    int wholeBloodType = 0;

    int g;
    for(g = 0; g < MAXBLOODGROUP; g++)
    {
        int n = N[wholeBloodType][g];
        Stock[n][wholeBloodType][g]= collectedTypes[g];
    }
    //Increment batch size for all items i and blood group g
    for(g = 0; g < MAXBLOODGROUP; g++)
    {
        N[wholeBloodType][g]++;
    }
    //Update stock levels

    /*
    if(sim_time >= 50)
    {
        float totalUnits[MAXITEM][MAXBLOODGROUP];
        for(i = 0; i < MAXBLOODGROUP; i++)
        {

            int n = N[wholeBloodType][i];
            int j;
            for(j = 0; j < n; j++)
            {
                totalUnits[wholeBloodType][i] += Stock[j][wholeBloodType][i];
            }
            printf("Total units of wholeblood of blood group %d = %f\n", i, totalUnits[wholeBloodType][i]);
        }
        exit(1);
    } */

	/*
	1) Update the stock levels of the items of various
	blood groups based on quantity collected ( based on
	collection policy), % failed tests, % componentized
	and % of various blood groups.
    */

    for(g = 0; g < MAXBLOODGROUP; g++)
    {
        transfer[ITEM] = wholeBloodType;
        transfer[BLOODGROUP] = g;

        //schedule expiry events for all items i of blood group g
        event_schedule(sim_time + expireTimes[wholeBloodType], EVENT_BLOOD_EXPIRATION);
    }

    /*
	2) Compute the day of expiry of the new batch of
	the various items.
	3) Generate the day of the next CAMP.
	*/


	/* Schedule next camp 3) */

	event_schedule(sim_time + (float)discrete_empirical(Fcamp, 17, STREAM_BLOOD_ARRIVAL), EVENT_BLOOD_ARRIVAL);
}

void bloodDonation()
{
	/*
	1) Update the stock levels of the items of various blood groups based
	on daily donation quantity,% failed tests ,% componentized and % of
	various blood groups.
	2) Compute the day of Expiry of the new batch of the various items
	*/
}

void bloodExpiration()
{
    /*
	1) Exhaust the batches of various items scheduled to be expired
	on the day.
	2) Update wastages if any.
	*/
    int bloodGroup = transfer[BLOODGROUP];
    int item = transfer[ITEM];

    int n;
    //O(N) operation :(
    //Move all badges in Stock 1 backwards
    printf("Threw away %f units of whole blood of blood group %d\n", Stock[0][item][bloodGroup], (int) bloodGroup);//debug print
    waste[item][bloodGroup] += Stock[0][item][bloodGroup];
    for (n = 1; n < N[item][bloodGroup]-2; n++)
    {
        Stock[n-1][item][bloodGroup] = Stock[n][item][bloodGroup];

    }
    N[item][bloodGroup] --; // Decrement total number of badges for item i and bloodgroup g

}

void bloodDemand()
{
	/*
	1) Generate the daily demand for the items of various blood groups.
	2) Update the stock levels of the items of various blood groups.
	3) If the unmet demand for an item is fulfilled using the stock of a
	substitutable item, update the stock of the latter.
	4) Update shortages if any.
	*/
}

void report()
{
    printf("\n");
    int wholeBloodType = 0;
    int i;
    for(i = 0; i < MAXBLOODGROUP; i++)
    {

        printf("Total wastage of wholeblood of group %d = %d\n", i, waste[wholeBloodType][i]);
    }
	//report information to outfile
	//fprintf(outfile, "..."),
	printf("End of blood bank simulation.\n");
	printf("Results have been written into \"bloodbank.out\".\n");

}
