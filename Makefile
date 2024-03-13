# Compiler
CC = g++

BASH = ./bash_scripts
SRC = ./src
INPUT = ./my_test_inputs

# Compile Options
CFLAGS = -Wall -Werror -I -g -pthread

# make client
CLIENT = pollSwayer

# Object files 
OBJC = $(SRC)/pollSwayer.o

$(CLIENT) : $(OBJC)
	$(CC) $(CFLAGS) $(OBJC) -o $(CLIENT)  -lrt 

# make server
SERVER = poller

# Object files 
OBJS = $(SRC)/poller.o


$(SERVER) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(SERVER)  -lrt 

clean:
	rm -f $(OBJC) $(OBJS) $(SERVER) $(CLIENT) stat.txt log.txt  tallyResultsFile pollerResultsFile inputFile

run_test_1 : $(SERVER)
	./$(SERVER) 5005 10 5 log.txt stat.txt

run_test_2 : $(SERVER)
	./$(SERVER) 5005 3 10 log.txt stat.txt

run_test_c : $(CLIENT)
	./$(CLIENT) linux05.di.uoa.gr 5005 $(INPUT)/inputFile

create :
	$(BASH)/create_input.sh $(INPUT)/political_parties.txt 50

tally : 
	$(BASH)/tallyVotes.sh tallyResultsFile

log : 
	$(BASH)/processLogFile.sh log.txt
