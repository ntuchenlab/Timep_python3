#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int uniform(int rangeLow, int rangeHigh);
void combination(int* p, int iti[], int select, int tn);

int main(int argc, const char * argv[]) {
	
	int q = 0;
	time_t wt;
    wt = time(NULL);
    srand((unsigned) time(&wt));
    rand();rand();rand();

	int iti[20], i;
	int *ptoiti, ITI[5] = {2, 5, 9, 11, 17};
	ptoiti = iti;
	int len_of_ITI = sizeof(ITI)/sizeof(int);
	int trial_number = 20;
	combination(ptoiti, ITI, len_of_ITI, trial_number);

	printf("%d\n", RAND_MAX);
	for (i = 0; i < trial_number; i++) {
		printf("%d\n", iti[i]);
	} 
	
	return 0;
}

// combination need this function
int uniform(int rangeLow, int rangeHigh) {
    return (int)(rand() / (RAND_MAX + 1.0) * (rangeHigh - rangeLow + 1) + rangeLow);
}


// p is output []
// iti is candidate
// select is length of candidate
// tn is trial_number
void combination(int* p, int iti[], int select, int tn) {
    int i, j; // iter
    int k, w, temp; // index
    int iti_dupe[select];
    for (i = 0; i < tn; i++) { // range tn
        w = i%select;
        if (w == 4) {
        	*(p + i) = iti[4];
        } else {
        	k = uniform(w, select - 1);
        	printf("k = %d\n", k);
        	*(p + i) = iti[k];
        	// swap k, w
        	temp = iti[k];
        	iti[k] = iti[w];
        	iti[w] = temp;
        }
    }
    //return 0;
}