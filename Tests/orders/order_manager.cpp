#include <LevelConfig.h>
#include <tester.h>
#include <DummyEventLoop.h>
#include <StockFighterAPIMock.h>
#include <ExchangeOrder.h>
#include <test_checkers.h>
#include <OrderManager.h>

using namespace TestChckers;

int InvalidAccount(testLogger& log);
int NewOrder(testLogger& log);
int AccountMgrInvalid(testLogger& log);
int AccountMgr(testLogger& log);
int InvalidOrderId(testLogger& log);
int ValidOrderId(testLogger& log);

int  main(int argc, char**argv) {
    Test("Creating an order for an unknown account results in a nullptr", InvalidAccount).RunTest();
    Test("Create a new order", NewOrder).RunTest();
    Test("Getting manager for invalid account returns nullptr", AccountMgrInvalid).RunTest();
    Test("Getting valid account manager", AccountMgr).RunTest();
    Test("Getting an invalid order ID returns a nullptr", InvalidOrderId).RunTest();
    Test("Getting a valid order returns the correct order",ValidOrderId).RunTest();
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
            "Name": "Another Account",
            "Code": "ANOTHERACC"
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

    Exchanges exchanges(api,cfg.VenuesConfig());
    OrderManager manager(api,event_loop,cfg,exchanges);

    auto porder = manager.NewOrder(order);

    if ( porder != nullptr ) {
        log << "invalid account did not reutrn a nullptr!" << endl;
        return 1;
    }

    return 0;
}

int NewOrder(testLogger& log) {
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

    Exchanges exchanges(api,cfg.VenuesConfig());
    OrderManager manager(api,event_loop,cfg,exchanges);

    auto porder = manager.NewOrder(order);

    if ( porder == nullptr ) {
        log << "valid order returned a nullptr!" << endl;
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,*porder,1)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    auto porder2 = manager.NewOrder(order);

    OrderDefinition order2;
    order2.account = "GB86011564";
    order2.symbol = "CaC";
    order2.venue= "XXEUEX";
    order2.direction = OrderDefinition::DIRECTION_SELL;
    order2.type = OrderDefinition::ORDER_TYPE_MARKET;
    order2.originalQuantity = 9;
    order2.limitPrice = 0;

    if ( porder2 == nullptr ) {
        log << "valid order 2 returned a nullptr!" << endl;
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,*porder2,2)) {
        log << "Failed to validate order 2 details!" << endl;
        return 1;
    }

    return 0;
}

int AccountMgrInvalid(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    Exchanges exchanges(api,cfg.VenuesConfig());
    OrderManager manager(api,event_loop,cfg,exchanges);

    LevelConfig altCfg(altLevelConfig);
    AccountConfig& another = altCfg.AccountsConfig()[0]; 

    if ( manager.GetAccountManager(another) != nullptr ) {
        log << "Invalid account returned non-nullptr!" << endl;
        return 1;
    }

    return 0;
}

int AccountMgr(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    Exchanges exchanges(api,cfg.VenuesConfig());
    OrderManager manager(api,event_loop,cfg,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    auto porder = manager.NewOrder(order);

    AccountConfig& trading = *cfg.GetAccountConfig("GB86011564");
    Exchange& testex = *exchanges.GetExchange("TESTEX");

    auto amanager = manager.GetAccountManager(trading);

    if ( amanager == nullptr ) {
        log << "Valid account returned nullptr!" << endl;
        return 1;
    }

    auto emanager = amanager->GetExchangeManager(testex);

    if ( emanager == nullptr ) {
        log << "Could not get exchnage manager from account manager!" << endl;
        return 1;
    }

    porder->SendOrder();
    while (event_loop.RunNext()) { } 

    size_t eid = porder->Details().exchangeOrderId;
    auto eorder = emanager->GetExchangeOrder(eid);

    if ( porder != eorder ) {
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

int InvalidOrderId(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    Exchanges exchanges(api,cfg.VenuesConfig());
    OrderManager manager(api,event_loop,cfg,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    auto porder = manager.NewOrder(order);

    size_t id = porder->Details().internalOrderId;
    auto gorder = manager.GetOrder(id+1);

    if (gorder != nullptr) {
        log << "Invalid order ID did not return a nullptr! " << endl;
        return 1;
    }

    return 0;
}

int ValidOrderId(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    Exchanges exchanges(api,cfg.VenuesConfig());
    OrderManager manager(api,event_loop,cfg,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    auto porder = manager.NewOrder(order);
    auto porder2 = manager.NewOrder(order);

    size_t id = porder->Details().internalOrderId;
    auto gorder = manager.GetOrder(id);

    if ( gorder == nullptr ) {
        log << "Failed to find a valid order!" << endl;
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,*gorder,id)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    porder->SendOrder();
    while (event_loop.RunNext()) { } 

    size_t eid = porder->Details().exchangeOrderId;

    if ( !ValidateOrderDetails(log,order,*gorder,1,eid)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if (!ValidateAcceptedOrder(log, *gorder, eid)){
        log << "Failed to validate order status!" << endl;
        return 1;
    }

    return 0;
}
