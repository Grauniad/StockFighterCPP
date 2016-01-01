LINKED_LIBS= libTestUtils libOrder libMarketData libStockFighter \
             libExchangeInterface \
             libLevelConfig \
             libThreadComms \
             libWebSockets \
             libJSON \
             libIOInterface\
             libUtils\
			 libTest

BUILD_TIME_TESTS=exchange_order exchange_manager account_manager validation order_manager action_handler
CPP_TAGS_FILE=stock-fighter-tests-orders-c++.tags
MODE=CPP

USE_BOOST=YES
USE_WEB_SOCKET_CPP=YES
USE_JSON=YES
USE_THREADS=YES
EXTERNAL_LIBS=-lssl -lcrypto

include ../../makefile.include
