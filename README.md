# Network Server 
## Overview
- Multithreaded server (poller) consisting of a master thread and worker threads with a consumer-producer synchronization in a mutually accessed buffer.
- Execution terminates with  Ctrl-C writing the total number of votes for every party in stat.txt.
- Multithreaded clients which create a thread for every registered vote in the input file.
## Bash scripts for debugging 
- create_input that creates a voting file with random names for given poliical parties for a given number of lines
- tallyVotes that reads inputFile from current directory and produces a tallyResultsFile with the votes of each party without counting duplicates.
- processLogFile that reads a poll-log file from current directory and writes the voting results in pollerResultsFile
- Except from duplicates (thread execution makes it uncertain)  we check for differences between poll-stats and outputs of the scripts as the outcomes must be the same
## Implementation Details
- Master thread saves clients' socket descriptors in a fixed size buffer accomodating requests in a LIFO order.
- When we shutdown, we complete the client's requests we had already started executing as well as the pending ones in the buffer
- Synching in buffer is being achieved with two condition variables that are protected by buf_mtx , empty and full , where master thread(main) blocks while buffer is full and worker thread blocks while buffer is empty.
- We also have one mutex for accessing files and the data structures each uses , file_mtx , that guarantees mutual exclusion for results accuracy when accessing them.
- ## Usage
- Run server with make run_test_1 or make run_test_1 or providing arguments in the following format; 
 port_number num_Worker_threads buffer_Size poll-log  stats.txt
- Run client with make run_test_c or providing arguments in the following format; server_name port_number inputFile.txt
- Run create_input with make create or providing arguments in format; politicalParties.txt numLines
- Run tallyVotes with make tally or providing arguments in format; tallyResultsFile
- Run processLogFile or providing arguments in format; poll-log
