#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

enum {  // operation code
        Char,
        Match,
        Jmp,
        Split
};

struct Inst {
        int opcode;
        int c;
        struct Inst *x;
        struct Inst *y;
};

int recursive(struct Inst *pc, char *sp);

int main(int argc, char **argv)
{
        // instructions of regex pattern "a+b+" 
        struct Inst regex_insts[5] = {
                {.opcode = Char, .c = 'a'},                                     // 0  char a
                {.opcode = Split, .x = &regex_insts[0], .y = &regex_insts[2]},  // 1  split 0 2
                {.opcode = Char, .c = 'b'},                                     // 2  char b
                {.opcode = Split, .x = &regex_insts[2], .y = &regex_insts[4]},  // 3  split 2 4
                {.opcode = Match}                                               // 4  match
        };

        if (argc != 2) {
                return EXIT_FAILURE;
        }

        if (recursive(regex_insts, argv[1]) == 1) {
                printf ("Match\n");
        } else {
                printf("Don't match\n");
        }

        return EXIT_SUCCESS;
}

int recursive(struct Inst *pc, char *sp)
{
        switch(pc->opcode) {
                case Char:
                        if (*sp != pc->c) {
                                // don't match
                                return 0;
                        }
                        // move to next instruction pointer and string pointer
                        return recursive(pc + 1, sp + 1);
                case Match:
                        // match
                        return 1;
                case Jmp:
                        // jump to another instruction
                        return recursive(pc->x, sp);
                case Split:
                        if (recursive(pc->x, sp) == 1)
                                return 1;
                        return recursive(pc->y, sp);
        }

        return -1;
}