Ariko
=====
A relative search tool written in C, compatible with the UNIX world.


__Compilation__ 
```bash
gcc ariko.c relsearch.c -std=c99 -o ariko
```

__Usage__
```bash
ariko [-b <NB>] [-e <END>] [-x <ID>] <FILEPATH> <SEARCHSTRING>

Execute a relative search to find character encoding.
  
  -b      bits per character (8 or 16)
  -e      endianness: "little" OR "big"
  -x      id of result to export to character map
  --help  display this message
```
