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

_____________________________________________________________________________________________________________________________

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

<b>another-wc</b><br>
This program will start a child process when it starts. 
This process will communicate with the parent through a large shared memory segment with more than 2 bytes. 
For a correct communication the use of semaphores will be necessary.
The child process will read the contents of the text file indicated as the only parameter (or from the standard input in its absence) and pass its contents to the parent process byte-by-byte. The father will have to process the content received by doing an analysis similar to that performed by the UNIX WC command: counting the number of characters, words and lines. Consider as space delimiters the spaces / tabs and the main punctuation marks (.,;:!).

<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/awc-u.png" width="700" alt="">

____________________________________________________________________________________________________________________________

<b>cpu-monitor</b><br>
<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/cpu-monitor.png" align="right" width="150" alt="">
The program, through the use of the special text file <b>/proc/stat</b> of the file-system virtual <b>/proc/</b>,
collects samples on the use of the CPU compartment of the system on which it is executed. 
A number of samples equal to those that are possibly collected is collected specified on the 
command line (30 by default) and then displayed on screen via a simple graph and in percentage terms.
The first line of the special text file <b>/proc/stat</b> has the following format:
cpu 10403824 223264 2676976 12200724 275004 0 21309 0 0 838
where the numbers are cumulative units of time units (called jiffies) expenses from the various
core, respectively, in:
• <b>user</b>: normal processes in user mode;
• <b>nice</b>: processes, under nice, in user mode;
• <b>system</b>: processes in kernel mode;
• <b>idle</b>: moments of inactivity;
• <b>iowait</b>: pending completion of I / O;
• <b>irq</b>: in the management of hardware interrupts;
• <b>softirq</b>: in the management of software interrupts.
For simplicity, we will consider only the time used in mode user (user), kernel mode (system) and idle mode (idle).
The parent F program, when it starts, will create three child processes: Sampler, Analyzer and Plotter. Children will communicate with each other only through a message queue created by the parent. Are expected two types of messages:
• raw msg: they carry 3 integers that represent cumulative data (user,system,idle) taken directly from the /proc/stat file;
• delta messages: carry 2 numbers in comma mobile that represent, respectively, the percentages of use in user mode and in kernel mode between a sample and the next.
The roles of the three child processes will be as follows:
• Sampler process, reading from the file /proc/stat, samples the whole samples (user, system, idle) required and sends them with separate raw messages to the Analyzer child; Each sample is taken at a distance of one second from each other;
• Analyzer process considers the differences between the various raw samples received and extrapolates the percentages of use of the CPU compartment in user mode and kernel mode in the last second; this information comes gradually sent to the son Plotter through delta messages;
• Plotter process displays the received delta type data on the standard output (one row per pair) with a simple chart on 60 columns and in terms percentages (see example below).


<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/cpum-u.png" width="700" alt="">

____________________________________________________________________________________________________________________________
