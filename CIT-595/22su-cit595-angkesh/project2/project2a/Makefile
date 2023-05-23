# Define variable CC to be the compiler we want to use
CC = clang

# Define CFLAGS for the flags we will want to use with clang
CLANG = -g -Wall

TARGETS = clean penn-sh



# This runs the 'clean' and 'penn-sh' targets when 'make all' is run
all: $(TARGETS)


tokenizer.o: tokenizer.c tokenizer.h
	clang -c -g tokenizer.c
# This will construct the binary 'penn-sh'
# $^ = names of all the dependent files, deduped and with spaces
# $@ = complete name of the target
penn-sh: penn-sh.c tokenizer.o
	clang $(CLANG) penn-sh.c tokenizer.o -o penn-sh

# $(RM) is the platform agnostic way to delete a file (here rm -f)
clean:
	$(RM) penn-sh
