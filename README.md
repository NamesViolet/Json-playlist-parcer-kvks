json_parser
===========

Small utility that scans a folder for `.json` files and prints two fields: `playlistName` and `shareCode`.
Build:

https://www.msys2.org (follow these to compile via g++)
g++ (MinGW / WSL):

```powershell
g++ -std=c++17 -O2 -Wall -o parsejson.exe json_parser.cpp
```


Usage: