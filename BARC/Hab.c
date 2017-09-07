
#include "RPI.h"
#include "rc_timer.h"
#include "Hab.h"

// fm is pointer of filename
// hab_duration is duration is testing time.
void standalone_hab(char* fm, int hab_duration) {

    // Set up gpi pointer for direct register access
    setup_io();

    /************************************************************************\
    * You are about to change the GPIO settings of your computer.          *
    * Mess this up and it will stop working!                               *
    * It might be a good idea to 'sync' before running this program        *
    * so at least you still have your code changes written to the SD-card! *
    \************************************************************************/
    INP_GPIO(SENSOR_PIN); // Initiating SENSOR_PIN

    int pressing_counter = 0;
    int response_once = 0;

    dnum upnum;
    upnum.up = 20;
    upnum.pre_detection_count = 0;

    // Random init
    // Run rand() several times to make it random
    // time_t wt;
    // wt = time(NULL);
    // srand((unsigned) time(&wt));
    // rand();rand();rand();

    // Init timer
    // tot is an updating timer
    // clock_gettime(CLOCK_REALTIME, &upnum.start_of_time);
    // upnum.eot = upnum.start_of_time;
    clock_gettime(CLOCK_REALTIME, &upnum.tot);

    // Session start
    // no space between comma in fprintf
    FILE *fp;
    fp=fopen(fm, "a");
    clock_gettime(CLOCK_REALTIME, &upnum.start_of_time);
    record_data(fp, OPENFILE, &upnum.start_of_time);

    // Start session
    record_data(fp, STARTSESSION, &upnum.start_of_time);
    fflush(fp);

    // Start ITI
    upnum.eot = upnum.start_of_time;
    record_data(fp, STARTITI, &upnum.eot);
    upnum.eot.tv_sec += hab_duration;
    // start detecting
    while((upnum.tot.tv_sec < upnum.eot.tv_sec) | (upnum.tot.tv_nsec < upnum.eot.tv_nsec)) {
        if(response_once = sensor_detection_pre(SENSOR_PIN, &upnum.tot, &upnum.eot, &upnum.up, &upnum.pre_detection_count)) {
            pressing_counter++;
            record_data(fp, PRESSING, &upnum.tot);
            printf("%d, Pressing during hab.\n", pressing_counter);
            fflush(fp);
        }
    } // Stop detecting

    // End of trial
    record_data(fp, ENDITI, &upnum.eot);
    // End of session
    record_data(fp, ENDSESSION, &upnum.eot);
    // Closing file
    fclose(fp);

    // RESET GPIO PIN
    INP_GPIO(SENSOR_PIN); // Initiating SENSOR_PIN

} // end of standalone_hab

void repeated_hab(char* fm, int hab_duration) {

    int pressing_counter = 0;
    int response_once = 0;

	dnum upnum;
    upnum.up = 20;
    upnum.pre_detection_count = 0;

    // Set up gpi pointer for direct register access
    setup_io();

    /************************************************************************\
    * You are about to change the GPIO settings of your computer.          *
    * Mess this up and it will stop working!                               *
    * It might be a good idea to 'sync' before running this program        *
    * so at least you still have your code changes written to the SD-card! *
    \************************************************************************/
    INP_GPIO(SENSOR_PIN);

    // Random init
    // Run rand() several times to make it random
    // time_t wt;
    // wt = time(NULL);
    // srand((unsigned) time(&wt));
    // rand();rand();rand();

    // timer init
    // tot is updating timer
    // clock_gettime(CLOCK_REALTIME, &upnum.start_of_time);
    // upnum.eot = upnum.start_of_time;
    clock_gettime(CLOCK_REALTIME, &upnum.tot);

    // Session start
    // no space between comma in fprintf
    FILE *fp;
    fp=fopen(fm, "a");
    clock_gettime(CLOCK_REALTIME, &upnum.start_of_time);
    record_data(fp, OPENFILE, &upnum.start_of_time);

    record_data(fp, STARTSESSION, &upnum.start_of_time);
    fflush(fp);

    upnum.eot = upnum.start_of_time;
    record_data(fp, STARTITI, &upnum.eot);
    upnum.eot.tv_sec += 1;
    upnum.start_of_time.tv_sec += 10;
    // start detecting
    int i = 0;
    for (i = 0; i < 10; i++) {
		while((upnum.tot.tv_sec < upnum.start_of_time.tv_sec) | (upnum.tot.tv_nsec < upnum.eot.tv_nsec)) {
			if(response_once = sensor_detection_pre(SENSOR_PIN, &upnum.tot, &upnum.eot, &upnum.up, &upnum.pre_detection_count)) {
				pressing_counter++;
				record_data(fp, PRESSING, &upnum.tot);
				printf("%d, Pressing during hab.\n", pressing_counter);
				fflush(fp);
			}
			else if (i < 10) {
				upnum.eot.tv_sec += 1;
			}
		}
    } // Stop detecting

    // End of trial
    record_data(fp, ENDITI, &upnum.eot);
    // End of session
    record_data(fp, ENDSESSION, &upnum.eot);
    // Closing file
    fclose(fp);

    // RESET GPIO PIN
    INP_GPIO(SENSOR_PIN); // Initiating SENSOR_PIN

} // end of repeated_hab
