#ifndef BACKTRACKING_H
#define BACKTRACKING_H

struct Thread {
        struct Inst *pc;        // program counter
        char *sp;               // string pointer
};

struct Thread thread(struct Inst *pc, char *sp);

// backtracking-based regex matching functions
int recursive(struct Inst *pc, char *sp);
int recursiveloop(struct Inst *pc, char *sp);
int backtrackingvm(struct Inst *prog, char *input);

#endif