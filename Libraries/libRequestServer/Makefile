SOURCES=$(shell echo *.cpp)
EXPORT_INCLUDES=$(shell echo *.h)

LINKED_LIBS= libOrder libMarketData libExchangeInterface libStockFighter libLevelConfig libThreadComms libWebSockets libJSON libUtils libIOInterface 

TARGET_LIB=libRequestServer

MODE=CPP
TAGS_FILE=stock-figher-libs-requests-c++.tags
USE_JSON=YES
USE_WEB_SOCKET_CPP=YES
EXTERNAL_LIBS=-lssl -lcrypto



include ../../makefile.include
