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

run_client: $(CLIENT)
	./$(CLIENT) linux07.di.uoa.gr 5634 inputFile.txt

# make server
SERVER = poller

# Object files 
OBJS = poller.o

$(SERVER) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(SERVER)  -lrt 

run_server: $(SERVER)
	./$(SERVER)  5634 8 16 pollLog.txt pollStats.txt

clean:
	rm -f $(OBJC) $(OBJS) $(SERVER) $(CLIENT) 