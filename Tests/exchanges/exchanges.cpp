#include <Exchanges.h>
#include <tester.h>
#include <LevelConfig.h>
#include <StockFighterAPIMock.h>

int venuesConfig(testLogger& log);
int getInvalidExchange(testLogger& log);
int getMissConfiguredExchange(testLogger& log);
int getValidExchange(testLogger& log);
int validExchangeStocks(testLogger& log);
int moveCTOR(testLogger& log);

int main(int argc, char**argv) {
    Test("Valid venues config is read in correctly...",venuesConfig).RunTest();
    Test("Getting an invalid echange returns null",getInvalidExchange).RunTest();
    Test("Getting a valid echange returns correct result",getValidExchange).RunTest();
    Test("Get a miss-configured exchange",getMissConfiguredExchange).RunTest();
    Test("Get stocks for valid exchanges",validExchangeStocks).RunTest();
    Test("Move C'tor exchanges",moveCTOR).RunTest();
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

const std::string levelMissConfig = R"(
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
            },
            {
                "Name": "This is not an exchange",
                "Code": "NOTEX"
            }
        ]
}
)";

class StockFighterAPIMock: public StockFighterAPIMockBase {
public:

    StockFighterReqHdl GetStocks(const std::string& exchange) {
        std::shared_ptr<TestHTTPRequest> request(new TestHTTPRequest);

        HTTPMessage& results = request->message;
        results.status_code = 404;
        if (exchange=="XXEUEX") {
            results.status_code = 200;
            results.content << StocksJSON({{"Stock Fighter IO","SFIO"},{"Cheap as Chips","Cac"}});
        } else if ( exchange == "TESTEX" ) {
            results.status_code = 200;
            results.content << StocksJSON({{"Test Stock","FOOBAR"}});
        }

        return request;
    }
};

bool TestKeyExchangeFields(testLogger& log,
                           Exchange& exch,
                           const std::string& name,
                           const std::string& id) 
{
    bool ok = true;
    if ( exch.Name() != name ) {
        ok = false;
        log.ReportStringDiff(name,exch.Name());
    }

    if ( exch.Id() != id ) {
        ok = false;
        log.ReportStringDiff(id,exch.Id());
    }

    if ( !ok) { 
        log << "Failed to validate exchange " << name << endl;
    }

    return ok;
}

bool TestReadiedExchange(testLogger& log,
                           Exchange& exch,
                           const std::string& name,
                           const std::string& id) 
{
    bool ok = TestKeyExchangeFields(log,exch,name,id);

    if ( !exch.Ready()) {
        log << "Exchange is not ready!" << endl;
        ok = false;
    }

    if ( !exch.OK()) {
        log << "Exchange is not OK" << endl;
        ok = false;
    }

    if ( !ok) { 
        log << "Failed to validate readied exchange " << name << endl;
    }

    return ok;
}

bool TestExchangeStocks(testLogger& log,
                        Exchange& exch,
                        std::vector<StockFighterAPIMock::raw_stock> stocks)
{
    bool ok = true;

    size_t stocksToCheck = stocks.size();

    if (stocks.size() != exch.Stocks().size() ) {
        log << "Invvalid number of stocks: " << endl;
        log << "   Expected: " << stocks.size() << endl;
        log << "   Actual:   " << exch.Stocks().size() << endl;
        ok = false;
        if (stocksToCheck > exch.Stocks().size()) {
            stocksToCheck = exch.Stocks().size();
        }
    }

    for (size_t i = 0; i < stocksToCheck; ++i) {
        StockFighterAPIMock::raw_stock& exp = stocks[i];
        Stock& act = exch.Stocks()[i];

        if (exp.name != act.Name()) {
            log << "Name missmatch!" << endl;
            log.ReportStringDiff(exp.name,act.Name());
            ok = false;
        }

        if (exp.id != act.Ticker()) {
            log << "Name missmatch!" << endl;
            log.ReportStringDiff(exp.id,act.Ticker());
            ok = false;
        }
    }

    if (!ok) {
        log << "Stocks on exchange " << exch.Name() << " did not match!" << endl;
    }

    return ok;
}


int venuesConfig(testLogger& log) {
    LevelConfig config(levelConfig);
    StockFighterAPIMock api;

    Exchanges exchanges(api,config.VenuesConfig());

    Exchange& XXEUEX = *exchanges.ExchangeList()[0];
    Exchange& TESTEX = *exchanges.ExchangeList()[1];

    bool ok  = TestKeyExchangeFields(log,XXEUEX, "XXEU Beijing Exchange", "XXEUEX");
    ok &= TestKeyExchangeFields(log,TESTEX, "The test exchange.", "TESTEX");

    if ( !ok) {
        return 1;
    }

    return 0;
}

int getInvalidExchange(testLogger& log ) {
    LevelConfig config(levelConfig);
    StockFighterAPIMock api;

    Exchanges exchanges(api,config.VenuesConfig());

    if ( exchanges.GetExchange("NOT AN Exchange") != nullptr) {
        return 1;
    }

    return 0;
}


int getValidExchange(testLogger& log) {
    LevelConfig config(levelConfig);
    StockFighterAPIMock api;

    Exchanges exchanges(api,config.VenuesConfig());

    Exchange& XXEUEX = *exchanges.GetExchange("XXEUEX");
    Exchange& TESTEX = *exchanges.GetExchange("TESTEX");

    bool ok  = TestReadiedExchange(log,XXEUEX, "XXEU Beijing Exchange", "XXEUEX");
    ok &= TestReadiedExchange(log,TESTEX, "The test exchange.", "TESTEX");

    if ( !ok) {
        return 1;
    }

    return 0;
}

int getMissConfiguredExchange(testLogger& log) {
    LevelConfig config(levelMissConfig);
    StockFighterAPIMock api;

    Exchanges exchanges(api,config.VenuesConfig());

    Exchange& NOTEX = *exchanges.GetExchange("NOTEX");

    bool ok  = TestKeyExchangeFields(log,NOTEX, "This is not an exchange", "NOTEX");

    if ( !ok) {
        return 1;
    }

    if (!NOTEX.Ready()) {
        log << "Exchange is not ready!" << endl;
        return 1;
    }

    if (NOTEX.OK()) {
        log << "NOTEX Initialised correctly!" << endl;
        return 1;
    }

    if (NOTEX.Stocks().size() != 0) {
        log << "NOTEX has stocks!" << endl;
        return 1;
    }

    return 0;
}

int validExchangeStocks(testLogger& log ) {
    LevelConfig config(levelConfig);
    StockFighterAPIMock api;

    Exchanges exchanges(api,config.VenuesConfig());

    Exchange& XXEUEX = *exchanges.GetExchange("XXEUEX");
    Exchange& TESTEX = *exchanges.GetExchange("TESTEX");

    bool ok  = TestReadiedExchange(log,XXEUEX, "XXEU Beijing Exchange", "XXEUEX");
    ok &= TestReadiedExchange(log,TESTEX, "The test exchange.", "TESTEX");

    ok &= TestExchangeStocks(log,TESTEX,{{"Test Stock","FOOBAR"}});
    ok &= TestExchangeStocks(log,XXEUEX,{{"Stock Fighter IO","SFIO"},{"Cheap as Chips","Cac"}});

    if ( !ok) {
        return 1;
    }

    return 0;
}

int moveCTOR(testLogger& log ) {
    LevelConfig config(levelConfig);
    StockFighterAPIMock api;

    auto get_exchanges = [&config, &api] () -> Exchanges {
        Exchanges exch(api,config.VenuesConfig());
        return exch;
    };

    Exchanges exchanges = get_exchanges();

    Exchange& XXEUEX = *exchanges.GetExchange("XXEUEX");
    Exchange& TESTEX = *exchanges.GetExchange("TESTEX");

    bool ok  = TestReadiedExchange(log,XXEUEX, "XXEU Beijing Exchange", "XXEUEX");
    ok &= TestReadiedExchange(log,TESTEX, "The test exchange.", "TESTEX");

    ok &= TestExchangeStocks(log,TESTEX,{{"Test Stock","FOOBAR"}});
    ok &= TestExchangeStocks(log,XXEUEX,{{"Stock Fighter IO","SFIO"},{"Cheap as Chips","Cac"}});

    if ( !ok) {
        return 1;
    }

    return 0;
}
