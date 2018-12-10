#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <hs/hs.h>

// This is the function that will be called for each match that occurs. @a ctx
// is to allow you to have some application-specific state that you will get
// access to for each match. In our simple example we're just going to use it
// to pass in the pattern that was being searched for so we can print it out.
static int eventHandler(unsigned int id, unsigned long long from,
                        unsigned long long to, unsigned int flags, void *ctx) 
{
    printf("Match for pattern \"%s\" at offset [%llu, %llu)\n", (char *)ctx, from, to);
    return 0;
}

int main(int argc, char **argv)
{   
	char *pattern;      // regex pattern 	
	char *subject;		// subject string 

    hs_database_t *database = NULL;
    hs_compile_error_t *compile_err = NULL;
    hs_scratch_t *scratch = NULL;

    if (argc != 3) {
		fprintf(stderr, "%s regex_pattern subject_string\n", argv[0]);
  		return -1;
  	}

    pattern = argv[1];
	subject = argv[2];

    // First, we compile the pattern provided on the command line.
    // The compiler will analyse the given pattern and either return a 
    // compiled Hyperscan database, or an error message explaining 
    // why the pattern didn't compile.
    if (hs_compile(pattern, 0, HS_MODE_BLOCK, NULL, &database, &compile_err) != HS_SUCCESS) {
        fprintf(stderr, "ERROR: Unable to compile pattern \"%s\": %s\n",
                pattern, compile_err->message);
        hs_free_compile_error(compile_err);
        return -1;
    }    

    // Second, we need to allocates a large enough region of scratch space to support a given database. 
    if (hs_alloc_scratch(database, &scratch) != HS_SUCCESS) {
        fprintf(stderr, "ERROR: Unable to allocate scratch space. Exiting.\n");
        hs_free_database(database);
        return -1;
    }

    // Third, we scan the subject string for the pattern represented in the bytecode. 
    if (hs_scan(database, subject, strlen(subject), 0, scratch, eventHandler, 
                pattern) != HS_SUCCESS) {
        fprintf(stderr, "ERROR: Unable to scan input subject string. Exiting.\n");
        hs_free_scratch(scratch);
        hs_free_database(database);
        return -1;
    }   

    // Scanning is complete, any matches have been handled。
    // So now we just clean up and exit.
    hs_free_scratch(scratch);
    hs_free_database(database);
    return 0;
}