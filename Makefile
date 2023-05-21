# Compiler
CC = gcc

# Compile Options
CFLAGS = -Wall -Werror -I -g

# Executable
EXEC = mysh

# Object files 
OBJ = aliases.o redirections.o commands.o mysh.o

all: $(EXEC)

$(EXEC): $(OBJ)

	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC)  -lrt -g

run: $(EXEC)
	./$(EXEC) 

clean:
	rm -f $(OBJ) $(EXEC) 