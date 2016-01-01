#include <LevelConfig.h>
#include <tester.h>
#include <DummyEventLoop.h>
#include <StockFighterAPIMock.h>
#include <ExchangeOrder.h>
#include <test_checkers.h>
#include <ExchangeOrders.h>

using namespace TestChckers;

int InvalidExchange(testLogger& log);
int InvalidStock(testLogger& log);
int ValidStock(testLogger& log);
int UnknownExchangeId(testLogger& log);
int GetOrderFromExchangeId(testLogger& log);

int  main(int argc, char**argv) {
    Test("Creating an order  for the wrong exchange results in a nullptr", InvalidExchange).RunTest();
    Test("Creating an order  for an unknown stock results in a nullptr", InvalidStock).RunTest();
    Test("Create a new draft order",ValidStock).RunTest();
    Test("An unknown exchange ID results in a nullptr",UnknownExchangeId).RunTest();
    Test("Get an order from an exchange ID",GetOrderFromExchangeId).RunTest();
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

int InvalidExchange(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "CaC";
    order.venue= "XXEUEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    AccountConfig& account = cfg.AccountsConfig()[0];
    Exchanges exchanges(api,cfg.VenuesConfig());
    Exchange& testex = *exchanges.GetExchange("TESTEX");
    ExchangeOrderManager manager(api,event_loop,account,testex);

    auto porder = manager.NewOrder(1, order);

    if ( porder != nullptr ) {
        log << "invalid exchange did not reutrn a nullptr!" << endl;
        return 1;
    }

    return 0;
}

int InvalidStock(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "CaC";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    AccountConfig& account = cfg.AccountsConfig()[0];
    Exchanges exchanges(api,cfg.VenuesConfig());
    Exchange& testex = *exchanges.GetExchange("TESTEX");
    ExchangeOrderManager manager(api,event_loop,account,testex);

    auto porder = manager.NewOrder(1, order);

    if ( porder != nullptr ) {
        log << "invalid stock did not reutrn a nullptr!" << endl;
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
    Exchange& testex = *exchanges.GetExchange("TESTEX");
    ExchangeOrderManager manager(api,event_loop,account,testex);

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

int UnknownExchangeId(testLogger& log) {
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
    Exchange& testex = *exchanges.GetExchange("TESTEX");
    ExchangeOrderManager manager(api,event_loop,account,testex);

    auto porder = manager.NewOrder(1, order);

    if ( porder == nullptr ) {
        log << "valid stock returned a nullptr!" << endl;
        return 1;
    }

    size_t unknown_e_id = porder->Details().exchangeOrderId;

    if ( manager.GetExchangeOrder(unknown_e_id) != nullptr) {
        log << "Found order by unknown change order id!" << endl;
        return 1;
    }

    return 0;
}

int GetOrderFromExchangeId(testLogger& log) {
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
    Exchange& testex = *exchanges.GetExchange("TESTEX");
    ExchangeOrderManager manager(api,event_loop,account,testex);

    auto porder = manager.NewOrder(1, order);

    if ( porder == nullptr ) {
        log << "valid stock returned a nullptr!" << endl;
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,*porder,1)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    porder->SendOrder();
    // Pull the message from the exchange
    event_loop.RunNext();
    // Execute the posted task
    event_loop.RunNext();

    size_t eid = porder->Details().exchangeOrderId;
    auto eorder = manager.GetExchangeOrder(eid);

    if ( eorder == nullptr ) {
        log << "Exchange order not found for valid ID!" << endl;  
        return 1;
    }

    if ( !event_loop.RunNext() ) {
        log << "No event to remove client subscription!" << endl;
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,*eorder,1,eid)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if ( porder != eorder ) {
        log << "Exchange order location does not match that returned by search!" << endl;
        return 1;
    }

    return 0;
}
