#include <LevelConfig.h>
#include <tester.h>
#include <DummyEventLoop.h>
#include <StockFighterAPIMock.h>
#include <ExchangeOrder.h>
#include <test_checkers.h>
#include <AccountOrderManager.h>

using namespace TestChckers;

int InvalidAccount(testLogger& log);
int InvalidExchange(testLogger& log);
int ValidStock(testLogger& log);
int GetInvalidExchange(testLogger& log);
int GetValidExchange(testLogger& log);

int  main(int argc, char**argv) {
    Test("Creating an order  for the wrong account results in a nullptr", InvalidAccount).RunTest();
    Test("Creating an order  for the wrong exchange results in a nullptr", InvalidExchange).RunTest();
    Test("Creating a valid order works correctly!", ValidStock).RunTest();
    Test("Getting an invalid exchange results in a nullptr", GetInvalidExchange).RunTest();
    Test("Getting a valid exchange works correctly", GetValidExchange).RunTest();
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

const std::string altLevelConfig = R"(
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
                "Name": "Another Exchange",
                "Code": "ANOTHEREX"
            }
        ]
}
)";


class StockFighterAPIMock: public StockFighterAPIMockBase {
public:
    StockFighterAPIMock(DummyEventLoop* e = nullptr) 
        : StockFighterAPIMockBase(e) { } 

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

int InvalidAccount(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    OrderDefinition order;
    order.account = "NOTACC";
    order.symbol = "CaC";
    order.venue= "XXEUEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    AccountConfig& account = cfg.AccountsConfig()[0];
    Exchanges exchanges(api,cfg.VenuesConfig());
    AccountOrderManager manager(api,event_loop,account,exchanges);

    auto porder = manager.NewOrder(1, order);

    if ( porder != nullptr ) {
        log << "invalid account did not reutrn a nullptr!" << endl;
        return 1;
    }

    return 0;
}

int InvalidExchange(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "NOTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    AccountConfig& account = cfg.AccountsConfig()[0];
    Exchanges exchanges(api,cfg.VenuesConfig());
    AccountOrderManager manager(api,event_loop,account,exchanges);

    auto porder = manager.NewOrder(1, order);

    if ( porder != nullptr ) {
        log << "invalid account did not reutrn a nullptr!" << endl;
        return 1;
    }

    return 0;
}

int ValidStock(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    AccountConfig& account = cfg.AccountsConfig()[0];
    Exchanges exchanges(api,cfg.VenuesConfig());
    AccountOrderManager manager(api,event_loop,account,exchanges);

    auto porder = manager.NewOrder(1, order);

    if ( porder == nullptr ) {
        log << "valid stock returned a nullptr!" << endl;
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,*porder,1)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    return 0;
}

int GetInvalidExchange(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    AccountConfig& account = cfg.AccountsConfig()[0];
    Exchanges exchanges(api,cfg.VenuesConfig());

    AccountOrderManager manager(api,event_loop,account,exchanges);

    LevelConfig altCfg(altLevelConfig);
    Exchanges altExchanges(api,altCfg.VenuesConfig());
    Exchange& aEx = *altExchanges.GetExchange("ANOTHEREX");

    if ( manager.GetExchangeManager(aEx) != nullptr ) {
        log << "Invalid exchange did not return a nullptr!" << endl;
        return 1;
    }

    return 0;
}

int GetValidExchange(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    AccountConfig& account = cfg.AccountsConfig()[0];
    Exchanges exchanges(api,cfg.VenuesConfig());

    AccountOrderManager manager(api,event_loop,account,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    auto aorder = manager.NewOrder(1, order);

    Exchange& testex = *exchanges.GetExchange("TESTEX");

    auto emanager = manager.GetExchangeManager(testex);

    if ( emanager == nullptr ) {
        log << "Getting TESTEX returned a nullptr" << endl;
        return 1;
    }

    aorder->SendOrder();

    while (event_loop.RunNext()) { } 

    size_t eid = aorder->Details().exchangeOrderId;
    auto eorder = emanager->GetExchangeOrder(eid);

    if ( aorder != eorder ) {
        log << "Found order does not match created order!" << endl;
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,*eorder,1,eid)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if (!ValidateAcceptedOrder(log, *eorder, eid)){
        log << "Failed to validate order status!" << endl;
        return 1;
    }

    return 0;
}
