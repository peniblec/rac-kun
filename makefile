ENABLE_DEBUG=-D DEBUG_ON
CCFLAGS=-Wall $(ENABLE_DEBUG)
RM=rm -f -R

SRC=src
INC=include

LIBS=-L/usr/lib -lboost_thread -lboost_system
LIBS_INC=/usr/include

INCLUDE=-I$(INC) -I$(LIBS_INC)

MESSAGES_OBJ=Message.o JoinMessage.o JoinNotifMessage.o JoinAckMessage.o ReadyMessage.o ReadyNotifMessage.o
OBJ=Main.o Config.o Listener.o $(MESSAGES_OBJ) Peer.o Network.o Utils.o
EXE=node


$(EXE): $(OBJ)
	g++ -o $(EXE) $(LIBS) $(OBJ)

Main.o : $(SRC)/Main.cpp $(INC)/Config.hpp
Config.o : $(SRC)/Config.cpp $(INC)/Config.hpp
Listener.o : $(SRC)/Listener.cpp $(INC)/Listener.hpp $(INC)/Config.hpp
Message.o : $(SRC)/Message.cpp $(INC)/Message.hpp $(INC)/Config.hpp
JoinMessage.o : $(SRC)/JoinMessage.cpp $(INC)/JoinMessage.hpp $(INC)/Config.hpp
JoinNotifMessage.o : $(SRC)/JoinNotifMessage.cpp $(INC)/JoinNotifMessage.hpp $(INC)/Config.hpp
JoinAckMessage.o : $(SRC)/JoinAckMessage.cpp $(INC)/JoinAckMessage.hpp $(INC)/Config.hpp
ReadyMessage.o : $(SRC)/ReadyMessage.cpp $(INC)/ReadyMessage.hpp $(INC)/Config.hpp
ReadyNotifMessage.o : $(SRC)/ReadyNotifMessage.cpp $(INC)/ReadyNotifMessage.hpp $(INC)/Config.hpp
Network.o : $(SRC)/Network.cpp $(INC)/Network.hpp $(INC)/Config.hpp
Peer.o : $(SRC)/Peer.cpp $(INC)/Peer.hpp $(INC)/Config.hpp
Utils.o : $(SRC)/Utils.cpp $(INC)/Utils.hpp $(INC)/Config.hpp


%.o : $(SRC)/%.cpp
	g++ -c $(CCFLAGS) $(INCLUDE) $<

clean :
	$(RM) $(OBJ) $(EXE)