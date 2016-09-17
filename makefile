SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .cpp .o .d

EXE_NAME = xplat-pmc-tutorial-05.exe

# Generates a list of the modules with ".o" appended
OBJS := \
	main.o 		\
	gameharness.o	\
	tiledmap.o 	\
	sprite.o 	\
	utils.o 	\
	constants.o

# external libraries.
# remember ordering is important to the linker...
LIBS := \
	-lSDL2 \
	-lSDL2_image

REBUILDABLES := $(OBJS) $(EXE_NAME)

# All warning, debug output, C++11, x64
# later we can tease out the debug
CXXFLAGS += -Wall -g -std=c++11 -m64

# list of external paths
INCLUDES := \
	-I/usr/include/SDL2 \
	-I./include

all : $(EXE_NAME)
	@echo All done

# This is the linking rule, it creates the exe from the list of dependent objects
$(EXE_NAME) : $(OBJS)
	@echo Linking $@...
	g++ -g -o $@ $^ $(LIBS)

# Compilation rule, it matches the object's corresponding .cpp file
.cpp.o : 
	@echo Compiling $<...
	g++ -o $@ -c $(CXXFLAGS) $(INCLUDES) $<
	@echo

.PHONY : clean
clean : 
	rm -f $(REBUILDABLES)
	@echo Clean done
