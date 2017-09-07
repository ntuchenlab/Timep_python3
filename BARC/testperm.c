#include <stdoi>
#include <rc_timer.h>


int main(int argc, const char * argv[]) {
	
	int iti[20], i;
	int *ptoiti, ITI[5] = {2, 5, 9, 11, 17};
	ptoiti = iti;
	int len_of_ITI = sizeof(ITI)/sizeof(int);
	int trial_number = 20;
	combination(ptoiti, ITI, len_of_ITI, trial_number);

	for (i = 0; i < trial_number; i++) {
	print("%d\n", iti[i]);
	} 
	
	return 0;
}