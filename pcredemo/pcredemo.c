#include <stdio.h>
#include <string.h>
#include <pcre.h>

#define OVECCOUNT 30    /* should be a multiple of 3 */

int main(int argc, char **argv)
{
	char *pattern;	/* regex pattern */	
	char *subject;	/* subject string */
	int subject_length;	/* length of subject string */

	pcre *re;	/* compiled pattern */
	pcre_extra *sd;	/* pattern study information */
	int rc;	/* result of pattern matching */

	const char *error;	/* error information */
	int erroffset;	
	int ovector[OVECCOUNT];
	int i;

	/* We require two arguments: pattern, subject string. */
	if (argc != 3) {
		printf("%s [regex pattern] [subject string] \n", argv[0]);
  		return EXIT_FAILURE;
  	}

	pattern = argv[1];
	subject = argv[2];
	subject_length = (int)strlen(subject);

	/* Compile the regex */
	re = pcre_compile(
			pattern,	/* the pattern */
			0,			/* default options */
  			&error,		/* for error message */
  			&erroffset,	/* for error offset */
  			NULL);		/* use default character tables */

	/* Compilation failed: print the error message and exit */
	if (!re) {
		printf("ERROR: Could not compile '%s' at offset %d: %s\n", pattern, erroffset, error);
  		return EXIT_FAILURE;
  	}

	/* Optimize the regex */
	sd = pcre_study(
			re,		/* compiled regex */
			0,		/* no options */
			&error	/* for error message */
			);	

	/* pcre_study() returns NULL for both errors and when it can not optimize the regex.  
	 * The last argument is how one checks for errors (it is NULL if everything works, 
	 * and points to an error string otherwise. */
	if (error) {
		printf("ERROR: Could not study '%s': %s\n", pattern, error);
		/* Release memory for compiled regex */
		pcre_free(re);
		return EXIT_FAILURE;
	}
	 
	/* Match the regex */
	rc = pcre_exec(
  			re,                   /* compiled regex pattern */
  			sd,                   /* extra data that we have studied */
  			subject,              /* the subject string */
  			subject_length,       /* the length of the subject */
  			0,                    /* start at offset 0 in the subject */
  			PCRE_NOTEMPTY,		  /* empty string is not a valid match */
  			ovector,              /* output vector for substring information */
  			OVECCOUNT);           /* number of elements in the output vector */

	/* Matching failed: handle error cases */
	if (rc < 0) {
  		switch(rc) {
			case PCRE_ERROR_NOMATCH: 
				printf("String did not match the pattern\n");        
				break;
			case PCRE_ERROR_NULL: 
				printf("Something was null\n");                      
				break;
			case PCRE_ERROR_BADOPTION: 
				printf("A bad option was passed\n");                 
				break;
			case PCRE_ERROR_BADMAGIC: 
				printf("Magic number bad (compiled re corrupt?)\n"); 
				break;
			case PCRE_ERROR_UNKNOWN_NODE: 
				printf("Something kooky in the compiled re\n");      
				break;
			case PCRE_ERROR_NOMEMORY: 
				printf("Ran out of memory\n");                       
				break;
			case PCRE_ERROR_MATCHLIMIT:
				printf("Too many matches\n");
				break;
			case PCRE_ERROR_RECURSIONLIMIT:
				printf("Too large recursion degree\n");
				break;
			default: 
				printf("Unknown error\n");                           
				break;			  
		}

		/* Release memory used for compiled regex and study */
		pcre_free(re);
		pcre_free_study(sd);     
  		return EXIT_FAILURE;
	}
	    
	printf("rc = %d\n", rc);
	
	/* Match succeded */
	printf("Match succeeded at offset %d\n", ovector[0]);

	/**************************************************************************
	 * We have found the first match within the subject string. If the output *
	 * vector wasn't big enough, say so. Then output any substrings that were *	
	 * captured.                                                              *
	 **************************************************************************/
	
	/* The output vector wasn't big enough */
	if (rc == 0) {
  		rc = OVECCOUNT / 3;
  		printf("ovector only has room for %d captured substrings\n", rc - 1);
  	}

	/* Show substrings stored in the output vector by number. */
	for (i = 0; i < rc; i++) {
  		char *substring_start = subject + ovector[2 * i];
  		int substring_length = ovector[2 * i + 1] - ovector[2 * i];
  		printf("%d: %.*s\n", i, substring_length, substring_start);
  	}

	pcre_free(re); 
	pcre_free_study(sd);         
	return EXIT_SUCCESS;
}