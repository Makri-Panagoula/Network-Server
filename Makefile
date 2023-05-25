# Compiler
CC = g++

# Compile Options
CFLAGS = -Wall -Werror -I -g -pthread

# Executable
CLIENT = pollSwayer

# Object files 
OBJC = pollSwayer.o

$(CLIENT) : $(OBJC)
	$(CC) $(CFLAGS) $(OBJC) -o $(CLIENT)  -lrt 

# Executable
SERVER = poller

# Object files 
OBJS = poller.o

$(SERVER) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(SERVER)  -lrt 
# $(EXEC): $(OBJ)

# 	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC) -g

run: $(EXEC)
	./$(EXEC) 
clean:
	rm -f $(OBJ) $(EXEC) 