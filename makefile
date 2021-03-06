#please use 'make clean' to clean the directory of intermediate build files and the executable
#simply typing 'make' will compile all source code files to object files .o, and then link all
#object files into an executable
#we are using a lot of makefile macros

#changing platform dependant stuff, do not change this
# Linux (default)
LDFLAGS = -lGL -lGLU -lglut
CFLAGS= -w -g -Wall -std=c++11 
CC=g++
EXEEXT= square1
RM=rm
CXXFLAGS=-std=c++11

# Windows (cygwin)
ifeq "$(OS)" "Windows_NT"
	EXEEXT=.exe #on windows applications must have .exe extension
	RM=del #rm command for windows powershell
    LDFLAGS = -lfreeglut -lglu32 -lopengl32
else
# OS X
	OS := $(shell uname)
	ifeq ($(OS), Darwin)
	        LDFLAGS = -framework Carbon -framework OpenGL -framework GLUT
	endif
endif

#change the 't1' name to the name you want to call your application
PROGRAM_NAME= onRails.x

#run target to compile and build, and then launch the executable
run: $(PROGRAM_NAME)
	./$(PROGRAM_NAME)$(EXEXT)

#when adding additional source files, such as boilerplateClass.cpp
#or yourFile.cpp, add the filename with an object extension below
#ie. boilerplateClass.o and yourFile.o
#make will automatically know that the objectfile needs to be compiled
#form a cpp source file and find it itself :)
#$(PROGRAM_NAME): main.o basicMathLibrary.o Target.o basicShapeLibrary.o
 

$(PROGRAM_NAME): main.o Bullet.o Enemy.o basicMathLibrary.o Target.o basicShapeLibrary.o objLoad.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)


clean:
	$(RM) *.o $(PROGRAM_NAME)$(EXEEXT)
