#include <iostream>
#include <string>
#include <vector>
#include <re2/re2.h>
#include <re2/stringpiece.h>

using namespace std;

int main(int argc, char **argv)
{       
        // We require two arguments: pattern, subject string. 
	if (argc != 3) {
		cerr << argv[0] << " [regex pattern] [subject string]" << endl;
  		return -1;
  	}

        unsigned int count = 10;
        re2::StringPiece submatches[count];
        RE2::Arg args[count];
        RE2::Arg* args_ptrs[count];
        RE2 re("(" + string(argv[1]) + ")");

        if (!re.ok()) {
                cerr << "Fail to compile regex pattern " << argv[1] << endl;
        }

        // Get # of groups in the regex pattern 
        unsigned int args_count = re.NumberOfCapturingGroups();

        if (args_count > count) {
                args_count = count;
        }

        for (unsigned int i = 0; i < args_count; i++) {
                args[i] = &submatches[i];
                args_ptrs[i] = &args[i];
        }

        if (RE2::PartialMatchN(re2::StringPiece(argv[2]), re, args_ptrs, args_count)) {
                cout << "Match" << endl;
                for (unsigned int i = 0; i < args_count; i++) {
                        printf("offset %d length %d: ", submatches[i].data() - argv[2], submatches[i].length());
                        printf("%.*s\n", submatches[i].length(), submatches[i].data());
                }

        } else {
                cout << "Don't match" << endl;
        }

        return 0;
}

