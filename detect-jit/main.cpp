#include <iostream>
#include "pcre.h"
using namespace std;

int main()
{
        int support_jit;

        // check availability of just-in-time (JIT) compiler support
        if (pcre_config(PCRE_CONFIG_JIT, (void*)&support_jit) == 0) {
                if (support_jit == 1) {
                        cout << "Support JIT" << endl;        
                } else if (support_jit ==0) {
                        cout << "Do not support JIT" << endl;
                } else {
                        cout << "Unknown result " << support_jit << endl;
                }
        } else {
                cout << "Error with PCRE_CONFIG_JIT" << endl;
        }
        
        return 0;
}