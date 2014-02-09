/* Ariko, a relative search tool for the UNIX world.
 * Copyright (C) 2013 Frederic Beaudet (fredericbeaudet@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Repository: https://github.com/fbeaudet/ariko.git
 */

#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "relsearch.h"


int main(int argc, char* argv[])
{
    char* helpString = "\
Usage: ariko [-b <NB>] [-e <END>] [-x <ID>] <FILEPATH> <SEARCHSTRING>\n\
Execute a relative search to find character encoding.\n\n\
  -b,     bits per character (8 or 16)\n\
  -e,     endianness: \"little\" OR \"big\"\n\
  -x,     id of result to export to character map\n\
  --help  display this message\n";
    int* result = NULL; //search result
    int bits = 8; //bits per character: 8 or 16
    int expId = -1; //charmap export result id
    bool bigEndian = true; //endianness

    //long options (--something)
    static struct option longOptions[] = {
        {"help",       no_argument,       0, 'h'},
    };

    //options
    int opt;
    int optindex = 0;
    while((opt = getopt_long(argc, argv, "b:e:x:",longOptions,&optindex)) != -1) {
        switch(opt) {
            case 'b':
                //control -b option
                bits = strtol(optarg,NULL,10);
                if(bits != 8 && bits != 16) {
                    printf("Error: -b option takes only 8 or 16.\n");
                    return 1;
                }
                break;
            case 'e':
                //control -e option
                if(argc == 4 || strcmp(optarg,"big") == 0) {
                    bigEndian = false;
                }
                else {
                    bigEndian = true;
                }
                break;
            case 'h':
                printf("%s", helpString);
                return 0;
            case 'x':
                expId = strtol(optarg,NULL,10);
                break;
            case '?':
                return 1;
            default:
                abort();
        }
    }

    //control number of required arguments
    if(argc-optindex != 3) {
        printf("Error: wrong arguments count. See --help.\n");
        return 1;
    }

    //first required argument: file to read in
    if(access(argv[optindex+1], F_OK) == -1 ) {
        printf("Can't read specified file.\n");
        return 1;
    }

    //second required argument : string to search for
    int uppers = 0; int lowers = 0;
    for(int x = 0; x < strlen(argv[optindex+2]); x++) {
        if(isalpha(argv[optindex+2][x])) {
            if(isupper(argv[optindex+2][x])) {
                uppers++;
            } else {
                lowers++;
            }
        }
    }
    if(lowers < 2 && uppers < 2) {
        printf("Error: string to search is badly formed.\n");
        return 1;
    }

    //ok, execute the relative search
    result = relSearch(argv[optindex+1], argv[optindex+2], bits, bigEndian);

    //output result(s)

    if(result[0] == 0) { // no results
        printf("No results found.\n");
        free(result);
        return 1;
    }
    if(expId != -1) { //-x option specified

        //result with specified id can't be exported to character map
        if(expId > result[0]-1) {
            printf("Error: can't export character map, id %d not found.\n", expId);
            printf("-> %d result(s) found. ", result[0]);
            free(result);
            return 1;
        }

        //export character map of specified result
        char temp[] = "x";
        if(result[expId+2]!= -1) {
            for(int i = 0; i < 26; i++) {
                temp[0] = 65+i;
                printf("%x=%s\n", result[expId+2]+i, temp);
            }
        }
        if(result[expId+3] != -1) {
            for(int i = 0; i < 26; i++) {
                temp[0] = 97+i;
                printf("%x=%s\n", result[expId+3]+i, temp);
            }
        }
        free(result);
        return 0;
    }

    //output all results in json format
    printf("[");
    for(int x = 0; x < result[0]*3; x+=3) {
        printf("{'id':%d,'address':'0x%x', ", x/3, result[x+1]);
        if(result[x+2] >= 0) {
            printf("'A':'0x%x'", result[x+2]);
            if(result[x+3] >= 0) {
                printf(", 'a':'0x%x'", result[x+3]);
            }
        } else if(result[x+3] >= 0) {
            printf("'a':'0x%x'", result[x+3]);
        }
        printf("}");
        if(x != (result[0]-1)*3) {
            printf(",\n");
        }
    }
    printf("]\n");

    free(result);
    return 0;
}
