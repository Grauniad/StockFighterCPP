SOURCES=$(shell echo *.cpp)

LINKED_LIBS= libRequestServer libOrder libMarketData libExchangeInterface libStockFighter libLevelConfig libThreadComms libWebSockets libJSON libUtils libIOInterface 

EXECUTABLE=stock_fighter
MODE=CPP
CPP_TAGS_FILE=stockfighter-server-c++.tags
USE_JSON=YES
USE_WEB_SOCKET_CPP=YES
EXTERNAL_LIBS=-lssl -lcrypto



include ../makefile.include
