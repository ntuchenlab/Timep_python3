#ifndef rc_timer_h
#define rc_timer_h

typedef struct up_dnum {
	int up;
	int pre_detection_count;
	struct timespec start_of_time, eot, tot, flash_of_time;
	struct timespec ba, ma, ta, tb;

} dnum;

#define OPENFILE 100
#define STARTSESSION 6
#define ENDSESSION 7

#define STARTITI 8
#define ENDITI  4
#define ENDTRIAL 5

#define PRESSING 0

#define AVOIDANCE 1
#define ESCAPE 2
#define OMISSION 3

void setup_io();
int rc_detect(int pin_to_detect);
int rc_detect_annotation_free(int pin_to_detect);
int sensor_detection(int dd, struct timespec* t, struct timespec* e, int* up);
int sensor_detection_pre(int dd, struct timespec* t, struct timespec* e, int* up, int* pre_detection_count);
int sensor_detection_timer(int dd, struct timespec* t, struct timespec* e, int* up);
int sensor_detection_log(int dd, struct timespec* t, struct timespec* e);
int check_time(struct timespec* t, struct timespec* e);
int add_time(struct timespec* t, long a);
long diff_time(struct timespec* t, struct timespec* e);
long diff_time_ns(struct timespec* t, struct timespec* e);
long diff_time_s(struct timespec* t, struct timespec* e);
void print_array(int* p,int tn);
int record_data(FILE* fp, int tag, struct timespec* t);
// combination need this
int uniform(int rangeLow, int rangeHigh);

// randon combination
void combination(int* p, int iti[], int select, int tn);

#define SENSOR_PIN 17
#define CUE_PIN 27
#define HOUSE_PIN 22
#define AHOUSE_PIN 18 // Active house pin
#define SHOCK_PIN 23
#define RELAY_PIN 24 //Passive shock pin

#endif /* rc_timer_h */
