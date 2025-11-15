json_parser
===========

Small utility that scans a folder for `.json` files and prints two fields: `playlistName` and `shareCode`.
Build:

https://www.msys2.org (follow these to compile via g++)

you might have to add msys2 to your system variables if you get an g++ error. -- You can google and figure this out your self.

g++ (MinGW / WSL):

```powershell
g++ -std=c++17 -O2 -Wall -o parsejson.exe json_parser.cpp
```


Usage:

                                • Basic usage

        

                                



                                • "advanced" usage:
                                
                                                               • COMMANDS/FLAGS:
                                                               
                                •  -d or --discription will include the discription.
                                • -a or --author will include the user of the person who made the playlist and their steam ID.
                                • use -q or --quiet to skip the statistic summary at the end
                                • use -o or --output flag to specify a custom output directory 
                                • use -q or --quiet to remove the statistics screen.



                                                                         • examples:

                                  •  .\json_parser.exe (current directory without author and discription info)
                                  •  .\json_parser.exe -a (current directory with author info)
                                  •  .\json_parser.exe --author C:\Program Files (x86)\Steam\steamapps\common\FPSAimTrainer\FPSAimTrainer\Saved\SaveGames\Playlists (specified directory with author info)
                                  •  .\json_parser.exe          ^ (specified directory without author info)
                                  •  .\json_parser.exe -n myfile.txt (output to parent_dir\myfile.txt)
                                  •  .\json_parser.exe -o C:\output -n custom.txt (output to C:\output\custom.txt)
                                  •  .\json_parser.exe -o C:\output (output to C:\output\results.txt)
                          •  .\parsejson.exe 'C:\Program Files (x86)\Steam\steamapps\common\FPSAimTrainer\FPSAimTrainer\Saved\SaveGames\Playlists' -d -a -n mreow.txt -o C:\Users\Violet\Desktop\output
                              ^               ^ path to where your kovaaks local files are and then playlists                                       ^   ^                ^ changes where the results file is put
                              ^                                                                                                                     ^  ^  ^ changes the name of the results file
                              ^                                                                                                                     ^  ^
                              ^ compiled EXE file name                                                                                              ^  ^ Outputs author/creators steam username + ID
                                                                                                                                                    ^ adds the description of the playlist






