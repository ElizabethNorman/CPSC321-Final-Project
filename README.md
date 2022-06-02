# CPSC321-Final-Project

This is the fake file system created for my Operating Systems class. I was really proud of what I did with this assignment. 
The code could use some refactoring in places, and I was going to fix it before uploading, but decided to keep it.
This is what I did in a week with very little C experience and a lot of end of semester pressure weighing down on me.

The following is copied verbatim from the assignment submission README

### Instructions:


- I still haven’t made a makefile. Open up the terminal on Ubuntu. Type in: gcc simpleShell.c DiskSim.c -o shell
That should give you the executable needed :) Then of course, run ./shell to get you to the program.


- The program will run on its own at first, creating a partition, a root directory and two children root directories, that will also contain files.
Then the user may do as they like.


- Available commands: writeFile, deleteFile, makeFile, createDirectory, readFile. You may do things in any order as you wish,
but I have implemented many checks to ensure seamless operation of my program.


- You will type in the command desired first and then you will enter arguments. Start with makeFile with a directory I made, or create your own directory


- There are many, many print statements. You will be shown detailed messages showing you the inner workings of the system as you run the program. 
Program will inform you as to where files and directories have been made and their IDs so you can create and write more files and directories!


- Keep some pen and paper near by, maybe, as you will need to inform the program where you want to utilize the commands on!

### The Bugs (that I can find):

- Sometimes in the directory, the parent ID is not affixing itself as we attach the “pointers” to the directory. I don’t know why. This causes issues 
documented later.
- Valgrind is unhappy about “conditional jump or moves depending on uninitialized values”. I see where the issue is, but I feel confident that my 
checks made prior to these complaints see to it that there isn’t much of an issue here. Also I don’t really know what else to do about it.
- readFile does not like printing out big strings or sometimes it doesn’t print at all (this is due to the directory bug mentioned above). 
I do not wish to focus on that because readFile was not part of the assignment but I am letting you know it gets weird.
