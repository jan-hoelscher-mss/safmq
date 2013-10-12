/*
 Copyright 2005 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software distributed
	under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied. See the License for the
	specific language governing permissions and limitations under the License.




This software implements a platform independent Store and Forward Message Queue.
*/

/*****
_getopt.c  provides compatibility between unix and windows for the unix standard functions getopt and getopt_long
*/

#include <string.h>
#include <stdio.h>
#include "_getopt.h"

char	*optarg;
int		optind = 1, opterr = 0, optopt = 0;


int getopt(int argc, char * const argv[], const char *optstring)
{
	static int cur_char = 0;
	int optfound = -1;

	if (optind == 0)
		optind = 1;

	if (optind < argc) {
		int paramlen;

		paramlen = strlen(argv[optind]);
		if (argv[optind][0] == '-') {
			const char* popt;
	
			cur_char++;
			if (cur_char >= paramlen) {
				/* Give one second chance */
				optind ++;
				if (optind >= argc || argv[optind][0] != '-')
					return -1;
				cur_char = 1;
				paramlen = strlen(argv[optind]);
			}
			

			popt = optstring;
			if (*popt == ':')
				popt ++;

			for(; optfound < 0 && *popt; popt++) {
				if (argv[optind][cur_char] == *popt) {
					if (*(popt+1) == ':') {
						if (paramlen > 2) {
							optarg=argv[optind] + 2;
						} else if ( optind+1 < argc) {
							/* advance the opt indicator for the option parameter as well */
							optarg=argv[optind+1];
							optind += 2;
							cur_char = 0;
						} else {
							if (opterr)
								fprintf(stderr,"getopt: missing parameter for %c\n", *popt);
							return optstring[0]==':' ? ':' : '?';
						}
					}
					optfound = *popt;
				}
			}
			if (!optfound) {
				if (opterr)
					fprintf(stderr,"getopt: unknown parameter %c\n", argv[optind][cur_char]);
				optind++;
				cur_char = 0;
				return '?';
			} 
			return optfound;
		}
	}
	return optfound;
}

int getopt_long(int argc, char * const argv[],
          const char *optstring,
          const struct option *longopts, int *longindex)
{
	if (optind < argc) {
		int optidx;
		int	paramlen = strlen(argv[optind]);
		int	optlen;
		int	optfound = 0;

		// Attempt to match a long option
		if (argv[optind][0] == '-' && argv[optind][1] == '-' ) {
			for(optidx=0; !optfound && longopts[optidx].name; optidx++) {
				optlen = strlen(longopts[optidx].name);

				if ( (paramlen-2 == optlen && !strcmp(argv[optind]+2,longopts[optidx].name)) ||
						(longopts[optidx].has_arg && paramlen-3>=optlen 
							&& argv[optind][optlen+2] == '=' && !strncmp(argv[optind]+2,longopts[optidx].name, optlen)) ) {

					if (longopts[optidx].has_arg) {
						if (paramlen-2 == optlen) {
							if (optind+1 < argc) {
								optarg = argv[++optind];
							} else {
								if (opterr) {
									fprintf(stderr,"getopt_long: missing parameter for %s\n", argv[optind]);
								}
								return optstring && *optstring == ':' ? ':' : '?'; 
							}
						} else {
							optarg=argv[optind]+optlen+3;
						}
					}

					if (longopts[optidx].flag)
						*(longopts[optidx].flag) = longopts[optidx].val;
					if (longindex)
						*longindex = optidx;
					optfound = 1;
					optind++;

					return longopts[optidx].flag ? 0 : longopts[optidx].val;
				}
			}
			if (opterr)
				fprintf(stderr,"getopt_long: unknown parameter %s\n", argv[optind]);
			optind++;
			return '?';
		}

		if (optstring && *optstring)
			return getopt(argc,argv,optstring);
	}

	return -1;
}
