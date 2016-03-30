//Disable visual studio warnings of unsafe FILE* oerations
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "BlodbankiSim.h"


void simulate()
{
	printf("Starting blood bank simulation.\n");
	infile = fopen("bloodbank.in", "r");
	outfile = fopen("bloodbank.out", "w");

	readInput();
	
	init_simlib();

	// Schedule first event

	//event_schedule(...);

	// Schedule simulation end 

	//event_schedule(..., EVENT_END_SIMULATION);

	eventLoop();
	report();

	fclose(infile);
	fclose(outfile);
}

void readInput()
{
	// Assign input from infile to blodbanki variables
	/*  
		fscanf(infile, ...);
	*/
	//Print input information to output file
	/* 
	fprintf(outfile,"....");
	*/
	
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
		case EVENT_BLOOD_USED:
			bloodUsed();
			break;
			/* ... all event types*/
		case EVENT_END_SIMULATION:
			return;
			break;
		}
	}
}

void bloodArrival()
{

}

void bloodUsed()
{

}

void report()
{
	//report information to outfile
	//fprintf(outfile, "..."),
	printf("End of blood bank simulation.\n");
	printf("Results have been written into \"bloodbank.out\".\n");

}