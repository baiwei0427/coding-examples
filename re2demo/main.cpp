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
        // Total # of submatches in the regex pattern
        int num_submatch = 1 + re.NumberOfCapturingGroups();
        // Index of the last non empty submatch
        int last_nonempty_submatch = num_submatch - 1;
        re2::StringPiece submatches[num_submatch];

        // If the string does not match the pattern
        if (!re.Match(s, startpos, endpos, RE2::UNANCHORED, submatches, num_submatch)) {
                return -1;
        }

        // Find the last non empty submatch 
        while (!submatches[last_nonempty_submatch].data()) {
                last_nonempty_submatch--;
        }

        int count = min(last_nonempty_submatch + 1, ovecsize / 3);
        // Extract submatch information as much as possible
        for (int i = 0; i < count; i++) {
                // An empty submatch
                if (!submatches[i].data()) {
                        ovector[2 * i] = -1;
                        ovector[2 * i + 1] = -1;
                } else {
                        ovector[2 * i] = submatches[i].data() - s;
                        ovector[2 * i + 1] = ovector[2 * i] + submatches[i].length();
                }
        }

        // The output vector has enough space to store the information of  
        // all non empty submatches + empty submatches among non empty submatches
        if (last_nonempty_submatch + 1 <= ovecsize / 3) {
                return last_nonempty_submatch + 1;
        }

        // Truncate empty submatches at the tail of 'ovector'        
        if (!submatches[ovecsize / 3 - 1].data()) {
                for (int i = ovecsize / 3 - 2; i >= 0; i--) {
                        if (submatches[i].data()) {
                                return i + 1;
                        }
                }
        }
        
        return 0;        
}

