# List of files
C_HEADERS = assembler.h lc3.h        symbol.h tokens.h
C_SRCS	  = assembler.c       main.c symbol.c
C_OBJS	  = assembler.o       main.o symbol.o
EXE       = mylc3as
ASM_LIB   = lc3as.a
DEFINES   =

# Compiler and loader commands and flags
GCC		= gcc
GCC_FLAGS	= -g -std=c99 -Wall $(DEFINES) -c
LD_FLAGS	= -g -std=c99 -Wall

# Compile .c files to .o files
.c.o:
	$(GCC) $(GCC_FLAGS) $<

all:    $(EXE) seeLC3 testTokens

# Target is the executable
$(EXE): $(ASM_LIB) $(C_OBJS)
	$(GCC) $(LD_FLAGS) -o $(EXE) $(C_OBJS) $(ASM_LIB)

testTokens: $(ASM_LIB) testTokens.o symbol.o
	$(GCC) $(LD_FLAGS) -o testTokens testTokens.o symbol.o $(ASM_LIB)

seeLC3: seeLC3.o symbol.o $(ASM_LIB)
	$(GCC) $(LD_FLAGS) seeLC3.o symbol.o $(ASM_LIB) -o seeLC3

# Recompile C objects if headers change
${C_OBJS}: ${C_HEADERS}

# Clean up the directory
clean:
	rm -f *.o *~ P10.tar $(EXE) testTokens seeLC3

# Package files
package:
	tar -cvf P10.tar assembler.c symbol.c


