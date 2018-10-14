# UNIX-OS-Apps
A collection of system calls for the OS Laboratory done at the University. Each project is the exam's solution.
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
<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/cpum.png" align="right" width="200" alt="">
The program, through the use of the special text file <b>/proc/stat</b> of the file-system virtual <b>/proc/</b>,
collects samples on the use of the CPU compartment of the system on which it is executed. 
A number of samples equal to those that are possibly collected is collected specified on the 
command line (30 by default) and then displayed on screen via a simple graph and in percentage terms.
The first line of the special text file <b>/proc/stat</b> has the following format:<br>
<b>cpu 10403824 223264 2676976 12200724 275004 0 21309 0 0 838</b><br>
where the numbers are cumulative units of time units (called jiffies) expenses from the various
core, respectively, in:<br>
• <b>user</b>: normal processes in user mode;<br>
• <b>nice</b>: processes, under nice, in user mode;<br>
• <b>system</b>: processes in kernel mode;<br>
• <b>idle</b>: moments of inactivity;<br>
• <b>iowait</b>: pending completion of I/O;<br>
• <b>irq</b>: in the management of hardware interrupts;<br>
• <b>softirq</b>: in the management of software interrupts.<br>
For simplicity, we will consider only the time used in mode user (user), kernel mode (system) and idle mode (idle).
The parent F program, when it starts, will create three child processes: Sampler, Analyzer and Plotter. Children will communicate with each other only through a message queue created by the parent. Are expected two types of messages:<br>
• <b>raw msg</b>: they carry 3 integers that represent cumulative data (user,system,idle) taken directly from the /proc/stat file;<br>
• <b>delta messages</b>: carry 2 numbers in comma mobile that represent, respectively, the percentages of use in user mode and in kernel mode between a sample and the next.<br>
The roles of the three child processes will be as follows:<br>
• <b>Sampler</b> process, reading from the file /proc/stat, samples the whole samples (user, system, idle) required and sends them with separate raw messages to the Analyzer child; Each sample is taken at a distance of one second from each other;<br>
• <b>Analyzer</b> process considers the differences between the various raw samples received and extrapolates the percentages of use of the CPU compartment in user mode and kernel mode in the last second; this information comes gradually sent to the son Plotter through delta messages;<br>
• <b>Plotter</b> process displays the received delta type data on the standard output (one row per pair) with a simple chart on 60 columns and in terms percentages (see example below).

<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/cpum-u.png" width="700" alt="">

____________________________________________________________________________________________________________________________

<b>fifo-palindrome-filter</b><br>
<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/fpf.png" align="right" width="200" alt="">
The program must act as a filter to select, among the input words, those that represent a palindroma word in case insensitive (ignoring the differences between upper and lower case). The expected input is a list of words (one per line)
from the specified fle on the command line or from the standard input in its absence. The output result of the selection will be reversed on the standard output. The program at its start will create two processes for R and W. The three
processes will communicate via two named pipes (FIFO and R → P and P → W).<br>
The roles of the three processes will be as follows:<br>
•<b>R</b> will read the list from the indicated file using the mapping of the fle in memory, validate and pass it on to his father P through theirs named pipe;<br>
•<b>P</b> will analyze the content received from R, word by word, discarding those that do not they represent palindromes. The words palindrome will come sent to the W process through their respective named pipe;<br>
•<b>W</b> will write the words received from the father on the standard output. Words that have been output will have to have the "uppercase/lowercase" structure unaltered compared to the input.

<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/fpf-u.png" width="700" alt="">

____________________________________________________________________________________________________________________________

<b>file-shell</b><br>
<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/fls.png" align="right" width="200" alt="">
At the start of this program, a non-predetermined number of processes will be created equal to that of the directories indicated on the command line. These processes will communicate between them with a message queue.
The program will have to offer an interactive shell that will accept the following requests:<br>
● <b>num-files n</b>: displays the number of regular files directly contained in the directory number n (no recursion);<br>
● <b>total-size n</b>: displays the total size (in bytes) of regular files directly contained in the directory number n (no recursion);<br>
● <b>search-char n character-file-name</b>: displays the number of occurrences of the character indicated (one byte) in the specified fle of the directory number n.<br>
The program must respect the following constraints:<br>
● Parent P process:<br>
-will be the only one able to interact with the terminal e to be able to view messages;<br>
-will never have to directly access the fle-system;<br>
-will dialogue with the processes of D-n uniquely using messages;<br>
● Process D-n:<br>
-will not be able to display any message on the terminal;
-will only be able to access your directory number n;
-will use the mapping of the fle in memory to read the contents of the files regular;
-will dialogue with the father only using the messages.

<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/fls-u.png" width="700" alt="">

____________________________________________________________________________________________________________________________

<b>list-dirs</b><br>
The program will essentially list, in some detail, the regular files and the sub-directories encountered when reading the specified folders. The parent process will create as many <b>Reader</b> child processes as the specified directories
on his command line. He will also create two more children: <b>File-Consumer</b> and <b>Dir-Consumer</b>. The
father and children communicate with a shared memory segment and coordinate with one certain number, minimal, of semaphores. The segment will contain a single record relative to a generic object of the file-system.<br>
Each child <b>Reader</b> will scan (non-recursive) the specified directory: only regular files and ordinary directories will be considered (ignoring e ..). The information of a regular file (name and size in bytes) or of a directory (only
name) will be inserted into the shared memory segment: one object at a time. Records relating to regular files will be extracted from the segment by the <b>File-Consumer</b> child; Those relating to directories instead of <b>Dir-Consumer</b>.

<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/ldr-u.png" width="700" alt="">

____________________________________________________________________________________________________________________________

<b>my-du-s</b><br>
The program will simulate the behavior of the du-s command-option: this, for each indicated path, calculates the disk space occupied by files contained therein recursively. The parent process, when it starts, will create a single child <b>Stater</b> process and a group of <b>Scanner</b> processes (a separate process for each root path indicated on the line of command). The father and all the children will communicate only through a segment of shared memory with the help of a certain number of semaphores. Scanner processes will have to act in parallel. Each child Scanner will perform the recursive scan of the root path a he assigned: for every object encountered, different from a directory, he will send his
identifying pathname to the Stater process. The Stater process, for every request received, will determine the disk space occupied by the object using the <b>st_blocks</b> field of the record reported by call stat. This information must be sent to the father which will count total employment, distinct for each root path, reporting to end on its standard-output all totals (like the du -s command).

<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/mds-u.png" width="700" alt="">

____________________________________________________________________________________________________________________________

<b>my-fgrep</b><br>
The program emulates the behavior of the homonymous shell command: reads sequentially the contents of the indicated files and select the lines containing the word specified; with the <b>v</b> option the control is reversed (the rows are selected do NOT contain the word); with the option <b>i</b> the control becomes case-insensitive; every line selected is shown in the standard output using the source file name as a prefix.<br>
The parent process will be responsible for creating the following children:<br>
• <b>Reader</b> for each file indicated on the command line: each of them will read the assigned file and will send its contents, line by line, to the Filterer process; these children must operate in a serial way (for example: the second will be activated only when the first one has finished, and so on ...);<br>
• Single process <b>Filterer</b>: for each line received will apply the selection criterion specified and will send the results rows, complete with prefix, back to the process original Father that will bring them back to the output.
<br>
The Reader children will communicate with the child Filterer through a message queue, created by Father: each message will contain a line of text and an indication of the file source (set maximum file and file path size). Child Filterer will communicate with Father through a pipe.

<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/mfg-u.png" width="700" alt="">

____________________________________________________________________________________________________________________________


<b>numbers-mod</b><br>
<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/nmd.png" align="right" width="150" alt="">
The program must read a series of numbers from the input file specified (one per line) and will have to output them once the function <b>f(x) = x mod n</b>, where n is applied the module specified on the command line. The program at its start will create two child processes <b>Mod</b> and <b>Out</b>. The three processes will communicate only through a memory segment
shared and a semaphore that the father will have to create and destroy. The segment will have to be big a sufficiency and organized to accommodate exactly 10 records of the type [number, type]. The semaphores they will have to be in optimal number to manage the coordination of the processes involved and be used appropriately.<br>
The roles of the three processes will be as follows:<br>
• <b>P</b> process will have to read the file specified by entering, for each number x read, the record [x, 0] in the shared buffer;<br>
• <b>Mod</b> child process will have to extract from the shared buffer, as soon as they are available, records of the type [x, 0] and reinsert the same number of the type [f(x), 1];<br>
• <b>Out</b> process will have to extract from the shared buffer, as soon as they are available, record of the type [y, 1] and output the number y.<br>

<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/nmd-u.png" width="700" alt="">

____________________________________________________________________________________________________________________________

<b>sort-list</b><br>
The program will read the specified text file, containing one word for each line, and output this reordered list (ascending alphanumeric order and case-insensitive). The father process, when it starts, will create 2 children: <b>Sorter</b> and <b>Comparer</b>. The two child processes they will communicate with each other using only one message queue, the father
he will communicate with his son Sorter through a pipe. The Sorter process will have to read the text file, extracting the word list (one for each line) and will have to apply one some sorting algorithms; in the fundamental step of comparing 2 strings, it must always request the help of the Comparer process sending him a message with the 2 strings on and off
getting back, with a second message, the result of the comparison (an integer). Once the sorting algorithm is applied, the Sorter process will have to pass, word for word, the list ordered to the father through the pipe, this will send it on the
Single-output.

<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/slt-u.png" width="700" alt="">

____________________________________________________________________________________________________________________________

<b>words-filter</b><br>
The program will basically read the indicated text file and apply to each line a series of filters indicated on the command line. The final result will be shown on the standard output.<br>
Each filter will have the following structure:<br>
• <b>^word</b>: will look for the occurrences of "word" in each line and transform them using only capital letters;<br>
• <b>_word</b>: will do the same but transforming them using only lowercase letters;<br>
• <b>=word1,word2</b>: will look for every occurrence of "word1" and in each line will replace with "word2" (assume that the two words have the same length but verify it at run-time).<br>
The parent process will first create as many Filter-n child processes as there are filters indicated on the command line. The father and the children will only communicate with a queue of messages created and destroyed by the first one.
The father will read the indicated file line by line; read a line (assuming a length maximum) will forward it to the first child Filter-1 by encapsulating it in a message of size equal to the minimum necessary. Each child <b>Filter-n</b>, received a row, will apply its modification (for all occurrences present) and forward the result to following child Filter-(n+1). The last child will take care of sending the message back to the father who, after sending the result line on the standard output transformations, will move to the next line.

<b>Usage:</b>

<img src="https://github.com/FabrizioBilleciUNICT/UNIX-OS-Apps/blob/master/res/wfl-u.png" width="700" alt="">

____________________________________________________________________________________________________________________________

