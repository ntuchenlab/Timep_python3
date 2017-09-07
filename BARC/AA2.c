//
// How to access GPIO registers from C-code on the Raspberry-Pi
// Example program
// 15-January-2012
// Dom and Gert
// Revised: 15-Feb-2013
// Access from ARM Running Linux

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

    int STARTSESSION = 6;
    int ENDSESSION = 7;

    int PRESSING = 0;
    
    int STARTITI = 8;
    int ENDITI = 4;

    int AVOIDANCE = 1;
    int ESCAPE = 2;
    int OMISSION = 3;

    int ENDTRIAL = 5;
    
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
    int flash_off = 0;
    int response_once = 0;
 
    int iti[trial_number], *ptoiti;
    ptoiti = iti;
    int up = 20, *ptoup;
    ptoup = &up;

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
    struct timespec start_of_time, end_of_time, tot, flash_of_time;
    clock_gettime(CLOCK_REALTIME, &start_of_time);
    clock_gettime(CLOCK_REALTIME, &end_of_time);
    clock_gettime(CLOCK_REALTIME, &tot);
    clock_gettime(CLOCK_REALTIME, &flash_of_time);

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
        CS_duration = 5;
        US_duration = 2, US_interruption = 0;
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
    INP_GPIO(HOUSE_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(HOUSE_PIN);
    GPIO_CLR = 1 << HOUSE_PIN; // turn HOUSE_PIN off
    INP_GPIO(SHOCK_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(SHOCK_PIN);
    GPIO_CLR = 1 << SHOCK_PIN; // turn SHOCK_PIN off
    INP_GPIO(RELAY_PIN); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(RELAY_PIN);
    GPIO_CLR = 1 << RELAY_PIN; // turn SHOCK_PIN off
    
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
    end_of_time = start_of_time;
    fprintf(fp, "%d,%lu.%09lu\n",STARTITI, end_of_time.tv_sec, end_of_time.tv_nsec);
    printf("Hab start: %lu.%03lu\n", end_of_time.tv_sec - start_of_time.tv_sec, end_of_time.tv_nsec/1000000);

    end_of_time.tv_sec = end_of_time.tv_sec + hab_duration;
    while((tot.tv_sec < end_of_time.tv_sec) | (tot.tv_nsec < end_of_time.tv_nsec)) {
        if(sensor_detection(SENSOR_PIN, &tot, &end_of_time, ptoup)) {
            fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
            printf("%d, %lu.%03lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
            //usleep(rest);
        }
    }
    fprintf(fp, "%d,%lu.%09lu\n", ENDITI, end_of_time.tv_sec, end_of_time.tv_nsec);
    printf("%d, hab end.\n", ENDITI);
    fflush(fp);
    // Hab end
    
    GPIO_SET = 1 << HOUSE_PIN; // turn HOUSE_PIN on
    for (i = 0; i < trial_number; i++) {
        fprintf(fp, "%d,%lu.%09lu\n",STARTITI, end_of_time.tv_sec, end_of_time.tv_nsec);
        printf("Trial NO.: %d\n", i + 1);
        printf("%d, trial start: %lu.%03lu, ITI: %d\n", STARTITI, end_of_time.tv_sec - start_of_time.tv_sec, end_of_time.tv_nsec/1000000, iti[i]);
        
        // ITI start
        end_of_time.tv_sec = end_of_time.tv_sec + iti[i];
        while((tot.tv_sec < end_of_time.tv_sec) | (tot.tv_nsec < end_of_time.tv_nsec)) {
            if(sensor_detection(SENSOR_PIN, &tot, &end_of_time, ptoup)) {
                fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
                printf("%d, %lu.%03lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                
            }
            
        }
        fprintf(fp, "%d,%lu.%09lu\n", ENDITI, end_of_time.tv_sec, end_of_time.tv_nsec);
        printf("%d, CS on.\n", ENDITI);
        // ITI end

        // CS on
        GPIO_SET = 1 << CUE_PIN; // turn CUE_PIN on
        end_of_time.tv_sec = end_of_time.tv_sec + CS_duration;
        if (response_once = sensor_detection(SENSOR_PIN, &tot, &end_of_time, ptoup)) {
            GPIO_CLR = 1 << CUE_PIN;
            GPIO_CLR = 1 << HOUSE_PIN; // flash: turn HOUSE_PIN off
            fprintf(fp, "%d,%lu.%09lu\n", AVOIDANCE, tot.tv_sec, tot.tv_nsec);
            printf("%d, %lu.%03lu, avoidance\n", AVOIDANCE, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
            //response_once = 1;

            // Remaining time of Avoidance
            // flash the house light
            flash_of_time = tot;
            flash_of_time.tv_sec += 1;
            flash_off = 1;
            
            end_of_time.tv_sec += US_duration;
        }
        else { // US on
            GPIO_SET = 1 << SHOCK_PIN; // turn SHOCK_PIN on
            GPIO_SET = 1 << RELAY_PIN; // turn RELAY_PIN on
            printf("US on.\n");
            end_of_time.tv_sec = end_of_time.tv_sec + US_duration;
            // US_interruption = sensor_detection(SENSOR_PIN, &tot, &end_of_time);
            if (response_once = sensor_detection(SENSOR_PIN, &tot, &end_of_time, ptoup)) {
                GPIO_CLR = 1 << SHOCK_PIN;
                GPIO_CLR = 1 << RELAY_PIN; // turn RELAY_PIN off
                GPIO_CLR = 1 << CUE_PIN;
                fprintf(fp, "%d,%lu.%09lu\n", ESCAPE, tot.tv_sec, tot.tv_nsec);
                printf("%d, %lu.%03lu, escape\n", ESCAPE, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                //response_once = 1;
            }
        }
        if (response_once){
            response_once = 0;
            if (!flash_off) {
                flash_of_time = end_of_time;
            }
            // Remaining time of A & Escape
            while((tot.tv_sec < end_of_time.tv_sec) | (tot.tv_nsec < end_of_time.tv_nsec)) {
                if(sensor_detection(SENSOR_PIN, &tot, &flash_of_time, ptoup)) {
                    fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
                    printf("%d, %lu.%lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                    //usleep(rest);
                }
                else if (flash_off) {
                    GPIO_SET = 1 << HOUSE_PIN; // turn HOUSE_PIN on
                    flash_of_time = end_of_time; // prolong it
                    flash_off = 0; // do it once
                }

            } // end of remaining time
        }
        else {
            fprintf(fp, "%d,%lu.%09lu\n", OMISSION, end_of_time.tv_sec, end_of_time.tv_nsec);
            //fprintf(fp, "%d,%lu.%09lu\n", ENDTRIAL, end_of_time.tv_sec, end_of_time.tv_nsec);
            GPIO_CLR = 1 << SHOCK_PIN;
            GPIO_CLR = 1 << RELAY_PIN;
            GPIO_CLR = 1 << CUE_PIN;
        } // end of trial, CS off, US off
        fprintf(fp, "%d,%lu.%09lu\n", ENDTRIAL, end_of_time.tv_sec, end_of_time.tv_nsec);
        printf("%d, CS and US off, trial end: %lu.%03lu\n", ENDTRIAL, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
        fflush(fp);
    }
    // end of session

    fprintf(fp, "%d,%lu.%09lu\n", ENDSESSION, end_of_time.tv_sec, end_of_time.tv_nsec);
    printf("Session end: %lu.%03lu\n", end_of_time.tv_sec - start_of_time.tv_sec, end_of_time.tv_nsec/1000000);
    fclose(fp);

    GPIO_CLR = 1 << HOUSE_PIN;
    //GPIO_CLR = 1 << CUE_PIN;
    INP_GPIO(CUE_PIN);
    INP_GPIO(HOUSE_PIN);
    INP_GPIO(SHOCK_PIN);
    INP_GPIO(RELAY_PIN);
    
} // end of aa()

// fm is is pointer to file name
// ignore_hab = 1 is ignoring 3 minutes habituation time.
// short_duration = 1 is fast_testing
// trail_number
// all_leakage
void aap (char* fm, int ignore_hab, int short_duration, int trial_number, int all_leakage) {
    if (!short_duration) {
        sleep(10);
    }
    
    int STARTSESSION = 6;
    int ENDSESSION = 7;

    int PRESSING = 0;
    
    int STARTITI = 8;
    int ENDITI = 4;

    int AVOIDANCE = 1;
    int ESCAPE = 2;
    int OMISSION = 3;

    int ENDTRIAL = 5;
    
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
    //int LEAKAGE[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if (all_leakage) {
        int LEAKAGE[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        int len_of_LEAKAGE = sizeof(LEAKAGE)/sizeof(int);
        combination(ptoleakage, LEAKAGE, len_of_LEAKAGE, trial_number);
    } else {
        int LEAKAGE[10] = {1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
        int len_of_LEAKAGE = sizeof(LEAKAGE)/sizeof(int);
        combination(ptoleakage, LEAKAGE, len_of_LEAKAGE, trial_number);
    }
    
    
	//printf("%d\n", leakage[0]);
	
    // init timer
    struct timespec start_of_time, end_of_time, tot, tots, flash_of_time, shine_of_time, remain_of_time, trial_of_time;
    clock_gettime(CLOCK_REALTIME, &start_of_time);
    clock_gettime(CLOCK_REALTIME, &end_of_time);
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
    end_of_time = start_of_time;
    fprintf(fp, "%d,%lu.%09lu\n",STARTITI, end_of_time.tv_sec, end_of_time.tv_nsec);
    printf("Hab start: %lu.%03lu\n", end_of_time.tv_sec - start_of_time.tv_sec, end_of_time.tv_nsec/1000000);

    end_of_time.tv_sec = end_of_time.tv_sec + hab_duration;
    while((tot.tv_sec < end_of_time.tv_sec) | (tot.tv_nsec < end_of_time.tv_nsec)) {
        if(sensor_detection(SENSOR_PIN, &tot, &end_of_time, ptoup)) {
            fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
            printf("%d, %lu.%03lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
            //usleep(rest);
        }
    }
    fprintf(fp, "%d,%lu.%09lu\n", ENDITI, end_of_time.tv_sec, end_of_time.tv_nsec);
    printf("%d, hab end.\n", ENDITI);
    fflush(fp);
    // Hab end
    


    GPIO_SET = 1 << AHOUSE_PIN; // turn AHOUSE_PIN on
    GPIO_SET = 1 << HOUSE_PIN; // turn HOUSE_PIN on
    for (i = 0; i < trial_number; i++) {
        fprintf(fp, "%d,%lu.%09lu\n",STARTITI, end_of_time.tv_sec, end_of_time.tv_nsec);
        printf("Trial NO.: %d\n", i + 1);
        printf("%d, trial start: %lu.%03lu, ITI: %d\n", STARTITI, end_of_time.tv_sec - start_of_time.tv_sec, end_of_time.tv_nsec/1000000, iti[i]);
        
        // ITI start
        end_of_time.tv_sec = end_of_time.tv_sec + iti[i];
        while((tot.tv_sec < end_of_time.tv_sec) | (tot.tv_nsec < end_of_time.tv_nsec)) {
            if(sensor_detection(SENSOR_PIN, &tot, &end_of_time, ptoup)) {
                fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
                printf("%d, %lu.%03lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                //usleep(rest);
            }
        }
        fprintf(fp, "%d,%lu.%09lu\n", ENDITI, end_of_time.tv_sec, end_of_time.tv_nsec);
        printf("%d, CS on.\n", ENDITI);
        // ITI end

        // CS on
        GPIO_SET = 1 << CUE_PIN; // turn CUE_PIN on
        trial_of_time = end_of_time;
        trial_of_time.tv_sec += trial_duration;
        end_of_time.tv_sec += + CS_duration;

        if (response_once = sensor_detection(SENSOR_PIN, &tot, &end_of_time, ptoup)) {
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
				
				// This only matters when A-E, ignore it when A-A.
                if (check_time(&end_of_time, &flash_of_time)) { // flash followed by US
					remain_of_time = flash_of_time;
                    flash_type = 1;
                    printf("Type I A-?\n");
                }
                else{ // US followed by flash
                    remain_of_time = end_of_time;
                    flash_type = 0;
                    printf("Type II A-?\n");
                }

                // waiting for rescue
                if (rescue_once = sensor_detection(SENSOR_PIN, &tot, &remain_of_time, ptoup)) { // A-A,type 1 flash

                    GPIO_CLR = 1 << CUE_PIN; // turn CUE_PIN off: A-A
                    // GPIO_CLR = 1 << HOUSE_PIN; //shine_counter = 0;// A-A shine
                    shine_counter = 0;
                    fprintf(fp, "%d,%lu.%09lu\n", AVOIDANCE + 10, tot.tv_sec, tot.tv_nsec);
                    printf("%d, %lu.%03lu, empathetic avoidance\n", AVOIDANCE + 10, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                }
                else if (flash_type){
                    GPIO_SET = 1 << AHOUSE_PIN;
                    if (rescue_once = sensor_detection(SENSOR_PIN, &tot, &end_of_time, ptoup)) { // A-A,type 1 flash

                        GPIO_CLR = 1 << CUE_PIN; // turn CUE_PIN off: A-A
                        // GPIO_CLR = 1 << HOUSE_PIN; //shine_counter = 0;// A-A shine
                        shine_counter = 0;
                        fprintf(fp, "%d,%lu.%09lu\n", AVOIDANCE + 10, tot.tv_sec, tot.tv_nsec);
                        printf("%d, %lu.%03lu, empathetic avoidance\n", AVOIDANCE + 10, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                    }
                }


                else { // A-E
                    GPIO_SET = 1 << SHOCK_PIN; // turn PSHOCK_PIN on
                    printf("PUS on.\n");
                    remain_of_time = trial_of_time;
                    end_of_time = trial_of_time;
                    if (rescue_once = response_once += sensor_detection(SENSOR_PIN, &tot, &remain_of_time, ptoup)) { // A-E
                        GPIO_CLR = 1 << SHOCK_PIN;
                        GPIO_CLR = 1 << CUE_PIN;
                        fprintf(fp, "%d,%lu.%09lu\n", ESCAPE + 10, tot.tv_sec, tot.tv_nsec);
                        printf("%d, %lu.%03lu, empathetic escape\n", ESCAPE + 10, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                        
                    }
                
                } // end of A-E

            } // end of leakage
            else { // yoked condition
                GPIO_CLR = 1 << CUE_PIN; 
                fprintf(fp, "%d,%lu.%09lu\n", AVOIDANCE, tot.tv_sec, tot.tv_nsec);
                printf("%d, %lu.%03lu, avoidance\n", AVOIDANCE, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);

                // Remaining time of Avoidance
                // flash the house light
                flash_off = 1;
                flash_of_time = tot;
                flash_of_time.tv_sec += 1;
                remain_of_time = tot;
                add_time(&remain_of_time, 200000000);
                shine_check = 0;
                shine_counter = 1;
                GPIO_CLR = 1 << AHOUSE_PIN; // turn AHOUSE_PIN off
                GPIO_CLR = 1 << HOUSE_PIN; // turn HOUSE_PIN off
                end_of_time = trial_of_time; //QQ
            } // end of yoked
        } // end of CS
            
        else { // US on
            GPIO_SET = 1 << RELAY_PIN; // turn RELAY_PIN on
            GPIO_SET = 1 << SHOCK_PIN; // turn PSHOCK_PIN on
            printf("US on.\n");
            end_of_time = trial_of_time;
            // US_interruption = sensor_detection(SENSOR_PIN, &tot, &end_of_time);
            if (response_once = sensor_detection(SENSOR_PIN, &tot, &end_of_time, ptoup)) {
                if (leakage[i]) {
                    GPIO_CLR = 1 << RELAY_PIN;
                    //GPIO_CLR = 1 << SHOCK_PIN; // turn SHOCK_PIN off
                    //GPIO_CLR = 1 << CUE_PIN;
                    fprintf(fp, "%d,%lu.%09lu\n", ESCAPE, tot.tv_sec, tot.tv_nsec);
                    printf("%d, %lu.%03lu, escape\n", ESCAPE, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                    
                    if (rescue_once = sensor_detection(SENSOR_PIN, &tot, &end_of_time, ptoup)) { // E-E
                        GPIO_CLR = 1 << SHOCK_PIN; // turn SHOCK_PIN off
                        GPIO_CLR = 1 << CUE_PIN;
                        fprintf(fp, "%d,%lu.%09lu\n", ESCAPE + 10, tot.tv_sec, tot.tv_nsec);
                        printf("%d, %lu.%03lu, empathetic escape\n", ESCAPE + 10, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                        
                    }
                }
                else { // yoked condition
                    GPIO_CLR = 1 << RELAY_PIN;
                    GPIO_CLR = 1 << SHOCK_PIN; // turn SHOCK_PIN off
                    GPIO_CLR = 1 << CUE_PIN;
                    fprintf(fp, "%d,%lu.%09lu\n", ESCAPE, tot.tv_sec, tot.tv_nsec);
                    printf("%d, %lu.%03lu, escape\n", ESCAPE, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);

                }
            } 
            
        } // end of US
    
        if (response_once) { // responded: remaining time
            while((tot.tv_sec < end_of_time.tv_sec) | (tot.tv_nsec < end_of_time.tv_nsec)) {
                if(sensor_detection(SENSOR_PIN, &tot, &end_of_time, ptoup)) {
                    fprintf(fp, "%d,%lu.%09lu\n", PRESSING, tot.tv_sec, tot.tv_nsec);
                    printf("%d, %lu.%03lu, pressing\n", PRESSING, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
                }
            }
        }
        else if (leakage && (!rescue_once)) { // responded: apathetic omission
            
            //GPIO_CLR = 1 << RELAY_PIN;
            GPIO_CLR = 1 << SHOCK_PIN;
            GPIO_CLR = 1 << CUE_PIN;
            fprintf(fp, "%d,%lu.%09lu\n", OMISSION + 10, end_of_time.tv_sec, end_of_time.tv_nsec);
            printf("%d, %lu.%03lu, apathetic omission\n", OMISSION, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
        }
        else if (!response_once) { // no response : omission
            
            GPIO_CLR = 1 << RELAY_PIN;
            GPIO_CLR = 1 << SHOCK_PIN;
            GPIO_CLR = 1 << CUE_PIN;
            fprintf(fp, "%d,%lu.%09lu\n", OMISSION, end_of_time.tv_sec, end_of_time.tv_nsec);
            printf("%d, %lu.%03lu, omission\n", OMISSION, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
        }
        fprintf(fp, "%d,%lu.%09lu\n", ENDTRIAL, end_of_time.tv_sec, end_of_time.tv_nsec);
        printf("%d, CS and US off, trial end: %lu.%03lu\n", ENDTRIAL, tot.tv_sec - start_of_time.tv_sec, tot.tv_nsec/1000000);
        fflush(fp);

    } // end of trial
    // end of session
    fprintf(fp, "%d,%lu.%09lu\n", ENDSESSION, end_of_time.tv_sec, end_of_time.tv_nsec);
    printf("Session end: %lu.%03lu\n", end_of_time.tv_sec - start_of_time.tv_sec, end_of_time.tv_nsec/1000000);
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


