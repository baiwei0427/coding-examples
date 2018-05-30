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
        char *regex, *subject;

        // We require two arguments: pattern, subject string. 
	if (argc != 4) {
		cerr << argv[0] << " regex_pattern subject_string start_offset" << endl;
  		return -1;
  	}

        regex = argv[1];
        subject = argv[2];
        startoffset = atoi(argv[3]);
        if (startoffset < 0 || startoffset >= strlen(argv[2])) {
                cerr << "Invalid start_offset value " << startoffset << endl;
                return -1;
        }

        RE2::Options opt;
        //opt.set_case_sensitive(false);  // Do case less matching  
        //opt.set_dot_nl(true);           // Let . match \n

        RE2 re(regex, opt);
        
        if (!re.ok()) {
                cerr << "Fail to compile regex pattern " << regex << endl;
                return -1;
        }

        rc = msc_regexec_ex(re, subject, strlen(subject), startoffset, ovector, ovecsize);

        if (rc < 0) {
                cout << "String did not match the pattern" << endl;  
                return 0;  
        } 

        cout << "rc = " << rc << endl; 
	
        // Match succeded 
        cout << "Match succeeded at offset " << ovector[0] << endl;

        // The output vector wasn't big enough 
        if (rc == 0) {
                rc = ovecsize / 3;
                cout << "ovector only has room for " << rc - 1 << " captured substrings" << endl;
        }

        // Show substrings stored in the output vector by number. 
        for (int i = 0; i < rc; i++) {
                char *substring_start = subject + ovector[2 * i];
                int substring_length = ovector[2 * i + 1] - ovector[2 * i];

                printf("Submatch %d [%d, %d): %.*s\n", 
                       i + 1,			// submatch ID
                       ovector[2 * i],		// submatch start offset
                       ovector[2 * i + 1],	// submatch end offset
                       substring_length,	// submatch length
                       substring_start);	// submatch pointer
  	}
        
        return 0;
}

int msc_regexec_ex(RE2 &re, const char *s, unsigned int slen, 
                   int startoffset, int *ovector, int ovecsize)
{
        size_t startpos = startoffset;
        const size_t endpos = slen;
        // # of submatches in the regex pattern
        unsigned int num_submatch = 1 + re.NumberOfCapturingGroups();
        // # of submatches that we can store
        unsigned int num_submatch_stored = (num_submatch > ovecsize / 3)? ovecsize / 3 : num_submatch;
        
        re2::StringPiece submatches[num_submatch_stored];

        while (startpos < endpos && 
               re.Match(s, startpos, endpos, RE2::UNANCHORED, submatches, num_submatch_stored)) {
                
                // The pattern matches an empty string
                if (submatches[0].length() == 0) {
                        // Match from the next byte
                        startpos++;
                        continue;
                }

                // The pattern matches a non-empty string
                for (unsigned int i = 0; i < num_submatch_stored; i++) {
                        ovector[2 * i] = submatches[i].data() - s;
                        ovector[2 * i + 1] = ovector[2 * i] + submatches[i].length();
                }

                // The output vector is not big enough to store all submatches
                if (num_submatch_stored < num_submatch) {
                        return 0;
                } else {
                        return num_submatch;
                }
        }

        // Not matching!
        return -1;
}

