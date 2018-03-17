#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "backtracking.h"

int main(int argc, char **argv)
{        
        if (argc != 3) {
                fprintf (stderr, "No enough arguments\n");
                return EXIT_FAILURE;
        }

        const int n = atoi(argv[1]);
        char *str = argv[2];    // subject string 

        if (n <= 0) {
                return EXIT_FAILURE;
        }

        // to store instructions of regex pattern a?^n a^n
        struct Inst regex_insts[3 * n + 1];

        // construct instructions
        for (int i = 0; i < n; i++) {
                // instructions of a?
                regex_insts[2 * i].opcode = Split;
                regex_insts[2 * i].x = &regex_insts[2 * i + 1];
                regex_insts[2 * i].y = &regex_insts[2 * i + 2];

                regex_insts[2 * i + 1].opcode = Char;
                regex_insts[2 * i + 1].c = 'a';

                // instructions of a
                regex_insts[2 * n + i].opcode = Char;
                regex_insts[2 * n + i].c = 'a';
        }
        regex_insts[3 * n].opcode = Match;

        if (backtrackingvm(regex_insts, str) == 1) {
                printf ("Match\n");
        } else {
                printf("Don't match\n");
        }

        return EXIT_SUCCESS;
}


