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
float perc_fail;
float componentizePolicy; // % of componentized whole blood
float collectionPolicy; // % of how much of the blood is collected - rest goes to HBB??
float max_sim_time;
float Tc; // Time between two succesive camps
float Sc;  // Donation at a camp (units)
float SBB; //Daily donation at the blood
float Dig;  //Daily demand for the item
float waste[MAXITEM][MAXBLOODGROUP];
float shortage[MAXITEM][MAXBLOODGROUP];
float expireTimes[MAXITEM];//lifetime for { wholeblood, plasma, plateletes, RBC }
float Perc[MAXBLOODGROUP]; /* A+ B+ O+ AB+ and 6.04% are of negative type */
char *bloodGroupTypes[MAXBLOODGROUP] = { "A+", "B+", "O+", "AB+" };
char *bloodItemTypes[MAXITEM] = { "WB", "RBC", "platelets", "plasma" };
FILE  *infile, *outfile;

void simulate()
{
	printf("Blood bank inventory management simulation.\n");
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

	event_schedule(sim_time + 0, EVENT_BLOOD_ARRIVAL); //first arrival from camp at beginning
	event_schedule(sim_time + 0, EVENT_BLOOD_DONATION); //first donation at beginning
	event_schedule(sim_time + 1, EVENT_BLOOD_DEMAND); //first demand next day


	event_schedule(max_sim_time, EVENT_END_SIMULATION);

	eventLoop();
	report();

	fclose(infile);
	fclose(outfile);
}

void readInput()
{
	int g = 0;
	// Assign input from infile to blodbanki variables
	fscanf(infile, "%f", &max_sim_time);
	fscanf(infile, "%f", &perc_fail);
	fscanf(infile, "%f", &componentizePolicy);
	fscanf(infile, "%f", &collectionPolicy);
	for (g = 0; g < MAXBLOODGROUP; g++)
		fscanf(infile, "%f ", &Perc[g]);
	for (g = 0; g < MAXITEM; g++)
		fscanf(infile, "%f ", &expireTimes[g]);
	

	fprintf(outfile, "Input data for the simulation.\n\n");

	//Print input information to output file
	fprintf(outfile, "Maximum simulation time: %d days.\n", (int)max_sim_time);
	fprintf(outfile, "Percentage of failed blood collections %d%%.\n", (int)(100*perc_fail));
	fprintf(outfile, "Componentization policy: %d%%\n", (int)(100*componentizePolicy));
	fprintf(outfile, "Collection policy: %d%%\n", (int)(100 * collectionPolicy));
	fprintf(outfile, "Percentage composition of blood group\n");
	for (g = 0; g < MAXBLOODGROUP; g++)
		fprintf(outfile, "Perc[%s] = %.3f \n", bloodGroupTypes[g], Perc[g]);
	fprintf(outfile, "\n");
	fprintf(outfile, "Lifetime for items\n");
	for (g = 0; g < MAXBLOODGROUP; g++)
		fprintf(outfile, "expiry[%s] = %.1f \n", bloodItemTypes[g], expireTimes[g]);
	fprintf(outfile, "\n");



}

void eventLoop()
{
	while (list_size[LIST_EVENT] != 0) {

		/* Determine the next event. */
		timing();

		switch (next_event_type) {

		case EVENT_BLOOD_ARRIVAL:
			bloodCamp();
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
void bloodArrival(int totalCollected)
{
	/* the following is the cumulative empirical distribution for days beween camps, data taken from Excel data file (duration between camps) */
	int g, i;
    float collected[MAXITEM][MAXBLOODGROUP];
	totalCollected *= collectionPolicy; //collect based on collection policy?
    totalCollected *= 1-perc_fail; //throw away the failed ones

	//Distribute to blood group types
	float portion = totalCollected * (1 - componentizePolicy); //whole blood portion
	for (i = 0; i < MAXITEM; i++)
	{
		if (i == 1) portion = totalCollected * componentizePolicy; // the componentized portion = (1-whole blood portion)
		for (g = 0; g < MAXBLOODGROUP; g++)
		{
			collected[i][g] = totalCollected  * portion * Perc[g];
		}
	}
	
	//Update stock
	for (i = 0; i < MAXITEM; ++i)
	{
		for (g = 0; g < MAXBLOODGROUP; g++)
		{
			int n = N[i][g];
			Stock[n][i][g] = collected[i][g];
		}
	}
		
    //Increment batch size for all items i and blood group g
	for (i = 0; i < MAXITEM; ++i)
	{
		for (g = 0; g < MAXBLOODGROUP; g++)
		{
			N[i][g]++;
		}
	}
	for (i = 0; i < MAXITEM; ++i)
	{
		for (g = 0; g < MAXBLOODGROUP; g++)
		{
			transfer[ITEM] = i;
			transfer[BLOODGROUP] = g;

			//schedule expiry events for all items i of blood group g
			event_schedule(sim_time + expireTimes[i], EVENT_BLOOD_EXPIRATION);
		}
	}
}

void bloodCamp()
{
	float Fcamp[17] = { 0.09363296, 0.48689139, 0.65543071, 0.77528090, 0.86516854, 0.91011236, 0.95505618, 0.97003745, 0.98127341,
		0.98127341, 0.98876404, 0.99250936, 0.99625468, 0.99625468, 0.99625468, 0.99625468, 1.0 };
	/* the description in the book chapter uses an Empirical distribution for units of blood donated, then you would do the same as for Fcamp,
	However, see bloodbank.R script for analysis of this data, we find that we could us the negative binomial distribution */
	float size = 2.7425933;
	float mu = 83.5663430;  /* these are the parameters found in the R script for the negative binomial distribution */
	float collected = (float) negativebinomrnd(size, mu, STREAM_BLOOD_ARRIVAL);
	DEBUGPRINTF("Blood from camp: %.4f units. Now storing in inventory based on policies. \n", collected);
	bloodArrival(collected);
	/* Schedule next camp 3) */
	event_schedule(sim_time + (float)discrete_empirical(Fcamp, 17, STREAM_BLOOD_ARRIVAL), EVENT_BLOOD_ARRIVAL);
}

void bloodDonation()
{
	//placeholders
	float collected = 21.4; // TODO: need generator function for blood collected here
	float nextDonation = 3; //TODO: need generator function for time until next donation event
	/*
	1) Update the stock levels of the items of various blood groups based
	on daily donation quantity,% failed tests ,% componentized and % of
	various blood groups.
	2) Compute the day of Expiry of the new batch of the various items
	*/
	DEBUGPRINTF("Blood from donation: %.4f units. Now storing in inventory based on policies. \n", collected);
	bloodArrival(collected);
	/* Schedule next donation event */
	event_schedule(sim_time + nextDonation, EVENT_BLOOD_DONATION);
	
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
    //Move all badges in Stocks backwards by 1
	DEBUGPRINTF("Threw away expired %.4f units of %s of blood group %s\n", Stock[0][item][bloodGroup], bloodItemTypes[item], bloodGroupTypes[bloodGroup]);
    waste[item][bloodGroup] += Stock[0][item][bloodGroup];
    for (n = 1; n < N[item][bloodGroup]-2; n++)
    {
        Stock[n-1][item][bloodGroup] = Stock[n][item][bloodGroup];

    }
    N[item][bloodGroup] --; // Decrement total number of badges for item i and bloodgroup g
}

void bloodDemand()
{
	int i, g, n;
	/*
	1) Generate the daily demand for the items of various blood groups.
	//TODO: implement generators for daily demand of items i of blood groups g.
	*/
	float bloodDemand[MAXITEM][MAXBLOODGROUP];
	float demandSum = 0.0f; //not important - just for printing
	for (i = 0; i < MAXITEM; i++)
	{
		for (g = 0; g < MAXBLOODGROUP; g++)
		{
			//Placeholder 
			bloodDemand[i][g] = 18.3f; //generator function for [i][g] comes here instead of this line
			demandSum += bloodDemand[i][g];
		}
	}
	/*
	2) Update the stock levels of the items of various blood groups.
	3) If the unmet demand for an item is fulfilled using the stock of a
	substitutable item, update the stock of the latter.
	*/
	for (i = 0; i < MAXITEM; i++)
	{
		for (g = 0; g < MAXBLOODGROUP; g++)
		{
			int n = 0;
			//search through badges n=0,1,2,...,MAXNBATCH.
			//if a batch is empty, try next batch
			//if a batch has blood, use blood. If demand is fulfilled stop. Else try next badge while there is demand.
			while (bloodDemand[i][g] > 0.0f && n < N[i][g])
			{
				if (Stock[n][i][g] == 0.0f) ++n; //badge n is empty - check next badge
				else if (Stock[n][i][g] > bloodDemand[i][g]) //enough of blood in badge n to fulfill demand
				{
					Stock[n][i][g] -= bloodDemand[i][g];
					bloodDemand[i][g] = 0.0f;
					break;
				}
				else if(Stock[n][i][g] <= bloodDemand[i][g]) //blood in badge n, but not enough to fulfill demand - use up all blood in stock and check next badge n+1
				{
					bloodDemand[i][g] -= Stock[n][i][g];
					Stock[n][i][g] = 0.0f;
					++n;
				}
			}
		}
	}
	float shortageSum = 0;
	/*
	4) Update shortages if any.
	*/
	//Now bloodDemand[i][g] has been modified to be the unfulfilled demands, a.k.a the shortage
	for (i = 0; i < MAXITEM; i++)
	{
		for (g = 0; g < MAXBLOODGROUP; g++)
		{
			shortage[i][g] += bloodDemand[i][g];
			shortageSum += bloodDemand[i][g];
		}
	}
	
	DEBUGPRINTF("Daily demand: total %.2f units. Failed to fulfill %d%% of demands\n", demandSum, (int)(100*shortageSum/demandSum));
	//Schedule demand on the next day
	event_schedule(sim_time + 1.0f, EVENT_BLOOD_DEMAND); //daily
}

void report()
{
	fprintf(outfile, "\n");
	fprintf(outfile, "End of blood bank simulation.\n");
	int wholeBloodType = 0;
	int i, g;
	float totalUnits[MAXITEM][MAXBLOODGROUP];
	memset(totalUnits, 0.0f, sizeof(totalUnits)); //initialize with 0.0f
	fprintf(outfile, "\n");
	fprintf(outfile, "State of inventory at the end of simulation. \n");
	for(i = 0; i < MAXITEM; i++)
	{
		for (g = 0; g < MAXBLOODGROUP; g++)
		{
			int n = N[i][g];
			int j;
			for (j = 0; j < n - 1; j++)
			{
				totalUnits[i][g] += Stock[j][i][g];
			}
			fprintf(outfile, "Total %.4f units of %s of blood group %s.\n", totalUnits[i][g], bloodItemTypes[i], bloodGroupTypes[g]);
		}
	} 
    fprintf(outfile, "\n");
	fprintf(outfile, "Total wasted blood across the simulation\n");
	for (i = 0; i < MAXITEM; i++)
	{
		for (g = 0; g < MAXBLOODGROUP; g++)
		{
			fprintf(outfile, "Wasted %.4f units of %s of group %s.\n", waste[i][g], bloodItemTypes[i], bloodGroupTypes[g]);
		}
	}

	fprintf(outfile, "\n");
	fprintf(outfile, "Total shortage of blood across the simulation\n");
	for (i = 0; i < MAXITEM; i++)
	{
		for (g = 0; g < MAXBLOODGROUP; g++)
		{
			fprintf(outfile, "Shortage of %.4f units of %s of group %s.\n", shortage[i][g], bloodItemTypes[i], bloodGroupTypes[g]);
		}
	}
	//report information to outfile
	//fprintf(outfile, "..."),
	printf("Results have been written into \"bloodbank.out\".\n");
}
