//
// How to access GPIO registers from C-code on the Raspberry-Pi
// Example program
// 15-January-2012
// Dom and Gert
// Revised: 15-Feb-2013
// Access from ARM Running Linux

#include <errno.h>
#include "RPI.h"
#include "Hab.h"
#include "AA.h"


int main(int argc, char **argv) {
    // find path
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        strcat(cwd, "/../Fan_data/");
        strcat(cwd, argv[1]);
        fprintf(stdout, "Saving file: %s\n", cwd);
    }
    else {
        perror("getcwd() error");
        return 0;
    }

    // arguement
    char sure[10];
    int i; // iter

    if (argc == 3) {
        system("clear");
        printf("%s\n", argv[1]);
        //char fm[strlen(argv[1])];
        //strcpy(fm, argv[1]);
        for (i = 0; i < strlen(argv[2]); i++) {
            if (argv[2][i] == 'h') {
                printf("***************************\n");
                printf("*                         *\n");
                printf("* Habituation: 5 minutes  *\n");
                printf("*                         *\n");
                printf("***************************\n");
                printf("Are you sure? (y) ");
                scanf("%s", sure);
                if (sure[0] == 'y') {
                    printf("Adding data into %s\n", argv[1]);
                    standalone_hab(cwd, 300);
                }
                else {
                    printf("Aborting program...\n");
                    return 1;
                }
            }
            
            else if (argv[2][i] == 'r') {
                printf("***************************\n");
                printf("*                         *\n");
                printf("* Habituation: 5 minutes  *\n");
                printf("*                         *\n");
                printf("***************************\n");
                printf("Are you sure? (y) ");
                scanf("%s", sure);
                if (sure[0] == 'y') {
                    printf("Adding data into %s\n", argv[1]);
                    repeated_hab(cwd, 10);
                }
                else {
                    printf("Aborting program...\n");
                    return 1;
                }
            }
            else if (argv[2][i] == 'a') { // AA training
                printf("*****************************\n");
                printf("*                           *\n");
                printf("* Active Avoidance Training *\n");
                printf("* Habituation: 3 minutes    *\n");
                printf("* 50 trials, ITI: 21 s      *\n");
                printf("*                           *\n");
                printf("*****************************\n");
                printf("Are you sure? (y) ");
                scanf("%s", sure);
                if (sure[0] == 'y') {
                    printf("Adding data into %s\n", argv[1]);
                    aa(cwd, 0, 0, 50); // **************************************
                }
                else {
                    printf("Aborting program...\n");
                    return 1;
                }

            }
            else if (argv[2][i] == 'p') { // Paired AA training
                printf("*****************************\n");
                printf("*                           *\n");
                printf("* Paired AA Training        *\n");
                printf("* Habituation: 10 seconds    *\n");
                printf("* 50 trials, ITI: 21 s      *\n");
                printf("*                           *\n");
                printf("*****************************\n");
                printf("Are you sure? (y) ");
                scanf("%s", sure);
                if (sure[0] == 'y') {
                    printf("Adding data into %s\n", argv[1]);
                    aap(cwd, 1, 0, 50, 2); // **************************************
                }
                else {
                    printf("Aborting program...\n");
                    return 1;
                }

            }
            else if (argv[2][i] == 'z') { // speed Paired AA training
                printf("*****************************\n");
                printf("*                           *\n");
                printf("* Fast Paired AA Training   *\n");
                printf("* Habituation: 10 seconds   *\n");
                printf("* 2 trials, ITI: 3 s        *\n");
                printf("*                           *\n");
                printf("*****************************\n");
                printf("Are you sure? (y) ");
                scanf("%s", sure);
                if (sure[0] == 'y') {
                    printf("Adding data into %s\n", argv[1]);
                    aap(cwd, 1, 1, 2, 1); // **************************************
                }
                else {
                    printf("Aborting program...\n");
                    return 1;
                }

            }
            else if (argv[2][i] == 't') { // 10 AA training
                printf("*****************************\n");
                printf("*                           *\n");
                printf("* Active Avoidance Training *\n");
                printf("* Habituation: 3 minutes    *\n");
                printf("* 10 trials, ITI: 21 s      *\n");
                printf("*                           *\n");
                printf("*****************************\n");
                printf("Are you sure? (y) ");
                scanf("%s", sure);
                if (sure[0] == 'y') {
                    printf("Adding data into %s\n", argv[1]);
                    aa(cwd, 0, 0, 10); // **************************************
                }
                else {
                    printf("Aborting program...\n");
                    return 1;
                }

            }
            else if (argv[2][i] == 's') { // speed AA testing
                printf("*****************************\n");
                printf("*                           *\n");
                printf("* Fast AA testing           *\n");
                printf("* Habituation: 10 seconds   *\n");
                printf("* 5 trials, ITI: 3 s        *\n");
                printf("*                           *\n");
                printf("*****************************\n");
                printf("Are you sure? (y) ");
                scanf("%s", sure);
                if (sure[0] == 'y') {
                    printf("Adding data into %s\n", argv[1]);
                    aa(cwd, 1, 1, 5); // **************************************
                }
                else {
                    printf("Aborting program...\n");
                    return 1;
                }

            }
            else if (argv[2][i] == 'f') { // fast hab
                printf("***************************\n");
                printf("*                         *\n");
                printf("*  Fast bar testing: 10 s *\n");
                printf("*                         *\n");
                printf("***************************\n");
                printf("Are you sure? (y) ");
                scanf("%s", sure);
                if (sure[0] == 'y') {
                    printf("Adding data into %s\n", argv[1]);
                    standalone_hab(cwd, 10);
                }
                else {
                    printf("Aborting program...\n");
                    return 1;
                }

            }
            else if (argv[2][i] == 't') { // fast hab
                printf("***************************\n");
                printf("*                         *\n");
                printf("*  Fast bar testing: 10 s *\n");
                printf("*  with timer             *\n");
                printf("*                         *\n");
                printf("***************************\n");
                printf("Are you sure? (y) ");
                scanf("%s", sure);
                if (sure[0] == 'y') {
                    printf("Adding data into %s\n", argv[1]);
                    standalone_hab(cwd, 10);
                }
                else {
                    printf("Aborting program...\n");
                    return 1;
                }

            }
            else {
                printf("\n");
                printf("The correct format is:\n");
                printf("    sudo %s file_name [options]\n", argv[0]);
                printf("\n");
                printf("[options]\n");
                printf("    a   Active avoidance training\n");
                printf("    s   Fast AA testing\n");
                printf("    h   Habituation: 5 minutes\n");
                printf("    f   Fast testing: 10 seconds\n");
                printf("    t   10 AA training\n");
                printf("    p   Paired AA training\n");
                printf("\n");
                return 1;
            }
        } // for (i = 0; i < strlen(argv[2]); i++)

    } // else if (argc == 3)

    return 0;
}
