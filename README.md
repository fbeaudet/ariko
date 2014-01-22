Ariko
=====

A binary text relative search tool for the UNIX world.

__Compilation__ 
```bash
gcc ariko.c -std=c99 -o "ariko"
```

__Usage__
```bash
ariko "filepath.rom" "words to search" 8/16 [little/big]
```

*Where:*
* 8/16 is the number of bits per character. 
* Little/big is the endianness, it's optional and concerns only 16-bit characters. (default: big)
