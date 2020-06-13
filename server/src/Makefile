INCL=-I../incl

.SUFFIXES: .cpp
.cpp.o:
	g++ ${INCL} -c $<

Server: Tools.o Client_info.o Server.o
	g++ -o Server $? -lpthread
	rm -f $?
	mv Server ../bin

