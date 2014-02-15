Ariko
=====
A relative search tool for the UNIX world.


__Compilation__ 
```bash
gcc ariko.c relsearch.c -std=c99 -o ariko
```
Windows users should use [Cygwin](http://www.cygwin.com "Cygwin")  to compile (not tested).

__Usage__
```bash
ariko [-b <NB>] [-e <END>] [-x <ID>] <FILEPATH> <SEARCHSTRING>

Execute a relative search to find character encoding.
  
  -b      bits per character (8 or 16)
  -e      endianness: "little" OR "big"
  -x      id of result to export to character map
  --help  display this message
```
