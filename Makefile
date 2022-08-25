CC = gcc
INCDIR = ./inc
SRCDIR = ./src
OBJDIR = ./obj
CFLAGS = -c -Wextra -Wall -I$(INCDIR)
LFLAGS = -lwiringPi -lpthread 
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
EXE = bin/app

all: clean $(EXE) 
    
$(EXE): $(OBJ) 
	$(CC) $(OBJDIR)/*.o -o $@ $(LFLAGS) 

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJDIR)/*.o 
	-rm -f $(EXE)

run:
	$(EXE)