#include <vector>
#include "thompson.h"
#include "common.h"

using namespace std;

struct Thread
{
        struct Inst *pc;        // program counter

        bool operator ==(struct Thread &x) 
        {
                return pc == x.pc;
        }
};

struct Thread thread(struct Inst *pc)
{
        struct Thread t = {.pc = pc};
        return t;
}

// Add the thread 't' only if no identical thread exists in the 'list'
void addthread(vector<struct Thread> *list, struct Thread t)
{
        unsigned int size = list->size();
        for (unsigned int i = 0; i < size; i++) {
                // identical thread exists
                if ((*list)[i] == t) {
                        return;
                }
        }

        list->push_back(t);
}

// Ken Thompson's algorithm
int thompsonvm(struct Inst *prog, char *input)
{
        vector<struct Thread> *clist, *nlist, *tmp;
        struct Inst *pc;
        char *sp;

        clist = new vector<struct Thread>;
        nlist = new vector<struct Thread>;

        // initial thread
        addthread(clist, thread(prog));

        // for each char in the subject string
        for (sp = input; *sp != 0; sp++) {
                // for each active thread
                for (unsigned int i = 0; i < clist->size(); i++) {
                        pc = (*clist)[i].pc;
                        
                        switch (pc->opcode) {
                                case Char:
                                        if (*sp == pc->c) {
                                                addthread(nlist, thread(pc + 1));
                                        } 
                                        break;
                                case Match:
                                        return 1;
                                case Jmp:
                                        addthread(clist, thread(pc->x));
                                        break;
                                case Split:
                                        addthread(clist, thread(pc->x));
                                        addthread(clist, thread(pc->y));
                                        break;                                        
                        }
                }

                // swap clist and nlist
                tmp = clist;
                clist = nlist;
                nlist = tmp;

                nlist->clear();
        }

        delete clist;
        delete nlist;

        return 0;
}
