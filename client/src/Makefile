INCL=-I../incl

.SUFFIXES: .cpp
.cpp.o:
	g++ ${INCL} -c $<
all: Client

Client: Tools.o Chatroom_Client.o
	g++ -o chatroom_client $? -lpthread
	rm -f $?
	mv chatroom_client ../bin

