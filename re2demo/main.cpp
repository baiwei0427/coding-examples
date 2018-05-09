#include <iostream>
#include <string>
#include "re2.h"

using namespace std;


int main(int argc, char **argv)
{       
        /* We require two arguments: pattern, subject string. */
        if (argc != 3) {
                cerr << argv[0] << " [regex pattern] [subject string]" << endl;
                return -1;
        }

        int 
        string s;
        RE2 re("(" + string(argv[1]) + ")");
        
        if (!re.ok()) {
                cerr << "Fail to compile regex pattern " << argv[1] << endl;
        }
        
        if (RE2::PartialMatch(argv[2], re, &s)) {
                cout << "Match" << endl;
                cout << s << endl;
        } else {
                cout << "Don't match" << endl;
        }

        return 0;
}

