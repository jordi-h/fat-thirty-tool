######################################
 # Standart Makefile for any C code
 #
 # author: Jordi Hoorelbeke
 #
######################################

## Nom de l'exécutable: exec

######################################################
# $@ = le nom de la cible
# $< = le nom de la première dépendance
# $^ = la liste des dépendances
# $? = la liste des dépendances plus récentes que la cible
# $* = le nom du fichier suffixe
#
# wildcard is to enable the usage of *
# @ in front of commands is to silent it
# .PHONY is a target which has dependences that will systematically be reconstructed
######################################################

## Variables
# Tools & flags
CC=gcc
CFLAGS=-g --std=c99 --pedantic -Wall -Wextra -Wmissing-prototypes -DNDEBUG -O3
LD=gcc
LDFLAGS=-lm

# Files
EXEC=fat32_tool
SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)

## Rules
all: $(EXEC)

$(EXEC): $(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS)

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean

clean:
	@rm -rf *.o $(EXEC) *~
	clear