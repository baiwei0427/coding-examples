#include <iostream>
#include <string>
#include <vector>
#include <re2/re2.h>
#include <re2/stringpiece.h>

using namespace std;

int msc_regexec_capture(RE2 &re, const char *s, unsigned int slen, int *ovector, int ovecsize, char **error_msg);

int main(int argc, char **argv)
{       
        int ovecsize = 30;
        int ovector[ovecsize];
        int rc; 

        // We require two arguments: pattern, subject string. 
	if (argc != 3) {
		cerr << argv[0] << " [regex pattern] [subject string]" << endl;
  		return -1;
  	}

        RE2 re("(" + string(argv[1]) + ")");
        if (!re.ok()) {
                cerr << "Fail to compile regex pattern " << argv[1] << endl;
                return -1;
        }

        rc = msc_regexec_capture(re, argv[2], strlen(argv[2]), ovector, ovecsize, NULL);
        if (rc > 0) {
                cout << "Match" << endl;
                for (int i = 0; i < rc; i++) {
                        printf("Submatch %d [%d, %d]: %.*s\n", 
                               i + 1,                                           // submatch ID
                               ovector[2 * i],                                  // submatch start offset
                               ovector[2 * i + 1],                              // submatch end offset
                               ovector[2 * i + 1] - ovector[2 * i] + 1,         // submatch length
                               argv[2] + ovector[2 * i]);                       // submatch pointer
                }
        } else {
                cout << "Don't match" << endl;
        }
        return 0;
}

int msc_regexec_capture(RE2 &re, const char *s, unsigned int slen, int *ovector, int ovecsize, char **error_msg)
{
        // Construct subject string
        re2::StringPiece str(s, slen); 
        unsigned int count = re.NumberOfCapturingGroups();

        // Max # of submatches that we can store
        if (count > ovecsize / 3) {
                count = ovecsize / 3;
        }

        re2::StringPiece submatches[count];
        RE2::Arg args[count];
        RE2::Arg* args_ptrs[count];        

        for (unsigned int i = 0; i < count; i++) {
                args[i] = &submatches[i];
                args_ptrs[i] = &args[i];
        }

        if (RE2::PartialMatchN(str, re, args_ptrs, count)) {
                for (unsigned int i = 0; i < count; i++) {
                        ovector[2 * i] = submatches[i].data() - s;
                        ovector[2 * i + 1] = ovector[2 * i] + submatches[i].length() - 1;
                }
                return count;
        } else {
                cout << "Don't match" << endl;
                return 0;
        }        
}

