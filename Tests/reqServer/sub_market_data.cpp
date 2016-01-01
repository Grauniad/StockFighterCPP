#include <tester.h>
#include <LevelConfig.h>
#include <SubMarketData.h>
#include <StockFighterAPIMock.h>
#include <DummyEventLoop.h>
#include <Exchanges.h>
#include <MarketDataManager.h>
#include <test_checkers.h>
#include <SubscriptionRequestMock.h>
#include <algorithm>
#include <json_tests.h>

using namespace JSONCheckers;

int InvalidAccount(testLogger& log);
int InvalidExchange(testLogger& log);
int InvalidStock(testLogger& log);
int NoMarketData(testLogger& log);
int InitialMarketData(testLogger& log);
int MarketDataUpdates(testLogger& log);

int main(int argc, char** argv) {
    Test("Sending an invalid account results in a rejected request",InvalidAccount).RunTest();
    Test("Sending an invalid exchange results in a rejected request",InvalidExchange).RunTest();
    Test("Sending an invalid stock results in a rejected request",InvalidStock).RunTest();
    Test("Current market data is sent back immediately",InitialMarketData).RunTest();
    Test("An all zero initial image is returned if there is no market data...",NoMarketData).RunTest();
    Test("Market data updates are pushed to the client",MarketDataUpdates).RunTest();
    return 0;
}

using namespace TestChckers;

const std::string levelConfig = R"(
{
    "Client": {
        "port"   : 8081
    },
    "StockFighter": {
        "authKey"   : "7f60efa6b0e4a5894ea0845d9f3b287940a7eeed",
        "apiServer" : "test.api.stockfighter.io",
        "apiServerBaseUrl": "/testing/ob/api/"
    },

    "Accounts" : [
        {
            "Name": "Trading Account",
            "Code": "TESTACC"
        }
    ],

    "Venues" : [
            {
                "Name": "The test exchange.",
                "Code": "TESTEX"
            }
        ]
}
)";

QuoteHandler::Quote ZERO_quote {
    "",
    {0,0},
    {0,0},
    {0,0}
};

std::vector<QuoteHandler::Quote> TESTEX_quotes = {
    {
        "FOOBAR",
        {5100,392},
        {5125,711},
        {5125,52}
    },
    {
        "FOOBAR",
        {5123,245098},
        {5124,234},
        {234,2304}
    }
};




class StockFighterAPIMock: public StockFighterAPIMockBase {
public:
    StockFighterReqHdl GetStocks(const std::string& exchange) {
        std::shared_ptr<TestHTTPRequest> request(new TestHTTPRequest);

        HTTPMessage& results = request->message;
        results.status_code = 404;

        if ( exchange == "TESTEX" ) {
            results.status_code = 200;
            results.content << StocksJSON({{"Test Stock","FOOBAR"}});
        }

        return request;
    }
};

int InvalidAccount(testLogger& log) { 
    LevelConfig config(levelConfig);
    DummyEventLoop event_loop;
    StockFighterAPIMock api;
    Exchanges exchanges(api,config.VenuesConfig());
    MarketDataManager marketData(api,exchanges,event_loop);
    SubMarketData handler(config, marketData);

    std::string request = R"( {
        "account": "NOTACC",
        "exchange": "Exchange",
        "symbol": "Stock ID"
    })";

    if (!SubscriptionIsRejected(log,handler,request,"Invalid account supplied.",0)) {
        return 1;
    }

    return 0;
}

int InvalidExchange(testLogger& log) { 
    LevelConfig config(levelConfig);
    DummyEventLoop event_loop;
    StockFighterAPIMock api;
    Exchanges exchanges(api,config.VenuesConfig());
    MarketDataManager marketData(api,exchanges,event_loop);
    SubMarketData handler(config, marketData);

    std::string request = R"( {
        "account": "TESTACC",
        "exchange": "NOTEX",
        "symbol": "Stock ID"
    })";

    if (!SubscriptionIsRejected(log,handler,request,"Invalid exchange / stock combination supplied.",0)) {
        return 1;
    }

    return 0;
}

NewDoubleField(bidPrice);
NewDoubleField(askPrice);
NewDoubleField(lastPrice);
typedef SimpleParsedJSON<bidPrice,askPrice,lastPrice> MarketDataUpdate;

void PopulateResult(MarketDataUpdate& expected, const QuoteHandler::Quote& source) {
    expected.Clear();
    expected.Get<bidPrice>() = 0.01 * source.bestBid.price;
    expected.Get<askPrice>() = 0.01 *  source.bestAsk.price;
    expected.Get<lastPrice>() = 0.01 * source.last.price;
}


int InvalidStock(testLogger& log) { 
    LevelConfig config(levelConfig);
    DummyEventLoop event_loop;
    StockFighterAPIMock api;
    Exchanges exchanges(api,config.VenuesConfig());
    MarketDataManager marketData(api,exchanges,event_loop);
    SubMarketData handler(config, marketData);

    std::string request = R"( {
        "account": "TESTACC",
        "exchange": "TESTEX",
        "symbol": "NOTS"
    })";

    if (!SubscriptionIsRejected(log,handler,request,"Invalid exchange / stock combination supplied.",0)) {
        return 1;
    }

    return 0;
}

int NoMarketData(testLogger& log) { 
    LevelConfig config(levelConfig);
    DummyEventLoop event_loop;
    StockFighterAPIMock api;
    Exchanges exchanges(api,config.VenuesConfig());
    MarketDataManager marketData(api,exchanges,event_loop);
    SubMarketData handler(config, marketData);

    std::string request = R"( {
        "account": "TESTACC",
        "exchange": "TESTEX",
        "symbol": "FOOBAR"
    })";

    MarketDataUpdate expected; 
    MarketDataUpdate actual; 

    expected.Get<bidPrice>() = 0.0;
    expected.Get<askPrice>() = 0.0;
    expected.Get<lastPrice>() = 0.0;

    SubRequestMock::Handle h = SubRequestMock::MakeHandle(request);

    if (!SubscriptionIsAccepted<askPrice,bidPrice,lastPrice>(log,handler,h,actual,expected)) {
        return 1;
    }

    return 0;
}

int InitialMarketData(testLogger& log) { 
    LevelConfig config(levelConfig);
    DummyEventLoop event_loop;
    StockFighterAPIMock api;
    Exchanges exchanges(api,config.VenuesConfig());
    MarketDataManager marketData(api,exchanges,event_loop);
    SubMarketData handler(config, marketData);

    std::string request = R"( {
        "account": "TESTACC",
        "exchange": "TESTEX",
        "symbol": "FOOBAR"
    })";

    MarketDataUpdate expected; 
    MarketDataUpdate actual; 
    // Spin up the quote handler so it can handle the initial quote.
    marketData.GetMarketDataProvider(config.AccountsConfig()[0]);

    // Publish an initial quote
    QuoteHandler::Quote& q = TESTEX_quotes[0];
    api.PublishQuote("TESTEX", q);

    // Poll the event_loop to give the provider an opportunity to
    // handle the data.
    event_loop.RunNext();

    SubRequestMock::Handle h = SubRequestMock::MakeHandle(request);
    PopulateResult(expected,q);

    if (!SubscriptionIsAccepted<askPrice,bidPrice,lastPrice>(log,handler,h,actual,expected)) {
        return 1;
    }

    return 0;
}

int MarketDataUpdates(testLogger& log) { 
    LevelConfig config(levelConfig);
    DummyEventLoop event_loop;
    StockFighterAPIMock api;
    Exchanges exchanges(api,config.VenuesConfig());
    MarketDataManager marketData(api,exchanges,event_loop);
    SubMarketData handler(config, marketData);

    std::string request = R"( {
        "account": "TESTACC",
        "exchange": "TESTEX",
        "symbol": "FOOBAR"
    })";

    MarketDataUpdate expected; 
    MarketDataUpdate actual; 

    SubRequestMock::Handle h = SubRequestMock::MakeHandle(request);
    PopulateResult(expected,ZERO_quote);

    if (!SubscriptionIsAccepted<askPrice,bidPrice,lastPrice>(log,handler,h,actual,expected)) {
        return 1;
    }

    // Publish an initial quote
    QuoteHandler::Quote& q1 = TESTEX_quotes[0];
    QuoteHandler::Quote& q2 = TESTEX_quotes[1];
    QuoteHandler::Quote& q3 = TESTEX_quotes[0];
    api.PublishQuote("TESTEX", q1);
    api.PublishQuote("TESTEX", q2);

    // Poll the event_loop to give the provider an opportunity to
    // handle the data.
    event_loop.RunNext();

    PopulateResult(expected,q1);

    if (!CheckSubscriptionMessage<askPrice,bidPrice,lastPrice>(log,h,actual,expected)) {
        return 1;
    }

    PopulateResult(expected,q2);

    if (!CheckSubscriptionMessage<askPrice,bidPrice,lastPrice>(log,h,actual,expected)) {
        return 1;
    }

    api.PublishQuote("TESTEX", q3);

    if (!NoMoreMessages(log,h)) {
        return 1;
    }

    event_loop.RunNext();
    PopulateResult(expected,q3);

    if (!CheckSubscriptionMessage<askPrice,bidPrice,lastPrice>(log,h,actual,expected)) {
        return 1;
    }

    if (!NoMoreMessages(log,h)) {
        return 1;
    }

    return 0;
}
