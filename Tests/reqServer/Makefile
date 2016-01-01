LINKED_LIBS= libTestUtils \
             libRequestServer \
             libOrder \
             libMarketData \
             libStockFighter \
             libExchangeInterface \
             libLevelConfig \
             libThreadComms \
             libWebSockets \
             libJSON \
             libIOInterface\
             libUtils\
             libMeta\
			 libTest

BUILD_TIME_TESTS=req_accounts req_venues sub_market_data req_new_order
CPP_TAGS_FILE=stock-fighter-tests-request-server-data-c++.tags
MODE=CPP

USE_BOOST=YES
USE_WEB_SOCKET_CPP=YES
USE_JSON=YES
USE_THREADS=YES
EXTERNAL_LIBS=-lssl -lcrypto

include ../../makefile.include
