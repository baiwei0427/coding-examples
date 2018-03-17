#ifndef COMMON_H
#define COMMON_H

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

#endif