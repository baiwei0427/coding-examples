#include <stdio.h>
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

struct Thread {
        struct Inst *pc;        // program counter
        char *sp;               // string pointer
};

struct Thread thread(struct Inst *pc, char *sp);

// regular expression matching functions 
int recursive(struct Inst *pc, char *sp);
int recursiveloop(struct Inst *pc, char *sp);
int backtrackingvm(struct Inst *prog, char *input);

int main(int argc, char **argv)
{        
        if (argc != 2) {
                return EXIT_FAILURE;
        }

        const int n = atoi(argv[1]);
        if (n <= 0) {
                return EXIT_FAILURE;
        }

        // to store instructions of regex pattern a?^n a^n
        struct Inst regex_insts[3 * n + 1];
        // to store subject string a^n
        char str[n + 1];

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

        // construct subject string
        for (int i = 0; i < n; i++) {
                str[i] = 'a';
        }
        str[n] = 0;

        if (backtrackingvm(regex_insts, str) == 1) {
                printf ("Match\n");
        } else {
                printf("Don't match\n");
        }

        return EXIT_SUCCESS;
}

struct Thread thread(struct Inst *pc, char *sp)
{
        struct Thread t = {.pc = pc, .sp = sp};
        return t;
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
                        if (recursive(pc->x, sp) == 1) {
                                return 1;
                        }
                        return recursive(pc->y, sp);
        }

        return -1;
}

int recursiveloop(struct Inst *pc, char *sp)
{
        while (1) {
                switch(pc->opcode) {
                        case Char:
                                if (*sp != pc->c) {
                                        // don't match
                                        return 0;
                                }
                                // move to next instruction pointer and string pointer
                                pc++;
                                sp++;
                                continue;
                        case Match:
                                // match
                                return 1;
                        case Jmp:
                                // jump to another instruction                        
                                pc = pc->x;
                                continue;
                        case Split:
                                if (recursiveloop(pc->x, sp) == 1) {
                                        return 1;
                                }
                                pc = pc->y;
                                continue;                                
                }
                return -1;               
        }

        return -1;
}

int backtrackingvm(struct Inst *prog, char *input)
{
        int MAXTHREAD = 25;
        struct Thread ready[MAXTHREAD];
        int nready;
        struct Inst *pc;
        char *sp;

        // queue initial thread
        ready[0] = thread(prog, input);
        nready = 1;

        // run threads in stack order
        while (nready > 0) {
                --nready;       // pop state for next thread to run
                pc = ready[nready].pc;
                sp = ready[nready].sp;

                // run a thread
                while (1) {
                        switch (pc->opcode) {
                                case Char:
                                        if (*sp != pc->c) {
                                                // don't match & kill this thread
                                                goto Dead;
                                        }
                                        
                                        // move to next instruction pointer and string pointer
                                        pc++;
                                        sp++;
                                        continue;
                                case Match:
                                        // match
                                        return 1;
                                case Jmp:
                                        // jump to another instruction                        
                                        pc = pc->x;
                                        continue;
                                case Split:
                                        if (nready >= MAXTHREAD) {
                                                fprintf(stderr, "too many threads\n");
                                                return -1;
                                        }

                                        // queue a new thread
                                        ready[nready++] = thread(pc->y, sp);
                                        pc = pc->x;     // continue current thread
                                        // printf("# of threads: %d ", nready);
                                        continue; 
                        }                
                }

                Dead:;
        }

        return 0;
}