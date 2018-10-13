# UNIX-OS-Apps
A collection of system calls for the OS Labotory done at the University. Each project is the exam's solution.
<br>

<b>alpha-stats</b><br>
<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/alpha-stats-msg.png" align="right" width="150" alt="">
This program will create as many children's process as the number of files on the command line.
The process will communicate each other by a messages' queue.
Each child T-n will read the F-n file, mapping the contents of files in its own address space.
The child will make a count of the occurrence of the 26 letters of the alphabet (uppercase and lowercase included),
displaying it on video on a single line. This statistic will be sent to the parent P process through a single message.
The father, received all the statistics, will display a video a general statistics given by the sum of the aforesaid and, in
in the case of a maximum without ex-aequo, he will decide the letter of the most used alphabet's character.

<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/asm-u.png" width="700" alt="">

_______________________________________________________________________________________________________________________________

<b>alpha-stats-sem</b><br>
This version uses a semaphore and shared memory. 
Each child will read the contents of the assigned file using the mechanism of
mapping of files to memory: for each alphabetic character (lowercase and uppercase)
encountered will increase the relative counter in the segment by one unit shared memory: 
at each increment it will have to wait for a random pause of at most one second before being 
able to resume the scan.
When all children have finished processing their files, the father will calculate and display 
the frequencies on the screen percentages of occurrence of the letters.

<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/ass-u.png" width="700" alt="">
____________________________________________________________________________________________________________________________
