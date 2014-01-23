/*
 * Ariko, a binary text relative search tool for the UNIX world.
 *
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
 */
#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int* relSearch8Bit(char* filePath, char* strToSearch)
{
    int* result = calloc(0, sizeof(int)); 
    int* temp;
    int fd, fo, so, co, cid; //file descriptor, file offset, string off-
                             //set, chunk offset, chunk id
    int strLength = strlen(strToSearch);
    int lastCLen, newCLen;
    int* cLen = NULL;
    int upperA, lowerA;
    int* a = NULL;
    int nbOfResults = 0;
    unsigned char lastChunk[1024], newChunk[1024]; //extracted chunks
    unsigned char *chunk; //pointer to current chunks
    unsigned char charIndex[strLength];

    //index search string characters
    for(char x = 0; x < strLength; x++) {
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
    upperA = -1; lowerA = -1; //-1 = unset

    while(true) { //chunks loop
        if(cid == 0) { //a new chunk is required
            co = 0;
            newCLen = read(fd, newChunk, 1024);
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
                    co += 1025;
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
            memcpy(lastChunk, newChunk, strlen(newChunk));
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
    unsigned char lastChunk[1024], newChunk[1024]; //extracted chunks
    unsigned char *chunk; //pointer to current chunks
    unsigned char charIndex[strLength];

    //index search string characters
    for(char x = 0; x < strLength; x++) {
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
    upperA = -1; lowerA = -1; //-1 = unset

    while(true) { //chunks loop
        if(cid == 0) { //a new chunk is required
            newCLen = read(fd, &newChunk[1], 1023);
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
                    co += 1025;
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
            memcpy(lastChunk, newChunk, strlen(newChunk));
            lastCLen = newCLen;
            lastByte = &chunk[1023];
            co -= 1023;
        }
        else if(cid == 1) { //go back to newChunk 
            cid = 2;
            co -= 1023;
            chunk = newChunk;
            cLen = &newCLen;
        } else if(cid == 2) { //this time we finaly need a fresh chunk
            cid = 0; 
            co -= 1023;
        }

    } //end of chunks loop

    result[0] = nbOfResults;
    
    return result;
}

int * relSearch(char* filePath, char* strToSearch, int bits, bool bigEndian)
{
    if(bits == 8) {
        return relSearch8Bit(filePath, strToSearch);
    } else {
        return relSearch16Bit(filePath, strToSearch, bigEndian);
    }
}

int main(int argc, char* argv[])
{
    //argument number
    if(argc < 4) { 
        printf("Error: not enough arguments to run.\n");
        exit(0);
    }
    
    //first argument: file to read in
    if(access(argv[1], F_OK) == -1 ) {
        printf("Can't read specified file.\n");
        exit(0);
    }

    //second argument : string to search for
    int uppers = 0;
    int lowers = 0;
    for(int x = 0; x < strlen(argv[2]); x++) {
        if(isalpha(argv[2][x])) {
            if(isupper(argv[2][x])) uppers++;
            else lowers++;
        }
    }
    if(lowers < 2 && uppers < 2) {
        printf("Error: string to search is badly formed.\n");
        exit(0);
    }

    //third argument : character bits
    int bits = strtol(argv[3],NULL,10);
    if(bits != 8 && bits != 16) {
        printf("Error: third arguments must be 8 or 16.\n");
        exit(0);
    }

    //fourth argument : byte endianness
    bool bigEndian;
    if(argc == 4 || strcmp(argv[4],"little") == 0) {
        bigEndian = false;
    }
    else {
        bigEndian = true;
    }

    //ok, fetch search result
    int* result = relSearch(argv[1], argv[2], strtol(argv[3],NULL,10),bigEndian);
    
    //print result in json format
    printf("[");
    for(int x = 0; x < result[0]*3; x+=3) {
        printf("{'hexOffset':'0x%x', 'decOffset':%d, ", result[x+1], result[x+1]);
        
        if(result[x+2] >= 0) {
            printf("'A':%d", result[x+2]);
            if(result[x+3] >= 0) {
                printf(", 'a':%d", result[x+3]);
            }
        } else if(result[x+3] >= 0) {
            printf("'a':%d", result[x+3]);
        }
        
        printf("}");
        if(x != (result[0]-1)*3) {
            printf("\n");
        }
    }
    printf("]\n");
    
    //free memory in heap from dynamic results
    free(result);
    
    //exit program
    return 0;
}

