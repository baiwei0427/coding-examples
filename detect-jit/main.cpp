#include <iostream>
#include "pcre.h"
using namespace std;

int main()
{
        char info[100];
        int result = pcre_config(PCRE_CONFIG_JIT, (void*)info);
        
        if (result == 1) {
                cout << "Support JIT" << endl;
        } else if (result == 0) {
                cout << "Do not support JIT" << endl;
        } else {
                cout << "Unknown errors" << endl;
        }

        return 0;
}