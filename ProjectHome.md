This little tool is meant for people working XIP images of Windows Mobile/Windows CE ROM images.
It searches for imageinfo.bin files and creates imageinfo.txt files based on them. For that task it needs the info about "physics first" value, which can be given in a commandline or it can be taken from romhdr.bin file (if so then the application creates ROMHDR.txt file, too).

Usage:

iitp

iitp -p physfirst

Value of physfirst is expected to be in hex. You can type it both as 0x1234abcd or 1234abcd.
If physfirst is not given, it will search for the romhdr.bin file and decode it into ROMHDR.txt

WARNING:
As of now, it creates proper info about XIP modules (imgfs modules are in todo list) - with the exception of nk.exe (kernel)! It's in the tot, too :)


Thanks go to globalbus for the help and to c\_shekhar for pushing me to make this little tool :=)