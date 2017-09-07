
#include "RPI.h"
#include "rc_timer.h"
#include "AA.h"

// fm is is pointer to file name
// ignore_hab = 1 is ignoring 3 minutes habituation time.
// short_duration = 1 is fast_testing
// trail_number
void aa (char* fm, int ignore_hab, int short_duration, int trial_number) {
    if (!short_duration) {
        sleep(10);
    }

    // Set up gpi pointer for direct register access
    setup_io();

    /************************************************************************\
    * You are about to change the GPIO settings of your computer.          *
    * Mess this up and it will stop working!                               *
    * It might be a good idea to 'sync' before running this program        *
    * so at least you still have your code changes written to the SD-card! *
    \************************************************************************/
    INP_GPIO(SENSOR_PIN); // Initiating SENSOR_PIN
    INP_GPIO(CUE_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(CUE_PIN);
    GPIO_CLR = 1 << CUE_PIN;
    INP_GPIO(HOUSE_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(HOUSE_PIN);
    GPIO_CLR = 1 << HOUSE_PIN; // turn HOUSE_PIN off
    INP_GPIO(SHOCK_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(SHOCK_PIN);
    GPIO_CLR = 1 << SHOCK_PIN; // turn SHOCK_PIN off
    INP_GPIO(RELAY_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(RELAY_PIN);
    GPIO_CLR = 1 << RELAY_PIN; // turn SHOCK_PIN off

    // Declaring main struct
    dnum upnum;
    // init variables
    upnum.pre_detection_count = 0;
    upnum.up = 20;

    // Random init
    // Run rand() several times to make it random
    time_t wt;
    wt = time(NULL);
    srand((unsigned) time(&wt));
    rand();rand();rand();

    int i, j; // iter
    int flash_off = 0;
    int response_once = 0;

    int iti[trial_number], *ptoiti;
    ptoiti = iti;

    //ITI pseudorandom: random sampling without replacement
    if (short_duration) {
        int ITI[5] = {1, 2, 3, 4, 5};
        int len_of_ITI = sizeof(ITI)/sizeof(int);
        combination(ptoiti, ITI, len_of_ITI, trial_number);
        print_array(ptoiti, trial_number);
    }
    else {
        int ITI[5] = {15, 18, 21, 24, 27};
        int len_of_ITI = sizeof(ITI)/sizeof(int);
        combination(ptoiti, ITI, len_of_ITI, trial_number);
        print_array(ptoiti, trial_number);
    }

    // init timer
    clock_gettime(CLOCK_REALTIME, &upnum.start_of_time);
    clock_gettime(CLOCK_REALTIME, &upnum.eot);
    clock_gettime(CLOCK_REALTIME, &upnum.tot);
    clock_gettime(CLOCK_REALTIME, &upnum.flash_of_time);

    int hab_duration;
    if (ignore_hab) {
        hab_duration = 10;
    }
    else {
        hab_duration = 180;
    }
    int CS_duration;
    int US_duration;
    int trial_duration;
    if (short_duration) {
        CS_duration = 5;
        US_duration = 2;
        trial_duration = CS_duration + US_duration;
    }
    else {
        CS_duration = 10;
        US_duration = 5;
        trial_duration = CS_duration + US_duration;
    }

    FILE *fp;
    fp=fopen(fm, "a");
    clock_gettime(CLOCK_REALTIME, &upnum.start_of_time);
    // open file
    record_data(fp, OPENFILE, &upnum.start_of_time);
    // start session
    record_data(fp, STARTSESSION, &upnum.start_of_time);
    fflush(fp);

    // EVERYTHING START HERE
    upnum.eot.tv_sec = upnum.start_of_time.tv_sec;
    upnum.eot.tv_nsec = upnum.start_of_time.tv_nsec;
    record_data(fp, STARTITI, &upnum.eot);

    // hab start
    upnum.eot.tv_sec += hab_duration;
    while((upnum.tot.tv_sec < upnum.eot.tv_sec) | (upnum.tot.tv_nsec < upnum.eot.tv_nsec)) {
        if(response_once = sensor_detection_pre(SENSOR_PIN, &upnum.tot, &upnum.eot, &upnum.up, &upnum.pre_detection_count)) {
            record_data(fp, PRESSING, &upnum.tot);
        }
    }
    record_data(fp, ENDITI, &upnum.eot);
    fflush(fp);
    // Hab end

    GPIO_SET = 1 << HOUSE_PIN; // turn HOUSE_PIN on
    for (i = 0; i < trial_number; i++) {
        record_data(fp, STARTITI, &upnum.eot);
        // ITI start
        upnum.eot.tv_sec += iti[i];
        while((upnum.tot.tv_sec < upnum.eot.tv_sec) | (upnum.tot.tv_nsec < upnum.eot.tv_nsec)) {
            if(response_once = sensor_detection_pre(SENSOR_PIN, &upnum.tot, &upnum.eot, &upnum.up, &upnum.pre_detection_count)) {
                record_data(fp, PRESSING, &upnum.tot);
            }
        }
        record_data(fp, ENDITI, &upnum.eot);
        // ITI end

		printf("Trial NO.: %d\n", i + 1); // Trial start
        printf("CS on.\n"); // CS on
        GPIO_SET = 1 << CUE_PIN; // turn CUE_PIN on
        upnum.eot.tv_sec += CS_duration;
        if(response_once = sensor_detection_pre(SENSOR_PIN, &upnum.tot, &upnum.eot, &upnum.up, &upnum.pre_detection_count)) {
            GPIO_CLR = 1 << CUE_PIN;
            GPIO_CLR = 1 << HOUSE_PIN; // flash: turn HOUSE_PIN off
            record_data(fp, AVOIDANCE, &upnum.tot);

            // Remaining time of Avoidance
            // flash the house light
            upnum.flash_of_time.tv_sec = upnum.tot.tv_sec + 1;
            upnum.flash_of_time.tv_nsec = upnum.tot.tv_nsec;
            flash_off = 1;

            upnum.eot.tv_sec += US_duration;
            while((upnum.tot.tv_sec < upnum.eot.tv_sec) | (upnum.tot.tv_nsec < upnum.eot.tv_nsec)) {
                if(sensor_detection_pre(SENSOR_PIN, &upnum.tot, &upnum.flash_of_time, &upnum.up, &upnum.pre_detection_count)) {
                    record_data(fp, PRESSING, &upnum.tot);
                }
                else if (flash_off) {
                    GPIO_SET = 1 << HOUSE_PIN; // turn HOUSE_PIN on
                    upnum.flash_of_time = upnum.eot; // prolong it
                    flash_off = 0; // do it once
                }

            } // end of remaining time

        }
        else { // US on
            GPIO_SET = 1 << SHOCK_PIN; // turn SHOCK_PIN on
            GPIO_SET = 1 << RELAY_PIN; // turn RELAY_PIN on
            printf("US on.\n");
            upnum.eot.tv_sec += US_duration;
            if(response_once = sensor_detection_pre(SENSOR_PIN, &upnum.tot, &upnum.eot, &upnum.up, &upnum.pre_detection_count)) {
                GPIO_CLR = 1 << SHOCK_PIN;
                GPIO_CLR = 1 << RELAY_PIN; // turn RELAY_PIN off
                GPIO_CLR = 1 << CUE_PIN;
                record_data(fp, ESCAPE, &upnum.tot);

                // Remaining time of A & Escape
				while((upnum.tot.tv_sec < upnum.eot.tv_sec) | (upnum.tot.tv_nsec < upnum.eot.tv_nsec)) {
					if(sensor_detection_pre(SENSOR_PIN, &upnum.tot, &upnum.eot, &upnum.up, &upnum.pre_detection_count)) {
						record_data(fp, PRESSING, &upnum.tot);
					}
				} // end of remaining time
            }
        }
        if (!response_once) {
            record_data(fp, OMISSION, &upnum.eot);

            GPIO_CLR = 1 << SHOCK_PIN;
            GPIO_CLR = 1 << RELAY_PIN;
            GPIO_CLR = 1 << CUE_PIN;
            printf("CS and US off.\n");
        } // end of trial, CS off, US off
        record_data(fp, ENDTRIAL, &upnum.eot);
        fflush(fp);
    }
    // end of session
    record_data(fp, ENDSESSION, &upnum.eot);

    fclose(fp);

    GPIO_CLR = 1 << HOUSE_PIN;
    // GPIO_CLR = 1 << CUE_PIN;
    INP_GPIO(SENSOR_PIN); // Initiating SENSOR_PIN
    INP_GPIO(CUE_PIN);
    INP_GPIO(HOUSE_PIN);
    INP_GPIO(SHOCK_PIN);
    INP_GPIO(RELAY_PIN);

} // end of aa()

// fm is is pointer to file name
// ignore_hab = 1 is ignoring 3 minutes habituation time.
// short_duration = 1 is fast_testing
// trail_number
void aap (char* fm, int ignore_hab, int short_duration, int trial_number, int all_leakage) {
    if (!short_duration) {
        sleep(10);
    }


// RPI.c: Set up gpi pointer for direct register access
    setup_io();

/************************************************************************\
* You are about to change the GPIO settings of your computer.          *
* Mess this up and it will stop working!                               *
* It might be a good idea to 'sync' before running this program        *
* so at least you still have your code changes written to the SD-card! *
\************************************************************************/
    // Random init
    // Run rand() several times to make it random
    time_t wt;
    wt = time(NULL);
    srand((unsigned) time(&wt));
    rand();rand();rand();

    int i, j; // iter
    int response_once = 0;
    int flash_off = 0;
    int flash_type = 0;
    int shine_off = 0;
    int rescue_once = 0;
    int iti[trial_number], *ptoiti;
    ptoiti = iti;
    int up = 20, *ptoup;
    ptoup = &up;
    // leakage pointer
    int leakage[trial_number], *ptoleakage;
    ptoleakage = leakage;

    //ITI pseudorandom: random sampling without replacement
    if (short_duration) {
        int ITI[5] = {1, 2, 3, 4, 5};
        int len_of_ITI = sizeof(ITI)/sizeof(int);
        combination(ptoiti, ITI, len_of_ITI, trial_number);
        print_array(ptoiti, trial_number);
    }
    else {
        int ITI[5] = {15, 18, 21, 24, 27};
        int len_of_ITI = sizeof(ITI)/sizeof(int);
        combination(ptoiti, ITI, len_of_ITI, trial_number);
        print_array(ptoiti, trial_number);
    }

    //Leakage pseudorandom: random sampling without replacement
    if (all_leakage == 0) {
        int LEAKAGE[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        int len_of_LEAKAGE = sizeof(LEAKAGE)/sizeof(int);
        combination(ptoleakage, LEAKAGE, len_of_LEAKAGE, trial_number);
    }
    else if (all_leakage == 1) {
        int LEAKAGE[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        int len_of_LEAKAGE = sizeof(LEAKAGE)/sizeof(int);
        combination(ptoleakage, LEAKAGE, len_of_LEAKAGE, trial_number);
    } else {
        int LEAKAGE[10] = {1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
        int len_of_LEAKAGE = sizeof(LEAKAGE)/sizeof(int);
        combination(ptoleakage, LEAKAGE, len_of_LEAKAGE, trial_number);
    }

    // init timer
    struct timespec start_of_time, eot, tot, flash_of_time, shine_of_time, remain_of_time, trial_of_time;
    clock_gettime(CLOCK_REALTIME, &start_of_time);
    clock_gettime(CLOCK_REALTIME, &eot);
    clock_gettime(CLOCK_REALTIME, &tot);
    clock_gettime(CLOCK_REALTIME, &flash_of_time);
    clock_gettime(CLOCK_REALTIME, &shine_of_time);
    clock_gettime(CLOCK_REALTIME, &remain_of_time);
    clock_gettime(CLOCK_REALTIME, &trial_of_time);

    int hab_duration;
    if (ignore_hab) {
        hab_duration = 10;
    }
    else {
        hab_duration = 180;
    }
    int CS_duration;
    int US_duration;
    int US_interruption = 0;
    int trial_duration;
    if (short_duration) {
        CS_duration = 10;
        US_duration = 5, US_interruption = 0;
        trial_duration = CS_duration + US_duration;
    }
    else {
        CS_duration = 10;
        US_duration = 5;
        trial_duration = CS_duration + US_duration;
    }

    // Set GPIO pins 7-11 to output
    INP_GPIO(SENSOR_PIN); // initiating SENSOR_PIN
    INP_GPIO(CUE_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(CUE_PIN);
    GPIO_CLR = 1 << CUE_PIN;
    INP_GPIO(AHOUSE_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(AHOUSE_PIN);
    GPIO_CLR = 1 << AHOUSE_PIN; // turn AHOUSE_PIN off
    INP_GPIO(HOUSE_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(HOUSE_PIN);
    GPIO_CLR = 1 << HOUSE_PIN; // turn HOUSE_PIN off
    INP_GPIO(RELAY_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(RELAY_PIN);
    GPIO_CLR = 1 << RELAY_PIN; // turn RELAY_PIN off
    INP_GPIO(SHOCK_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(SHOCK_PIN);
    GPIO_CLR = 1 << SHOCK_PIN; // turn PSHOCK_PIN off

    FILE *fp;
    fp=fopen(fm, "a");
    clock_gettime(CLOCK_REALTIME, &start_of_time);
    record_data(fp, OPENFILE, &start_of_time);
    record_data(fp, STARTSESSION, &start_of_time);
    fflush(fp);

    // EVERYTHING START HERE
    eot = start_of_time;
    record_data(fp, STARTITI, &eot);

    eot.tv_sec += hab_duration;
    while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
        if(sensor_detection(SENSOR_PIN, &tot, &eot, ptoup)) {
            record_data(fp, PRESSING, &tot);
        }
    }
    record_data(fp, ENDITI, &eot);
    fflush(fp);
    // Hab end

    GPIO_SET = 1 << AHOUSE_PIN; // turn AHOUSE_PIN on
    GPIO_SET = 1 << HOUSE_PIN; // turn HOUSE_PIN on
    for (i = 0; i < trial_number; i++) {
        fprintf(fp, "%d,%lu.%09lu\n",STARTITI, eot.tv_sec, eot.tv_nsec);
        printf("Trial NO.: %d\n", i + 1);
        printf("%d, trial start: %lu.%03lu, ITI: %d\n", STARTITI, eot.tv_sec - start_of_time.tv_sec, eot.tv_nsec/1000000, iti[i]);

        // ITI start
        eot.tv_sec = eot.tv_sec + iti[i];
        while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
            if(sensor_detection(SENSOR_PIN, &tot, &eot, ptoup)) {
                record_data(fp, PRESSING, &tot);
            }
        }
        fprintf(fp, "%d,%lu.%09lu\n", ENDITI, eot.tv_sec, eot.tv_nsec);
        printf("%d, CS on.\n", ENDITI);
        // ITI end

        // CS on
        GPIO_SET = 1 << CUE_PIN; // turn CUE_PIN on
        trial_of_time = eot;
        trial_of_time.tv_sec += trial_duration;
        eot.tv_sec += + CS_duration;

        if (response_once = sensor_detection(SENSOR_PIN, &tot, &eot, ptoup)) {
            if (leakage[i]) { // A-A A-E AO
				GPIO_CLR = 1 << AHOUSE_PIN; // turn AHOUSE_PIN off
                record_data(fp, AVOIDANCE, &tot);
				printf("Leakage condition\n");

				// Remaining time of Avoidance
				// flash the house light
				flash_off = 1;
                flash_of_time = tot;
                flash_of_time.tv_sec += 1; // turn AHOUSE_PIN on atfer 1 sec

                if (check_time(&flash_of_time, &eot)) { // flash followed by US
					remain_of_time = flash_of_time;
                    flash_type = 1;
                    printf("Type I A-?\n");
                }
                else{ // US followed by flash
                    remain_of_time = eot;
                    flash_type = 0;
                    printf("Type II A-?\n");
                }

                // waiting for rescue
                rescue_once = 1;
                while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
    				if (sensor_detection(SENSOR_PIN, &tot, &remain_of_time, ptoup)) { // A-A,type 1 flash
                        GPIO_CLR = 1 << CUE_PIN; // turn CUE_PIN off: A-A
                        GPIO_CLR = 1 << HOUSE_PIN; //shine_counter = 0;// A-A shine
                        shine_off = 1;
                        rescue_once = 0;
						record_data(fp, AVOIDANCE + 10, &tot);
                        // Remaining time of Avoidance
    					// trouble shine the phouse light
						shine_of_time = tot;
						shine_of_time.tv_sec += 1;
                        // 1. type I but flashed
                        // 2. type I not flashed yet
                        if (flash_off) { // 2.
							remain_of_time = flash_of_time;
                            flash_off = 2;
                            printf("type I not flashed yet\n");

                        } else { // 1.
							remain_of_time = shine_of_time;
                            printf("type I flashed\n");
                        }
    					// start shine and no US
                        // remaining time until end

                        while((tot.tv_sec < trial_of_time.tv_sec) | (tot.tv_nsec < trial_of_time.tv_nsec)) {
                            if (sensor_detection(SENSOR_PIN, &tot, &remain_of_time, ptoup)) {
                                record_data(fp, PRESSING, &tot);
                            }
                            else if (flash_off == 2) { // rescue flash == 2
								GPIO_SET = 1 << AHOUSE_PIN;
								flash_off = 0;
								remain_of_time = shine_of_time;
								eot = trial_of_time;
							}
                            else if (shine_off == 1) {
								GPIO_SET = 1 << HOUSE_PIN;
								shine_off = 0;
								remain_of_time = trial_of_time;
								eot = trial_of_time;

							}
                        }
                    }
                    else if (flash_off) {
                        GPIO_SET = 1 << AHOUSE_PIN;
                        flash_off = 0;
                        remain_of_time = eot;
                        //printf("flash_off = 1\n");
                    }
                }

                if (rescue_once) { // A-E
                    GPIO_SET = 1 << SHOCK_PIN; // turn PSHOCK_PIN on
                    printf("PUS on.\n");
                    eot = trial_of_time;

                    // remember to trun flash on if in case
                    if (!flash_type) {
                        remain_of_time = flash_of_time;
                    }
                    else {
                        remain_of_time = eot;
                    }
                    while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
                        if (sensor_detection(SENSOR_PIN, &tot, &remain_of_time, ptoup)) {
                            if (rescue_once) {
                                GPIO_CLR = 1 << SHOCK_PIN;
                                GPIO_CLR = 1 << CUE_PIN;
                                record_data(fp, OMISSION + 10, &eot);
                                rescue_once = 0;
                            }
                            else {
                                record_data(fp, PRESSING, &tot);
                            }
                        }
                        else if (flash_off){
                            GPIO_SET = 1 << AHOUSE_PIN;
                            flash_off = 0;
                            remain_of_time = eot;
                        }
					} // while
                } // end of A-E
                if (rescue_once) { // A-O
                    //GPIO_CLR = 1 << RELAY_PIN;
                    GPIO_CLR = 1 << SHOCK_PIN;
                    GPIO_CLR = 1 << CUE_PIN;
                    record_data(fp, OMISSION + 10, &eot);
                }
            } // end of leakage
            else { // yoked condition
				GPIO_CLR = 1 << CUE_PIN;
                GPIO_CLR = 1 << AHOUSE_PIN; // turn AHOUSE_PIN off
                GPIO_CLR = 1 << HOUSE_PIN; // turn HOUSE_PIN off
				record_data(fp, AVOIDANCE, &tot);

				// Remaining time of Avoidance
				// flash the house light
                flash_off = 1;
				flash_of_time = tot;
				flash_of_time.tv_sec += 1;
                remain_of_time = flash_of_time;
				eot = trial_of_time; //QQ
				while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
					if(sensor_detection(SENSOR_PIN, &tot, &remain_of_time, ptoup)) {
						record_data(fp, PRESSING, &tot);
					}
					else if (flash_off) { // 4
						GPIO_SET = 1 << AHOUSE_PIN; // turn AHOUSE_PIN on
                        GPIO_SET = 1 << HOUSE_PIN; // turn HOUSE_PIN on
						flash_off = 0;
						remain_of_time = eot;
					}

				}// end of remaining time
			}
        }
        // else US on
        else {
            GPIO_SET = 1 << RELAY_PIN; // turn RELAY_PIN on
            GPIO_SET = 1 << SHOCK_PIN; // turn PSHOCK_PIN on
            printf("US on.\n");
            eot = trial_of_time;
            // US_interruption = sensor_detection(SENSOR_PIN, &tot, &eot);
            if (sensor_detection(SENSOR_PIN, &tot, &eot, ptoup)) {
                if (leakage[i] == 1) {
                    GPIO_CLR = 1 << RELAY_PIN;
                    record_data(fp, ESCAPE, &tot);
                    rescue_once = 1; // wait for rescue;

                    // Remaining time of Escape
                    while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
                        if(sensor_detection(SENSOR_PIN, &tot, &eot, ptoup)) { //E-E
                            if (rescue_once) {
								record_data(fp, ESCAPE + 10, &tot);
                                GPIO_CLR = 1 << SHOCK_PIN; // turn SHOCK_PIN off
                                GPIO_CLR = 1 << CUE_PIN;
                                rescue_once = 0;
                            } else {
                                record_data(fp, PRESSING, &tot);

                            }
                        }
                    }
                    if (rescue_once) { // E-O
						GPIO_CLR = 1 << RELAY_PIN;
                        GPIO_CLR = 1 << SHOCK_PIN;
                        GPIO_CLR = 1 << CUE_PIN;
                        record_data(fp, OMISSION + 10, &eot);
                    }
                    // end of remaining time

                }// end of leakage
                else { //yoked condition
                    GPIO_CLR = 1 << RELAY_PIN;
                    GPIO_CLR = 1 << SHOCK_PIN; // turn SHOCK_PIN off
                    GPIO_CLR = 1 << CUE_PIN;
                    record_data(fp, ESCAPE, &tot);

                    // Remaining time of Escape
                    while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
                        if(sensor_detection(SENSOR_PIN, &tot, &eot, ptoup)) {
                            record_data(fp, PRESSING, &tot);
                        }
                    }
                    // end of remaining time
                }
            }
            else {
                GPIO_CLR = 1 << RELAY_PIN;
                GPIO_CLR = 1 << SHOCK_PIN;
                GPIO_CLR = 1 << CUE_PIN;
                record_data(fp, OMISSION, &eot);
            }

        } // end of trial
        // CS off, US off
        record_data(fp, ENDTRIAL, &eot);
        fflush(fp);
    }
    // end of session
    record_data(fp, ENDSESSION, &eot);
    fclose(fp);

    GPIO_CLR = 1 << AHOUSE_PIN;
    GPIO_CLR = 1 << HOUSE_PIN;
    //GPIO_CLR = 1 << CUE_PIN;
    INP_GPIO(CUE_PIN);
    INP_GPIO(AHOUSE_PIN);
    INP_GPIO(RELAY_PIN);
    INP_GPIO(SHOCK_PIN);
    INP_GPIO(HOUSE_PIN);

} // end of aap()


void aap_shine (char* fm, int ignore_hab, int short_duration, int trial_number, int all_leakage) {
    if (!short_duration) {
        sleep(10);
    }


// RPI.c: Set up gpi pointer for direct register access
    setup_io();

/************************************************************************\
* You are about to change the GPIO settings of your computer.          *
* Mess this up and it will stop working!                               *
* It might be a good idea to 'sync' before running this program        *
* so at least you still have your code changes written to the SD-card! *
\************************************************************************/
    // Random init
    // Run rand() several times to make it random
    time_t wt;
    wt = time(NULL);
    srand((unsigned) time(&wt));
    rand();rand();rand();

    int i, j; // iter
    int response_once = 0;
    int flash_off = 0;
    int flash_type = 0;
    int shine_counter = 0;
    int shine_check = 0;
    int max_shine = 8;
    int rescue_once = 0;
    int eot_counter, fot_counter;
    int iti[trial_number], *ptoiti;
    ptoiti = iti;
    int up = 20, *ptoup;
    ptoup = &up;
    // leakage pointer
    int leakage[trial_number], *ptoleakage;
    ptoleakage = leakage;

    //ITI pseudorandom: random sampling without replacement
    if (short_duration) {
        int ITI[5] = {1, 2, 3, 4, 5};
        int len_of_ITI = sizeof(ITI)/sizeof(int);
        combination(ptoiti, ITI, len_of_ITI, trial_number);
        print_array(ptoiti, trial_number);
    }
    else {
        int ITI[5] = {15, 18, 21, 24, 27};
        int len_of_ITI = sizeof(ITI)/sizeof(int);
        combination(ptoiti, ITI, len_of_ITI, trial_number);
        print_array(ptoiti, trial_number);
    }

    //Leakage pseudorandom: random sampling without replacement
    if (all_leakage == 0) {
        int LEAKAGE[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        int len_of_LEAKAGE = sizeof(LEAKAGE)/sizeof(int);
        combination(ptoleakage, LEAKAGE, len_of_LEAKAGE, trial_number);
    }
    else if (all_leakage == 1) {
        int LEAKAGE[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        int len_of_LEAKAGE = sizeof(LEAKAGE)/sizeof(int);
        combination(ptoleakage, LEAKAGE, len_of_LEAKAGE, trial_number);
    } else {
        int LEAKAGE[10] = {1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
        int len_of_LEAKAGE = sizeof(LEAKAGE)/sizeof(int);
        combination(ptoleakage, LEAKAGE, len_of_LEAKAGE, trial_number);
    }

    // init timer
    struct timespec start_of_time, eot, tot, tots, flash_of_time, shine_of_time, remain_of_time, trial_of_time;
    clock_gettime(CLOCK_REALTIME, &start_of_time);
    clock_gettime(CLOCK_REALTIME, &eot);
    clock_gettime(CLOCK_REALTIME, &tot);
    clock_gettime(CLOCK_REALTIME, &tots);
    clock_gettime(CLOCK_REALTIME, &flash_of_time);
    clock_gettime(CLOCK_REALTIME, &shine_of_time);
    clock_gettime(CLOCK_REALTIME, &remain_of_time);
    clock_gettime(CLOCK_REALTIME, &trial_of_time);

    int hab_duration;
    if (ignore_hab) {
        hab_duration = 10;
    }
    else {
        hab_duration = 180;
    }
    int CS_duration;
    int US_duration;
    int US_interruption = 0;
    int trial_duration;
    if (short_duration) {
        CS_duration = 10;
        US_duration = 5, US_interruption = 0;
        trial_duration = CS_duration + US_duration;
    }
    else {
        CS_duration = 10;
        US_duration = 5;
        trial_duration = CS_duration + US_duration;
    }

    // Set GPIO pins 7-11 to output
    INP_GPIO(SENSOR_PIN); // initiating SENSOR_PIN
    INP_GPIO(CUE_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(CUE_PIN);
    GPIO_CLR = 1 << CUE_PIN;
    INP_GPIO(AHOUSE_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(AHOUSE_PIN);
    GPIO_CLR = 1 << AHOUSE_PIN; // turn AHOUSE_PIN off
    INP_GPIO(HOUSE_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(HOUSE_PIN);
    GPIO_CLR = 1 << HOUSE_PIN; // turn HOUSE_PIN off
    INP_GPIO(RELAY_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(RELAY_PIN);
    GPIO_CLR = 1 << RELAY_PIN; // turn RELAY_PIN off
    INP_GPIO(SHOCK_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(SHOCK_PIN);
    GPIO_CLR = 1 << SHOCK_PIN; // turn PSHOCK_PIN off

    FILE *fp;
    fp=fopen(fm, "a");
    fprintf(fp, "event,time\n");
    fprintf(fp, "100,%s", ctime(&wt));
    printf("%s",ctime(&wt));
    clock_gettime(CLOCK_REALTIME, &start_of_time);
    fprintf(fp, "%d,%lu.%09lu\n",STARTSESSION, start_of_time.tv_sec, start_of_time.tv_nsec);
    fflush(fp);
    printf("Session start: %lu.%09lu.\n", start_of_time.tv_sec, start_of_time.tv_nsec);

    // EVERYTHING START HERE
    eot = start_of_time;
    fprintf(fp, "%d,%lu.%09lu\n",STARTITI, eot.tv_sec, eot.tv_nsec);
    printf("Hab start: %lu.%03lu\n", eot.tv_sec - start_of_time.tv_sec, eot.tv_nsec/1000000);

    eot.tv_sec = eot.tv_sec + hab_duration;
    while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
        if(sensor_detection(SENSOR_PIN, &tot, &eot, ptoup)) {
            fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
            printf("%d, %lu.%03lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
            //usleep(rest);
        }
    }
    fprintf(fp, "%d,%lu.%09lu\n", ENDITI, eot.tv_sec, eot.tv_nsec);
    printf("%d, hab end.\n", ENDITI);
    fflush(fp);
    // Hab end

    GPIO_SET = 1 << AHOUSE_PIN; // turn AHOUSE_PIN on
    GPIO_SET = 1 << HOUSE_PIN; // turn HOUSE_PIN on
    for (i = 0; i < trial_number; i++) {
        fprintf(fp, "%d,%lu.%09lu\n",STARTITI, eot.tv_sec, eot.tv_nsec);
        printf("Trial NO.: %d\n", i + 1);
        printf("%d, trial start: %lu.%03lu, ITI: %d\n", STARTITI, eot.tv_sec - start_of_time.tv_sec, eot.tv_nsec/1000000, iti[i]);

        // ITI start
        eot.tv_sec = eot.tv_sec + iti[i];
        while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
            if(sensor_detection(SENSOR_PIN, &tot, &eot, ptoup)) {
                fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
                printf("%d, %lu.%03lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                //usleep(rest);
            }
        }
        fprintf(fp, "%d,%lu.%09lu\n", ENDITI, eot.tv_sec, eot.tv_nsec);
        printf("%d, CS on.\n", ENDITI);
        // ITI end

        // CS on
        GPIO_SET = 1 << CUE_PIN; // turn CUE_PIN on
        trial_of_time = eot;
        trial_of_time.tv_sec += trial_duration;
        eot.tv_sec += + CS_duration;

        if (response_once = sensor_detection(SENSOR_PIN, &tot, &eot, ptoup)) {
            if (leakage[i]) { // A-A A-E AO
				GPIO_CLR = 1 << AHOUSE_PIN; // turn AHOUSE_PIN off
                fprintf(fp, "%d,%lu.%09lu\n", AVOIDANCE, tot.tv_sec, tot.tv_nsec);
				printf("%d, %lu.%03lu, avoidance\n", AVOIDANCE, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
				printf("Leakage condition\n");

				// Remaining time of Avoidance
				// flash the house light
				flash_off = 1;
                flash_of_time = tot;
                flash_of_time.tv_sec += 1; // turn AHOUSE_PIN on atfer 1 sec

                if (check_time(&eot, &flash_of_time)) { // flash followed by US
					remain_of_time = flash_of_time;
                    flash_type = 1;
                    printf("Type I A-?\n");
                }
                else{ // US followed by flash
                    remain_of_time = eot;
                    flash_type = 0;
                    printf("Type II A-?\n");
                }

                // waiting for rescue
                rescue_once = 1;
                while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
    				if (sensor_detection(SENSOR_PIN, &tot, &remain_of_time, ptoup)) { // A-A,type 1 flash
                        GPIO_CLR = 1 << CUE_PIN; // turn CUE_PIN off: A-A
                        GPIO_CLR = 1 << HOUSE_PIN; //shine_counter = 0;// A-A shine
                        shine_counter = 0;
                        rescue_once = 0;

    					fprintf(fp, "%d,%lu.%09lu\n", AVOIDANCE + 10, tot.tv_sec, tot.tv_nsec);
    					printf("%d, %lu.%03lu, empathetic avoidance\n", AVOIDANCE + 10, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);

                        // Remaining time of Avoidance
    					// trouble shine the phouse light
                        fot_counter = -1;
                        if (flash_off == 1) {
                            shine_of_time = tot;
                            for (j = 0; j < 5; j++) {
                                add_time(&shine_of_time, 200000000);

                                if (check_time(&shine_of_time, &flash_of_time)) {
                                    fot_counter = j;
                                    break;
                                }
                            }
                        }

                        // 1. type I but flashed
                        // 2. type I not flashed yet

                        if (flash_off) { // 2.
                            printf("type I not flashed yet\n");
                            max_shine = 6;
                        } else { // 1.
                            printf("type I flashed\n");
                            max_shine = 5;
                        }

                        printf("flash on %d\n", fot_counter);
                        shine_of_time = tot;
                        if (fot_counter == 0) {
                            remain_of_time = flash_of_time;
                            flash_off = 2;
                        } else {
                            add_time(&shine_of_time, 200000000);
                            remain_of_time = shine_of_time;
                        }
    					// start shine and no US
                        // remaining time until end
                        shine_check = 0; // current condition: 0 is off
                        while((tot.tv_sec < trial_of_time.tv_sec) | (tot.tv_nsec < trial_of_time.tv_nsec)) {
                            if (sensor_detection(SENSOR_PIN, &tot, &remain_of_time, ptoup)) {
                                fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
                                printf("%d, %lu.%lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);

                            }
                            else {
                                if (flash_off == 2) { // rescue flash = 2
                                    GPIO_SET = 1 << AHOUSE_PIN;
                                    flash_off = 0;
                                }
                                if (shine_counter < max_shine) {
                                    if (!(fot_counter == shine_counter)) {
                                        if (shine_check) {
                                            GPIO_CLR = 1 << HOUSE_PIN; // turn HOUSE_PIN off
                                            shine_check = 0;
                                        } else {
                                            GPIO_SET = 1 << HOUSE_PIN; // turn HOUSE_PIN on
                                            shine_check = 1;
                                        }
                                    }


                                    shine_counter++;

                                    if (fot_counter == shine_counter) {

                                        remain_of_time = flash_of_time;
                                        printf("flash: %d\n", shine_counter);
                                        flash_off = 2;

                                    } else {

                                        printf("shine: %d\n", shine_counter);
                                        //printf("%lu  ", shine_of_time.tv_nsec);
                                        add_time(&shine_of_time, 200000000); // next shine_of_time
                                        remain_of_time = shine_of_time;
                                        //printf("%lu\n", shine_of_time.tv_nsec);

                                    }

                                }
                                else {
                                    remain_of_time = trial_of_time;
                                    eot = trial_of_time;
                                }
                            }
                        }
                    }
                    else if (flash_off) {
                        GPIO_SET = 1 << AHOUSE_PIN;
                        flash_off = 0;
                        remain_of_time = eot;
                        //printf("flash_off = 1\n");
                    }
                }

                if (rescue_once) { // A-E
                    GPIO_SET = 1 << SHOCK_PIN; // turn PSHOCK_PIN on
                    printf("PUS on.\n");
                    eot = trial_of_time;

                    // remember to trun flash on if in case
                    if (!flash_type) {
                        remain_of_time = flash_of_time;
                    }
                    else {
                        remain_of_time = eot;
                    }

                    while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
                        if (sensor_detection(SENSOR_PIN, &tot, &remain_of_time, ptoup)) {
                            if (rescue_once) {
                                GPIO_CLR = 1 << SHOCK_PIN;
                                GPIO_CLR = 1 << CUE_PIN;
                                fprintf(fp, "%d,%lu.%09lu\n", ESCAPE + 10, tot.tv_sec, tot.tv_nsec);
                                printf("%d, %lu.%03lu, empathetic escape\n", ESCAPE + 10, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                                rescue_once = 0;
                            }
                            else {
                                fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
                                printf("%d, %lu.%lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);

                            }
                        }
                        else if (flash_off){
                            GPIO_SET = 1 << AHOUSE_PIN;
                            flash_off = 0;
                            remain_of_time = eot;
                        }

                    }

                } // end of A-E
                if (rescue_once) { // A-O
                    //GPIO_CLR = 1 << RELAY_PIN;
                    GPIO_CLR = 1 << SHOCK_PIN;
                    GPIO_CLR = 1 << CUE_PIN;
                    fprintf(fp, "%d,%lu.%09lu\n", OMISSION + 10, eot.tv_sec, eot.tv_nsec);
                    printf("%d, %lu.%03lu, apathetic omission\n", OMISSION, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                }
            } // end of leakage
            else { // yoked condition
				GPIO_CLR = 1 << CUE_PIN;
                GPIO_CLR = 1 << AHOUSE_PIN; // turn AHOUSE_PIN off
                GPIO_CLR = 1 << HOUSE_PIN; // turn HOUSE_PIN off
				fprintf(fp, "%d,%lu.%09lu\n", AVOIDANCE, tot.tv_sec, tot.tv_nsec);
				printf("%d, %lu.%03lu, avoidance\n", AVOIDANCE, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);

				// Remaining time of Avoidance
				// flash the house light
                flash_off = 1;
				flash_of_time = tot;
				flash_of_time.tv_sec += 1;
                remain_of_time = tot;
                add_time(&remain_of_time, 200000000);
                shine_check = 0; // HOUSE_PIN is off
                shine_counter = 1;

				eot = trial_of_time; //QQ
				while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
					if(sensor_detection(SENSOR_PIN, &tot, &remain_of_time, ptoup)) {
						fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
						printf("%d, %lu.%03lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);

					}
                    else if (shine_counter < 4) {
                        shine_counter++;
                        add_time(&remain_of_time, 200000000);
                        if (shine_check) {
                            GPIO_CLR = 1 << HOUSE_PIN; // turn HOUSE_PIN off
                            shine_check = 0;
                        } else {
                            GPIO_SET = 1 << HOUSE_PIN; // turn HOUSE_PIN on
                            shine_check = 1;
                        }


                    }
					else if (flash_off) { // 4
						GPIO_SET = 1 << AHOUSE_PIN; // turn AHOUSE_PIN on
                        GPIO_SET = 1 << HOUSE_PIN; // turn HOUSE_PIN on
						flash_off = 0;
						remain_of_time = eot;
					}

				}// end of remaining time
			}
        }
        // else US on
        else {
            GPIO_SET = 1 << RELAY_PIN; // turn RELAY_PIN on
            GPIO_SET = 1 << SHOCK_PIN; // turn PSHOCK_PIN on
            printf("US on.\n");
            eot = trial_of_time;
            // US_interruption = sensor_detection(SENSOR_PIN, &tot, &eot);
            if (sensor_detection(SENSOR_PIN, &tot, &eot, ptoup)) {
                if (leakage[i] == 1) {
                    GPIO_CLR = 1 << RELAY_PIN;
                    //GPIO_CLR = 1 << SHOCK_PIN; // turn SHOCK_PIN off
                    //GPIO_CLR = 1 << CUE_PIN;
                    fprintf(fp, "%d,%lu.%09lu\n", ESCAPE, tot.tv_sec, tot.tv_nsec);
                    printf("%d, %lu.%03lu, escape\n", ESCAPE, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                    rescue_once = 1; // wait for rescue;

                    // Remaining time of Escape
                    while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
                        if(sensor_detection(SENSOR_PIN, &tot, &eot, ptoup)) { //E-E
                            if (rescue_once) {
                                fprintf(fp, "%d,%lu.%09lu\n", ESCAPE + 10, tot.tv_sec, tot.tv_nsec);
                                printf("%d, %lu.%03lu, empathetic escape\n", ESCAPE + 10, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                                GPIO_CLR = 1 << SHOCK_PIN; // turn SHOCK_PIN off
                                GPIO_CLR = 1 << CUE_PIN;
                                rescue_once = 0;
                            } else {
                                fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
                                printf("%d, %lu.%lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                                //usleep(rest);

                            }
                        }
                    }
                    if (rescue_once) { // E-O
                        fprintf(fp, "%d,%lu.%09lu\n", OMISSION + 10, eot.tv_sec, eot.tv_nsec);
                        GPIO_CLR = 1 << RELAY_PIN;
                        GPIO_CLR = 1 << SHOCK_PIN;
                        GPIO_CLR = 1 << CUE_PIN;
                        printf("%d, %lu.%03lu, apathetic omission\n", OMISSION + 10, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                    }
                    // end of remaining time

                }// end of leakage
                else { //yoked condition
                    GPIO_CLR = 1 << RELAY_PIN;
                    GPIO_CLR = 1 << SHOCK_PIN; // turn SHOCK_PIN off
                    GPIO_CLR = 1 << CUE_PIN;
                    fprintf(fp, "%d,%lu.%09lu\n", ESCAPE, tot.tv_sec, tot.tv_nsec);
                    printf("%d, %lu.%03lu, escape\n", ESCAPE, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                    //usleep(rest);

                    // Remaining time of Escape
                    while((tot.tv_sec < eot.tv_sec) | (tot.tv_nsec < eot.tv_nsec)) {
                        if(sensor_detection(SENSOR_PIN, &tot, &eot, ptoup)) {
                            fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
                            printf("%d, %lu.%lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                            //usleep(rest);
                        }
                    }
                    // end of remaining time
                }
            }
            else {
                GPIO_CLR = 1 << RELAY_PIN;
                GPIO_CLR = 1 << SHOCK_PIN;
                GPIO_CLR = 1 << CUE_PIN;
                fprintf(fp, "%d,%lu.%09lu\n", OMISSION, eot.tv_sec, eot.tv_nsec);
                printf("%d, %lu.%03lu, omission\n", OMISSION, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
            }

        } // end of trial
        // CS off, US off
        fprintf(fp, "%d,%lu.%09lu\n", ENDTRIAL, eot.tv_sec, eot.tv_nsec);
        printf("%d, CS and US off, trial end: %lu.%03lu\n", ENDTRIAL, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
        fflush(fp);
    }
    // end of session
    fprintf(fp, "%d,%lu.%09lu\n", ENDSESSION, eot.tv_sec, eot.tv_nsec);
    printf("Session end: %lu.%03lu\n", eot.tv_sec - start_of_time.tv_sec, eot.tv_nsec/1000000);
    fclose(fp);

    GPIO_CLR = 1 << AHOUSE_PIN;
    GPIO_CLR = 1 << HOUSE_PIN;
    //GPIO_CLR = 1 << CUE_PIN;
    INP_GPIO(CUE_PIN);
    INP_GPIO(AHOUSE_PIN);
    INP_GPIO(RELAY_PIN);
    INP_GPIO(SHOCK_PIN);
    INP_GPIO(HOUSE_PIN);

} // end of aap_shine()
