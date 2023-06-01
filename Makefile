# Compiler
CC = g++

# Compile Options
CFLAGS = -Wall -Werror -I -g -pthread

# make client
CLIENT = pollSwayer

# Object files 
OBJC = pollSwayer.o

$(CLIENT) : $(OBJC)
	$(CC) $(CFLAGS) $(OBJC) -o $(CLIENT)  -lrt 

# make server
SERVER = poller

# Object files 
OBJS = poller.o

$(SERVER) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(SERVER)  -lrt 

clean:
	rm -f $(OBJC) $(OBJS) $(SERVER) $(CLIENT) stat.txt log.txt  tallyResultsFile pollerResultsFile

run_test_1 : $(SERVER)
	./$(SERVER) 5005 10 5 log.txt stat.txt

run_test_2 : $(SERVER)
	./$(SERVER) 5005 3 10 log.txt stat.txt

run_test_c : $(CLIENT)
	./$(CLIENT) linux05.di.uoa.gr 5005 inputFile

create :
	./create_input.sh political_parties.txt 50

tally : 
	./tallyVotes.sh tallyResultsFile

log : 
	./processLogFile.sh log.txt
