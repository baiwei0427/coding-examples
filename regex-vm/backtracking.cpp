#include <iostream>
#include "backtracking.h"
#include "common.h"

using namespace std;

struct Thread {
        struct Inst *pc;        // program counter
        char *sp;               // string pointer
};

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
                                                cerr << "too many threads" << endl;
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