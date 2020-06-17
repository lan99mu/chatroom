INCL=-I../incl

.SUFFIXES: .cpp
.cpp.o:
	g++ ${INCL} -c $<
all: Server

Server: Tools.o Client_info.o Chatroom_Server.o
	g++ -o chatroom_server $? -lpthread
	rm -f $?
	mv chatroom_server ../bin

install: 
	cp ../bin/chatroom_server /usr/bin/
	cp ../script/chatroom_server /etc/init.d/
