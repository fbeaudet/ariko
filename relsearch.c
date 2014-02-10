/* This file is part of Ariko.
 *
 * Ariko is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ariko is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Ariko.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "relsearch.h"

#define CHUNK_SIZE 1024

/*Search for 8-bit characters and return an array of ints where first position
is the number of results, then results follow with 3 ints groups. In these groups,
first position is the memory address, second is 'A' value and third is 'a' value.
A/a values can be NULL if there is only 1 character case in strToSearch.*/
int* relSearch8Bit(char* filePath, char* strToSearch)
{
    int* result = calloc(0, sizeof(int));
    int* temp = NULL;
    int fd, fo, so, co, cid; //file descriptor, file offset, string off-
                             //set, chunk offset, chunk id
    int strLength = strlen(strToSearch);
    int lastCLen, newCLen;
    int* cLen = NULL;
    int upperA, lowerA;
    int* a = NULL;
    int nbOfResults = 0;
    unsigned char lastChunk[CHUNK_SIZE];
    unsigned char newChunk[CHUNK_SIZE];
    unsigned char *chunk = NULL; //pointer to current chunk
    unsigned char charIndex[strLength];

    //index search string characters
    for(unsigned char x = 0; x < strLength; x++) {
        if(!isalpha(strToSearch[x])) {
            charIndex[x] = 0xFF; //0xFF = free character (*)
        } else {
            if(isupper(strToSearch[x])) {
                charIndex[x] = strToSearch[x] - 65;
            } else {
                charIndex[x] = strToSearch[x] - 97;
            }
        }
    }

    //begin the search
    fd = open(filePath, O_RDONLY);
    cid = 0;
    fo = 1;
    so = 0;
    upperA = -1; lowerA = -1; //-1 = unset

    while(true) { //chunks loops

        if(cid == 0) { //a new chunk is required
            co = 0;
            newCLen = read(fd, newChunk, CHUNK_SIZE);
            if(newCLen == 0) break;
            cLen = &newCLen;
            chunk = newChunk;
        }

        while(co < *cLen) { //bytes loop

            //get a/A value to use.
            if(charIndex[so] != 0xFF) {
                if(isupper(strToSearch[so])) {
                    if(upperA == -1) {
                        upperA = *(chunk+co) - charIndex[so];
                    }
                    a = &upperA;
                } else {
                    if(lowerA == -1) {
                        lowerA = *(chunk+co) - charIndex[so];
                    }
                    a = &lowerA;
                }
            }

            //compare byte with character index
            if(charIndex[so] == 0xFF ||
            ((*a >= 0) && *(chunk+co) - *a == charIndex[so])) {
                //character match !
                so += 1;
                if(so == strLength) {
                    nbOfResults += 1;
                    temp = realloc(result, (nbOfResults*3+1)*sizeof(int));
                    if(temp == NULL) {
                        printf("Failed to allocate memory for search results.\n");
                        exit(0);
                    }
                    result[(nbOfResults-1)*3+1] = fo-so;
                    result[(nbOfResults-1)*3+2] = upperA;
                    result[(nbOfResults-1)*3+3] = lowerA;
                    so = 0;
                    upperA = -1; lowerA = -1;
                }

            } else {
                //wrong character found !
                co -= so;
                fo -= so;
                so = 0;
                upperA = -1; lowerA = -1;
                if(co < -1) { //we have to go back to last chunk
                    fo += 1;
                    co += CHUNK_SIZE+1;
                    cid = -1;
                    chunk = lastChunk;
                    cLen = &lastCLen;
                    break;
                }
            }
            fo += 1; co += 1;

        } //end of bytes loop

        //operations before next chunk
        if(cid == -1) { //we have to go back to lastChunk
            cid = 1;
        } else if(cid == 0) { //save newChunk in lastChunk
            memcpy(lastChunk, newChunk, strlen((const char*)newChunk));
            lastCLen = newCLen;
        }
        else if(cid == 1) { //go back to newChunk
            cid = 2;
            co = 0;
            chunk = newChunk;
            cLen = &newCLen;
        } else if(cid == 2) {//this time we need a fresh chunk
            cid = 0;
        }

    } //end of chunks loop

    result[0] = nbOfResults;

    return result;
}

/*Search for 16-bit characters and return an array of ints where first position
is the number of results, then results follow with 3 ints groups. In these groups,
first position is the memory address, second is 'A' value and third is 'a' value.
A/a values can be NULL if there is only 1 character case in strToSearch.*/
int* relSearch16Bit(char* filePath, char* strToSearch, bool bigEndian)
{
    int* result = calloc(0, sizeof(int));
    int* temp;
    int fd, fo, so, co, cid; //file descriptor, file offset, string off-
                             //set, chunk offset, chunk id
    int value;
    unsigned char *lastByte = NULL;
    int strLength = strlen(strToSearch);
    int lastCLen, newCLen;
    int* cLen = NULL;
    int upperA, lowerA;
    int* a = NULL;
    int nbOfResults = 0;
    unsigned char lastChunk[CHUNK_SIZE], newChunk[CHUNK_SIZE];
    unsigned char *chunk = NULL; //pointer to current chunk
    unsigned char charIndex[strLength];

    //index search string characters
    for(unsigned char x = 0; x < strLength; x++) {
        if(!isalpha(strToSearch[x])) {
            charIndex[x] = 0xFF; //0xFF = free character (*)
        } else {
            if(isupper(strToSearch[x])) {
                charIndex[x] = strToSearch[x] - 65;
            } else {
                charIndex[x] = strToSearch[x] - 97;
            }
        }
    }

    //begin the search
    fd = open(filePath, O_RDONLY);
    cid = 0;
    fo = 1;
    co = 1;
    so = 0;
    upperA = -1; lowerA = -1; //-1 = unset

    while(true) { //chunks loop
        if(cid == 0) { //a new chunk is required
            newCLen = read(fd, &newChunk[1], CHUNK_SIZE-1);
            if(newCLen == 0) break;
            if(lastByte != NULL) {
                newChunk[0] = *lastByte;
                lastByte = NULL;
            }
            newCLen += 1;
            cLen = &newCLen;
            chunk = newChunk;
        }

        while(co < *cLen-1) { //bytes loop
            if(charIndex[so] != 0xFF) {

                //get byte value
                if(bigEndian) {
                    value = (*(chunk+co) << 8 ) | (*(chunk+co+1) & 0xff);
                }  else {
                    value = (*(chunk+co+1) << 8 ) | (*(chunk+co) & 0xff);
                }

                //get a/A value to use.
                if(isupper(strToSearch[so])) { //point to value of relative A/a
                    if(upperA == -1) {
                        upperA = value - charIndex[so];
                    }
                    a = &upperA;
                } else {
                    if(lowerA == -1) {
                        lowerA = value - charIndex[so];
                    }
                    a = &lowerA;
                }
            }

            //compare bytes character with character index
            if(charIndex[so] == 0xFF ||
            ((*a >= 0) && value - *a == charIndex[so])) {
                //character match !
                so += 1;
                if(so == strLength) {
                    nbOfResults += 1;
                    temp = realloc(result, (nbOfResults*3+1)*sizeof(int));
                    if(temp == NULL) {
                        printf("Failed to allocate memory for search results.\n");
                        exit(0);
                    }
                    result[(nbOfResults-1)*3+1] = fo-(so*2-1);
                    result[(nbOfResults-1)*3+2] = upperA;
                    result[(nbOfResults-1)*3+3] = lowerA;
                    so = 0;
                    upperA = -1; lowerA = -1;
                }

            } else {
            //wrong character found !
                co -= so*2+1;
                fo -= so*2+1;
                so = 0;
                upperA = -1; lowerA = -1;
                if(co < -2) { //we have to go back to last chunk
                    fo += 2;
                    co += CHUNK_SIZE+1;
                    cid = -1;
                    chunk = lastChunk;
                    cLen = &lastCLen;
                    break;
                }
            }
            fo += 2; co += 2;

        } //end of bytes loop

        //operations before next chunk
        if(cid == -1) { //we have to go back to lastChunk
            cid = 1;
        } else if(cid == 0) { //save newChunk in lastChunk
            memcpy(lastChunk, newChunk, strlen((const char*)newChunk));
            lastCLen = newCLen;
            lastByte = &chunk[CHUNK_SIZE-1];
            co -= CHUNK_SIZE-1;
        }
        else if(cid == 1) { //go back to newChunk
            cid = 2;
            co -= CHUNK_SIZE-1;
            chunk = newChunk;
            cLen = &newCLen;
        } else if(cid == 2) { //this time we finaly need a fresh chunk
            cid = 0;
            co -= CHUNK_SIZE-1;
        }

    } //end of chunks loop

    result[0] = nbOfResults;

    return result;
}

/*A generic 8/16-bit function that calls the right sub-function.*/
int* relSearch(char* filePath, char* strToSearch, int bits, bool bigEndian)
{
    if(bits == 8) {
        return relSearch8Bit(filePath, strToSearch);
    } else {
        return relSearch16Bit(filePath, strToSearch, bigEndian);
    }
}
