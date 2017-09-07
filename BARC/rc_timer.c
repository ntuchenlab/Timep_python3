
#include "RPI.h"
#include "rc_timer.h"


//
// old style calling function
//
//
int rc_detect(int pin_to_detect) {

    int count = 0;
    // INP_GPIO(pin_to_detect);
    // printf("1 %d\n", GET_GPIO(pin_to_detect));
    OUT_GPIO(pin_to_detect);
    // GET_GPIO(pin_to_detect);
    GPIO_CLR = 1 << pin_to_detect;

    // GET_GPIO(pin_to_detect);

    INP_GPIO(pin_to_detect);
    // printf("4 %d\n", GET_GPIO(pin_to_detect));
    while (!GET_GPIO(pin_to_detect)) {
        count++;
        // printf("%d\n", count);
        if (count == 7){
            break;
        } // if (count == 7)

    } // while (!GET_GPIO(pin_to_detect))
    // printf("count = %d\n", count);

    return count;

} // rc_detect

//
// whithout annotation
//
int rc_detect_annotation_free(int pin_to_detect) {
    int count = 0;
    OUT_GPIO(pin_to_detect);
    GPIO_CLR = 1 << pin_to_detect;
    INP_GPIO(pin_to_detect);
    while (!GET_GPIO(pin_to_detect)) {
        count++;
        if (count == 7){
            break;
        } // if
    } // while
    return count;
} // rc_detect

int sensor_detection(int dd, struct timespec* t, struct timespec* e, int* up) {

    // debug
    struct timespec ba, ma;
    clock_gettime(CLOCK_REALTIME, &ba);
    //clock_gettime(CLOCK_REALTIME, &ma);
    long diff_return;
    //debug

    //clock_gettime(CLOCK_REALTIME, t); // t is already a pointer
    int dis1, dis2;
    int detection_start = 0;
    int pre_detection_count = 0;
    int response_count = 0;
    int response = 0;

    // dis2 = rc_detect(dd); // Do not call function anymore
    dis2 = 0;
    OUT_GPIO(dd);
    // GPIO_CLR = 1 << dd;
    INP_GPIO(dd);
    while (!GET_GPIO(dd)) {
        dis2++;
        if (dis2 == 7){
            break;
        } // if
    } // while

    while ((t->tv_sec < e->tv_sec) | (t->tv_nsec < e->tv_nsec)) {
        dis1 = dis2;
        // dis2 = rc_detect(dd); // Do not call function anymore
		dis2 = 0;
		OUT_GPIO(dd);
		// GPIO_CLR = 1 << dd;
		INP_GPIO(dd);
		while (!GET_GPIO(dd)) {
			dis2++;
			if (dis2 == 7){
				break;
			} // if
		} // while

        // 5 then start, otherwise 0
        if (detection_start) {
            if ((dis1 == 0) & (dis2 == 0)) {
                response_count++;
                if (response_count > 5) {
                    response = 1;
                    *up = 16400;
                    break;
                }
            }
            else {
                response_count = 0;
            }
        } // if (detection_start)
        else {
            if ((dis1 > 5) & (dis2 > 5)) {
                pre_detection_count++;
                if (pre_detection_count > *up) {
                    detection_start = 1;

                    *up = 0;
                    clock_gettime(CLOCK_REALTIME, &ma);
                    diff_return = diff_time(&ma, &ba);
                    printf("sensitivity = %d\n", diff_return);
                }
            }
            else {
                pre_detection_count = 0;
            }
        } // else
        clock_gettime(CLOCK_REALTIME, t);
    } // while

    return response;
} // sensor_detection

int sensor_detection_pre(int dd, struct timespec* t, struct timespec* e, int* up, int* pre_detection_count) {

    // debug
    struct timespec ba, ma;
    clock_gettime(CLOCK_REALTIME, &ba);
    //clock_gettime(CLOCK_REALTIME, &ma); // skip this
    long diff_return;
    //debug

    // clock_gettime(CLOCK_REALTIME, t); // t is already a pointer
    int dis1, dis2;
    int detection_start = 0;
    int response_count = 0;
    int response = 0;

    // dis2 = rc_detect(dd); // Do not call function anymore
    dis2 = 0;
    OUT_GPIO(dd);
    //GPIO_CLR = 1 << dd; // skip this
    INP_GPIO(dd);
    while (!GET_GPIO(dd)) {
        dis2++;
        if (dis2 == 7){
            break;
        } // if
    } // while

    while ((t->tv_sec < e->tv_sec) | (t->tv_nsec < e->tv_nsec)) {
        dis1 = dis2;
        // dis2 = rc_detect(dd); // Do not call function anymore
		dis2 = 0;
		OUT_GPIO(dd);
		//GPIO_CLR = 1 << dd; // skip this
		INP_GPIO(dd);
		while (!GET_GPIO(dd)) {
			dis2++;
			if (dis2 == 7){
				break;
			} // if
		} // while

        // 5 then start, otherwise 0
        if (detection_start) {
            if ((dis1 == 0) & (dis2 == 0)) {
                response_count++;
                if (response_count > 5) {
                    response = 1;
                    *up = 2100;
                    return response;
                }
            }
            else {
                response_count = 0;
            }
        } // if (detection_start)
        else {
            if ((dis1 > 5) & (dis2 > 5)) {
                *pre_detection_count += 1;
                if (*pre_detection_count > *up) {
                    detection_start = 1;
                    *up = 0;
                    *pre_detection_count = 0;
                    clock_gettime(CLOCK_REALTIME, &ma);
                    diff_return = diff_time_ns(&ma, &ba);
                    printf("sensitivity = %d\n", diff_return);
                }
            }
            else {
                *pre_detection_count = 0;
            }
        } // else
        clock_gettime(CLOCK_REALTIME, t);
    } // while

    return response;
} // sensor_detection

int sensor_detection_timer(int dd, struct timespec* t, struct timespec* e, int* up) {

    // debug
    struct timespec ba, ma, ta, tb;
    clock_gettime(CLOCK_REALTIME, &ba);
    clock_gettime(CLOCK_REALTIME, &ma);
    clock_gettime(CLOCK_REALTIME, &ta);
    clock_gettime(CLOCK_REALTIME, &tb);
    long diff_return;
    //debug

    clock_gettime(CLOCK_REALTIME, t); // t is already a pointer
    int dis1, dis2;
    int detection_start = 0;
    int pre_detection_count = 0;
    int response_count = 0;
    int response = 0;

    /*
    // dis1 = rc_detect(dd); // Do not call function anymore
    dis1 = 0;
    OUT_GPIO(dd);
    GPIO_CLR = 1 << dd;
    INP_GPIO(dd);
    while (!GET_GPIO(dd)) {
        dis1++;
        if (dis1 == 7){
            break;
        } // if
    } // while
    */
    // dis2 = rc_detect(dd); // Do not call function anymore
    dis2 = 0;
    OUT_GPIO(dd);
    GPIO_CLR = 1 << dd;
    INP_GPIO(dd);
    while (!GET_GPIO(dd)) {
        dis2++;
        if (dis2 == 7){
            break;
        } // if
    } // while

    while(!((t->tv_sec >= e->tv_sec) & (t->tv_nsec >= e->tv_nsec))) {
        dis1 = dis2;
        // dis2 = rc_detect(dd); // Do not call function anymore
		dis2 = 0;
		OUT_GPIO(dd);
		GPIO_CLR = 1 << dd;
		INP_GPIO(dd);
		while (!GET_GPIO(dd)) {
			dis2++;
			if (dis2 == 7){
				break;
			} // if
		} // while

        // 5 then start, otherwise 0
        if (detection_start) {
            if ((dis1 == 0) & (dis2 == 0)) {
				if (response_count == 0){
					clock_gettime(CLOCK_REALTIME, &ta);
				}
                response_count++;
                if (response_count > 5) {
					clock_gettime(CLOCK_REALTIME, &tb);
					diff_return = diff_time_ns(&tb, &ta);
					printf("bar sensitivity = %ld\n", diff_return);
                    response = 1;
                    *up = 16400;
                    break;
                }
            }
            else {
                response_count = 0;
            }
        }
        else {
            if ((dis1 > 5) & (dis2 > 5)) {
                pre_detection_count++;
                if (pre_detection_count > *up) {
                    detection_start = 1;
                    clock_gettime(CLOCK_REALTIME, &ma);
                    diff_return = diff_time(&ma, &ba);
                    printf("sensitivity = %ld\n", diff_return);
                    *up = 0;
                }
            }
            else {
                pre_detection_count = 0;
                clock_gettime(CLOCK_REALTIME, &ba);
            }
        }
        clock_gettime(CLOCK_REALTIME, t);
    }

    return response;
}

// Out of date
int sensor_detection_log(int dd, struct timespec* t, struct timespec* e) {
    clock_gettime(CLOCK_REALTIME, t); // t is already a pointer
    FILE *fp;
    fp=fopen("rc_log", "a");
    fprintf(fp, "A new rc\n");
    int dis1, dis2;
    int detection_start = 0;
    int pre_detection_count = 0;
    int response = 0;
    dis1 = rc_detect(dd);
    dis2 = rc_detect(dd);
    fprintf(fp, "%d ", dis1);
    while(!((t->tv_sec >= e->tv_sec) & (t->tv_nsec >= e->tv_nsec))) {
        dis1 = dis2;
        dis2 = rc_detect(dd);
        fprintf(fp, "%d ", dis2);
        //printf("count = %d,%d\n", dis1, dis2);

        // 5 then start, otherwise 0
        if (detection_start) {
            if ((dis1 == 0) & (dis2 == 0)) {
                response = 1;
                fprintf(fp, "responsed!\n");
                *e = *t;
                break;
            }
        }
        else {
            if ((dis1 == 7) & (dis2 == 7)) {
                pre_detection_count++;
                if (pre_detection_count > 4) {
                    detection_start = 1;
                    fprintf(fp, "detection_start!");
                }
            }
            else {
                pre_detection_count = 0;
            }
        }

        //

        clock_gettime(CLOCK_REALTIME, t);
    }

    fclose(fp);
    return response;
}

int check_time(struct timespec* t, struct timespec* e) {
    if (t->tv_sec == e->tv_sec) {
        if (t->tv_nsec < e->tv_nsec) {
            return 1;
        } else {
            return 0;
        }
    } else if (t->tv_sec < e->tv_sec) {
        return 1;
    } else {
        return 0;
    }
} // check_time

int add_time(struct timespec* t, long a) {
    if (t->tv_nsec + a >= 1000000000) {
        t->tv_nsec = t->tv_nsec + a - 1000000000;
        t->tv_sec++;
    }
    else {
        t->tv_nsec = t->tv_nsec + a;
    } // else
    return 1;
} // add_time

long diff_time(struct timespec* t, struct timespec* e) {
	// t - e
    long diff_s = diff_time_s(t, e);
    long diff_ns = diff_time_ns(t, e);

    if (diff_ns >= 0) {
        if (diff_s == 0) {
            return diff_ns;
        } else if (diff_s == -1) {
            return 1000000000 - diff_ns;
        } else if (diff_s > 0) {
            printf("diff > 1 sec\n");
            return diff_ns;
        } else if (diff_s < -1) {
            return 1000000000 - diff_ns;
        }
    } else {
        if (diff_s == 0) {
            return diff_ns;
        } else if (diff_s == 1) {
            return 1000000000 - diff_ns;
        } else if (diff_s > 1) {
            printf("diff > 1 sec\n");
            return 1000000000 - diff_ns;
        } else if (diff_s < 0) {
            printf("diff > 1 sec\n");
            return diff_ns;
        }
    } // else
} // diff_time

long diff_time_ns(struct timespec* t, struct timespec* e) {

    return t->tv_nsec - e->tv_nsec;

} // diff_time_ns

long diff_time_s(struct timespec* t, struct timespec* e) {

    return t->tv_sec - e->tv_sec;

} // diff_time_s


// combination need this function
int uniform(int rangeLow, int rangeHigh) {
    return (int)(rand() / (RAND_MAX + 1.0) * (rangeHigh - rangeLow + 1) + rangeLow);
}

// p is output []
// iti is candidate
// select is length of candidate
// tn is trial_number
void combination(int* p, int iti[], int select, int tn) {
    int i; // iter
    int k, w, len, temp; // index
    //int iti_dupe[select];
    len = select - 1;
    for (i = 0; i < tn; i++) { // range tn
        w = i%(select);
        if (w == len) {
            *(p + i) = iti[len];
        } else {
            k = uniform(w, len);
            //printf("k = %d\n", k);
            *(p + i) = iti[k];
            // swap k, w
            temp = iti[k];
            iti[k] = iti[w];
            iti[w] = temp;
        }
    }
    //return 0;
}

void print_array(int* p,int tn) {
    int j; // iter
    printf("[");
    for (j = 0; j < tn; j++) {
        if (j == (tn - 1)){
            printf("%d]\n", *(p + j));
        }
        else if ((j%5) == 4) {
            printf("%d,\n", *(p + j));
        }
        else {
            printf("%d, ", *(p + j));

        }
    }
}

int record_data(FILE* fp, int tag, struct timespec* t) {
	switch(tag) {
		case 0 : // PRESSING
			fprintf(fp, "%d,%lu.%09lu\n", tag, t->tv_sec, t->tv_nsec); // no space after comma
            printf("%d, %lu.%03lu\n", tag, t->tv_sec, t->tv_nsec/1000000);
            break;
		case 1 :
            fprintf(fp,"%d,%lu.%09lu\n", tag, t->tv_sec, t->tv_nsec);
            printf("%d, %lu.%03lu, avoidance\n", tag, t->tv_sec, t->tv_nsec/1000000);
			break;
		case 2 :
            fprintf(fp,"%d,%lu.%09lu\n", tag, t->tv_sec, t->tv_nsec);
            printf("%d, %lu.%03lu, escape\n", tag, t->tv_sec, t->tv_nsec/1000000);
			break;
        case 3 :
            fprintf(fp,"%d,%lu.%09lu\n", tag, t->tv_sec, t->tv_nsec);
            printf("%d, %lu.%03lu, omission\n", tag, t->tv_sec, t->tv_nsec/1000000);
            break;
        case 5 :
            fprintf(fp,"%d,%lu.%09lu\n", tag, t->tv_sec, t->tv_nsec);
            printf("%d, trial end: %lu.%03lu\n", tag, t->tv_sec, t->tv_nsec/1000000);
            break;
		case 6 : // STARTSESSION
			fprintf(fp, "%d,%lu.%09lu\n", tag, t->tv_sec, t->tv_nsec);
			printf("%d, Session start: %lu.%09lu\n", tag, t->tv_sec, t->tv_nsec);
			break;
		case 7 :	// ENDSESSION
			fprintf(fp, "%d,%lu.%09lu\n", tag, t->tv_sec, t->tv_nsec);
			printf("%d, Session end: %lu.%03lu\n", tag, t->tv_sec, t->tv_nsec/1000000);
			break;
		case 8 : // STARTITI
			fprintf(fp, "%d,%lu.%09lu\n", tag, t->tv_sec, t->tv_nsec);
			printf("%d, Hab start: %lu.%03lu\n", tag, t->tv_sec, t->tv_nsec/1000000);
			break;
		case 4 : // ENDITI
			fprintf(fp, "%d,%lu.%09lu\n", tag, t->tv_sec, t->tv_nsec);
			printf("%d, Hab end: %lu.%03lu\n", tag, t->tv_sec, t->tv_nsec/1000000);
			break;
		case 11 :
            fprintf(fp,"%d,%lu.%09lu\n", tag, t->tv_sec, t->tv_nsec);
            printf("%d, %lu.%03lu, empathetic avoidance\n", tag, t->tv_sec, t->tv_nsec/1000000);
			break;
		case 12 :
            fprintf(fp,"%d,%lu.%09lu\n", tag, t->tv_sec, t->tv_nsec);
            printf("%d, %lu.%03lu, empathetic escape\n", tag, t->tv_sec, t->tv_nsec/1000000);
			break;
		case 13 :
            fprintf(fp,"%d,%lu.%09lu\n", tag, t->tv_sec, t->tv_nsec);
            printf("%d, %lu.%03lu, apathetic omission\n", tag, t->tv_sec, t->tv_nsec/1000000);
			break;
		case 100 :
			fprintf(fp, "event,time\n");
			fprintf(fp, "100,%s", ctime(&t->tv_sec));
			printf("%s",ctime(&t->tv_sec));
			break;
		default :
			printf("No data recorded!\n");
	}

	return 1;
}
