SOURCES=$(shell echo *.cpp)
EXPORT_INCLUDES=$(shell echo *.h)

LINKED_LIBS= libRequestServer libOrder libMarketData libExchangeInterface libStockFighter libLevelConfig libThreadComms libWebSockets libTest libJSON libMeta libUtils libIOInterface 

TARGET_LIB=libTestUtils

MODE=CPP
TAGS_FILE=stock-figher-libs-test-utils-c++.tags
USE_JSON=YES
USE_WEB_SOCKET_CPP=YES
EXTERNAL_LIBS=-lssl -lcrypto



include ../../makefile.include
