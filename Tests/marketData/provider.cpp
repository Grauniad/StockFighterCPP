#include <tester.h>
#include <MarketDataProvider.h>
#include <LevelConfig.h>
#include <StockFighterAPIMock.h>
#include <test_checkers.h>
#include <DummyEventLoop.h>

int InvalidExchange(testLogger& log);
int InvalidStock(testLogger& log);
int EventLoopIntegration(testLogger& log);
int StockFiltering(testLogger& log);
int InitialData(testLogger& log);

int main(int argc, char** argv) {
    Test("Requesting market data for an invalid exchange return a nullptr",InvalidExchange).RunTest();
    Test("Requesting market data for an invali stock, on a valid exchange",InvalidStock).RunTest();
    Test("Validating event_loop integration...",EventLoopIntegration).RunTest();
    Test("Market data updates are filtered correctly by stock...",StockFiltering).RunTest();
    Test("A new market data client is populated with the current market data...",InitialData).RunTest();
    return 0;
}

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
            "Code": "GB86011564"
        }
    ],

    "Venues" : [
            {
                "Name": "XXEU Beijing Exchange",
                "Code": "XXEUEX"
            },
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

std::vector<QuoteHandler::Quote> XXEUEX_quotes = {
    {
        "SFIO",
        {5100,392},
        {5125,711},
        {5125,52}
    },
    {
        "CaC",
        {5100,392},
        {5125,711},
        {5125,52}
    },
    {
        "SFIO",
        {5123,4333333333},
        {5124,234},
        {234,2304}
    },
    {
        "CaC",
        {23432,392},
        {5125,711},
        {5125,23432}
    }
};


using namespace TestChckers;

class StockFighterAPIMock: public StockFighterAPIMockBase {
public:
    StockFighterReqHdl GetStocks(const std::string& exchange) {
        std::shared_ptr<TestHTTPRequest> request(new TestHTTPRequest);

        HTTPMessage& results = request->message;
        results.status_code = 404;
        if (exchange=="XXEUEX") {
            results.status_code = 200;
            results.content << StocksJSON({{"Stock Fighter IO","SFIO"},{"Cheap as Chips","CaC"}});
        } else if ( exchange == "TESTEX" ) {
            results.status_code = 200;
            results.content << StocksJSON({{"Test Stock","FOOBAR"}});
        }

        return request;
    }
};

int InvalidExchange(testLogger& log) {
    StockFighterAPIMock api;
    DummyEventLoop event_loop;
    LevelConfig config(levelConfig);
    AccountConfig& account = config.AccountsConfig()[0];
    Exchanges exchanges(api,config.VenuesConfig());
    const Exchanges::ExchangeRefList& exchangeList = exchanges.ExchangeList();

    MarketDataProvider marketData(api,account,exchangeList,event_loop);
    if ( marketData.NewUpdatesClient("NOTEX","FOOBAR") != nullptr ) {
        log << "NOTEX did not return NULL market data client!" << endl;
        return 1;
    }
    return 0;
}

int InvalidStock(testLogger& log) {
    StockFighterAPIMock api;
    DummyEventLoop event_loop;
    LevelConfig config(levelConfig);
    AccountConfig& account = config.AccountsConfig()[0];
    Exchanges exchanges(api,config.VenuesConfig());
    const Exchanges::ExchangeRefList& exchangeList = exchanges.ExchangeList();

    MarketDataProvider marketData(api,account,exchangeList,event_loop);
    if ( marketData.NewUpdatesClient("TESTEX","NOTS") != nullptr ) {
        log << "NOTS did not return a null market data client!" << endl;
        return 1;
    }
    return 0;
}

int EventLoopIntegration(testLogger& log) {
    StockFighterAPIMock api;
    DummyEventLoop event_loop;
    LevelConfig config(levelConfig);
    AccountConfig& account = config.AccountsConfig()[0];
    Exchanges exchanges(api,config.VenuesConfig());
    const Exchanges::ExchangeRefList& exchangeList = exchanges.ExchangeList();

    MarketDataProvider marketData(api,account,exchangeList,event_loop);

    MarketDataClient client = marketData.NewUpdatesClient("TESTEX","FOOBAR");

    if (client == nullptr) {
        log << "FOOBAR returned a nullptr!" << endl;
        return 1;
    }

    for (QuoteHandler::Quote q: TESTEX_quotes) {
        api.PublishQuote("TESTEX", q);
    }

    if ( !CheckClient(log,"Pre-Poll 1", client, {ZERO_quote})) {
        return 1;
    }

    if (!event_loop.RunNext()) {
        log << "No posted event to run!" << endl;
        return 1; 
    }

    if ( !CheckClient(log,"Post-Poll 1", client, TESTEX_quotes)) {
        return 1;
    }

    if (event_loop.RunNext()) {
        log << "Additional posted event to run!" << endl;
        return 1; 
    }

    for (QuoteHandler::Quote q: TESTEX_quotes) {
        api.PublishQuote("TESTEX", q);
    }

    if ( !CheckClient(log,"Pre-Poll 2", client, {})) {
        return 1;
    }

    if (!event_loop.RunNext()) {
        log << "No posted event to run!" << endl;
        return 1; 
    }

    if ( !CheckClient(log,"Post-Poll 2", client, TESTEX_quotes)) {
        return 1;
    }

    if (event_loop.RunNext()) {
        log << "Additional posted event to run!" << endl;
        return 1; 
    }

    return 0;
}

int CurrentData(testLogger& log) {
    StockFighterAPIMock api;
    DummyEventLoop event_loop;
    LevelConfig config(levelConfig);
    AccountConfig& account = config.AccountsConfig()[0];
    Exchanges exchanges(api,config.VenuesConfig());
    const Exchanges::ExchangeRefList& exchangeList = exchanges.ExchangeList();

    MarketDataProvider marketData(api,account,exchangeList,event_loop);

    MarketDataClient client = marketData.NewUpdatesClient("TESTEX","FOOBAR");

    if (client == nullptr) {
        log << "FOOBAR returned a nullptr!" << endl;
        return 1;
    }

    for (QuoteHandler::Quote q: TESTEX_quotes) {
        api.PublishQuote("TESTEX", q);
    }

    MarketData currentQuote = client->CurrentQuote();
    if ( !QuotesEqual(log,TESTEX_quotes[1],currentQuote)) {
        return 1;
    }

    api.PublishQuote("TESTEX", TESTEX_quotes[0]);

    MarketData currentQuote2 = client->CurrentQuote();
    if ( !QuotesEqual(log,TESTEX_quotes[0],currentQuote2)) {
        return 1;
    }

    MarketData currentQuote3 = client->CurrentQuote();
    if ( !QuotesEqual(log,TESTEX_quotes[0],currentQuote3)) {
        return 1;
    }

    return 0;
}

int StockFiltering(testLogger& log) {
    StockFighterAPIMock api;
    DummyEventLoop event_loop;
    LevelConfig config(levelConfig);
    AccountConfig& account = config.AccountsConfig()[0];
    Exchanges exchanges(api,config.VenuesConfig());
    const Exchanges::ExchangeRefList& exchangeList = exchanges.ExchangeList();

    MarketDataProvider marketData(api,account,exchangeList,event_loop);

    MarketDataClient CaC_client = marketData.NewUpdatesClient("XXEUEX","CaC");
    MarketDataClient SFIO_client = marketData.NewUpdatesClient("XXEUEX","SFIO");

    for (QuoteHandler::Quote q: XXEUEX_quotes) {
        api.PublishQuote("XXEUEX", q);
    }

    event_loop.RunNext();

    if ( !CheckClient(log,"SFIO", SFIO_client, {ZERO_quote, XXEUEX_quotes[0], XXEUEX_quotes[2]} )) {
        return 1;
    }

    if ( !CheckClient(log,"CaC", CaC_client, {ZERO_quote, XXEUEX_quotes[1], XXEUEX_quotes[3]} )) {
        return 1;
    }

    return 0;
}

int InitialData(testLogger& log) {
    StockFighterAPIMock api;
    DummyEventLoop event_loop;
    LevelConfig config(levelConfig);
    AccountConfig& account = config.AccountsConfig()[0];
    Exchanges exchanges(api,config.VenuesConfig());
    const Exchanges::ExchangeRefList& exchangeList = exchanges.ExchangeList();

    MarketDataProvider marketData(api,account,exchangeList,event_loop);

    for (QuoteHandler::Quote q: XXEUEX_quotes) {
        api.PublishQuote("XXEUEX", q);
    }

    event_loop.RunNext();

    MarketDataClient CaC_client = marketData.NewUpdatesClient("XXEUEX","CaC");
    MarketDataClient SFIO_client = marketData.NewUpdatesClient("XXEUEX","SFIO");

    if ( !CheckClient(log,"SFIO", SFIO_client, {XXEUEX_quotes[2]} )) {
        return 1;
    }

    if ( !CheckClient(log,"CaC", CaC_client, {XXEUEX_quotes[3]} )) {
        return 1;
    }

    return 0;
}
