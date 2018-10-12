# UNIX-OS-Apps
A collection of system calls for the OS Labotory done at the University. Each project is the exam's solution.

<b>alpha-stats</b>
This program will create as many children's process as the number of files on the command line.
The process will communicate each other by a messages' queue.
Each child T-n will read the F-n file, mapping the contents of files in its own address space.
The child will make a count of the occurrence of the 26 letters of the alphabet (uppercase and lowercase included),
displaying it on video on a single line. This statistic will be sent to the parent P process through a single message.
The father, received all the statistics, will display a video a general statistics given by the sum of the aforesaid and, in
in the case of a maximum without ex-aequo, he will decide the letter of the most used alphabet's character.
<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/alpha-stats-msg.png" align="right" width="200" alt="">


