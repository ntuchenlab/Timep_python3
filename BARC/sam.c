#include <errno.h>
#include "RPI.h"
#include "Hab.h"
#include "AA.h"
#include "rc_timer.h"

int main(int argc, char **argv) {
    // Random init
    // Run rand() several times to make it random
    time_t wt;
    wt = time(NULL);
    srand((unsigned) time(&wt));
    rand();rand();rand();

    int trial_number = 50;
    int iti[trial_number], *ptoiti;
    ptoiti = iti;

    int ITI[5] = {15, 18, 21, 24, 27};
    int len_of_ITI = sizeof(ITI)/sizeof(int);
    combination(ptoiti, ITI, len_of_ITI, trial_number);
    print_array(ptoiti, trial_number);

    dnum upnum;
    clock_gettime(CLOCK_REALTIME, &upnum.tot);
    upnum.eot = upnum.tot;
    upnum.eot.tv_sec += 2;
    printf("destination: %lu.%09lu\n", upnum.eot.tv_sec, upnum.eot.tv_nsec);
    int i = 0, j;
    while (i < 30) {
        j = check_time(&upnum.tot, &upnum.eot);
        printf("%d %lu.%09lu\n", j, upnum.tot.tv_sec, upnum.tot.tv_nsec);
        usleep(200000);
        clock_gettime(CLOCK_REALTIME, &upnum.tot);
        i++;

    }

    return 0;
}
