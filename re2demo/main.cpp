#include <iostream>
#include <string>
#include <vector>
#include <re2/re2.h>
#include <re2/stringpiece.h>

using namespace std;

// Executes regular expression
int msc_regexec_ex(RE2 &re, const char *s, unsigned int slen, 
                   int startoffset, int *ovector, int ovecsize);

int main(int argc, char **argv)
{       
        int ovecsize = 30, rc, startoffset;
        int ovector[ovecsize];

        // We require two arguments: pattern, subject string. 
	if (argc != 4) {
		cerr << argv[0] << " regex_pattern subject_string start_offset" << endl;
  		return -1;
  	}

        startoffset = atoi(argv[3]);
        if (startoffset < 0 || startoffset >= strlen(argv[2])) {
                cerr << "Invalid start_offset value " << startoffset << endl;
                return -1;
        }

        RE2::Options opt;
        //opt.set_case_sensitive(false);  // Do case less matching  
        //opt.set_dot_nl(true);           // Let . match \n

        RE2 re(argv[1], opt);
        
        if (!re.ok()) {
                cerr << "Fail to compile regex pattern " << argv[1] << endl;
                return -1;
        }

        rc = msc_regexec_ex(re, argv[2], strlen(argv[2]), startoffset, ovector, ovecsize);

        if (rc > 0) {
                cout << "Match" << endl;
                for (int i = 0; i < rc; i++) {
                        printf("Submatch %d [%d, %d): %.*s\n", 
                               i + 1,                                           // submatch ID
                               ovector[2 * i],                                  // submatch start offset
                               ovector[2 * i + 1],                              // submatch end offset
                               ovector[2 * i + 1] - ovector[2 * i],             // submatch length
                               argv[2] + ovector[2 * i]);                       // submatch pointer
                }
        } else {
                cout << "Don't match" << endl;
        }
        return 0;
}

int msc_regexec_ex(RE2 &re, const char *s, unsigned int slen, 
                   int startoffset, int *ovector, int ovecsize)
{
        size_t startpos = startoffset;
        const size_t endpos = slen;
        // # of submatches
        unsigned int count = 1 + re.NumberOfCapturingGroups();

        // Max # of submatches that we can store
        if (count > ovecsize / 3) {
                count = ovecsize / 3;
        }

        re2::StringPiece submatches[count];

        while (startpos < endpos && 
               re.Match(s, startpos, endpos, RE2::UNANCHORED, submatches, count)) {
                
                // The pattern matches an empty string.  
                if (submatches[0].length() == 0) {
                        // Match from the next byte
                        startpos++;
                        continue;
                }

                // The pattern matches a non-empty string
                for (unsigned int i = 0; i < count; i++) {
                        ovector[2 * i] = submatches[i].data() - s;
                        ovector[2 * i + 1] = ovector[2 * i] + submatches[i].length();
                }
                return count;
        }

        // Not matching!
        return 0;
}

