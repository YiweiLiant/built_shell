myShell: myShell.cpp CommandLine.h pipeCommand.h CommandLine.cpp pipeCommand.cpp
	g++ -o myShell -ggdb3 -pedantic -std=gnu++98 -Wall -Werror -ggdb3 myShell.cpp CommandLine.h pipeCommand.h CommandLine.cpp pipeCommand.cpp

