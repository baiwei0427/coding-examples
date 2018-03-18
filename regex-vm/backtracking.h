#ifndef BACKTRACKING_H
#define BACKTRACKING_H

// backtracking-based regex matching functions
int recursive(struct Inst *pc, char *sp);
int recursiveloop(struct Inst *pc, char *sp);
int backtrackingvm(struct Inst *prog, char *input);

#endif