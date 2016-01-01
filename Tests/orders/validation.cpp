#include <tester.h>
#include <OrderValidation.h>
#include <test_checkers.h>
#include <DummyEventLoop.h>
#include <StockFighterAPIMock.h>
#include <LevelConfig.h>

int InvalidAccount(testLogger& log);
int InvalidVenue(testLogger& log);
int InvalidStock(testLogger& log);
int InvalidOrderType(testLogger& log);
int InvalidDirection(testLogger& log);
int InvalidQty(testLogger& log);
int InvalidLimitPrice(testLogger& log);
int MarketOrderLimitPrice(testLogger& log);
int ValidMarketOrder(testLogger& log);
int ValidLimitOrder(testLogger& log);

int main(int argc, const char *argv[])
{
    Test("Order with an invalid account does not validate",InvalidAccount).RunTest();
    Test("Order with an invalid exchange does not validate",InvalidVenue).RunTest();
    Test("Order with an invalid stock does not validate",InvalidStock).RunTest();
    Test("Order with an invalid type does not validate",InvalidOrderType).RunTest();
    Test("Order with an invalid direction does not validate",InvalidDirection).RunTest();
    Test("Order with an invalid quantity does not validate",InvalidQty).RunTest();
    Test("Order with an invalid limit price does not validate",InvalidLimitPrice).RunTest();
    Test("Market order may not have limit price",MarketOrderLimitPrice).RunTest();
    Test("Market order may may be created if it does not have a limit price",ValidMarketOrder).RunTest();
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

int InvalidAccount(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);
    Exchanges exchanges(api,cfg.VenuesConfig());
    BasicExchangeOrderValidator validator(cfg,exchanges);

    OrderDefinition order;
    order.account = "NOTACC";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    ExchangeOrder e_order(api,event_loop,1,order);

    std::string error;

    if ( validator.ValidateDraftOrder(e_order,error)) {
        log << "Invalid account validated!" << endl;
        return 1;
    }

    if ( error != "Invalid account 'NOTACC' supplied." ) {
        log << "Incorrect error message returned" << endl;
        log.ReportStringDiff("Invalid account 'NOTACC' supplied.",error);
        return 1;
    }

    return 0;
}

int InvalidVenue(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);
    Exchanges exchanges(api,cfg.VenuesConfig());
    BasicExchangeOrderValidator validator(cfg,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "NOTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    ExchangeOrder e_order(api,event_loop,1,order);

    std::string error;
    const std::string msg = "Invalid exchange 'NOTEX' supplied.";

    if ( validator.ValidateDraftOrder(e_order,error)) {
        log << "Invalid exchange validated!" << endl;
        return 1;
    }

    if ( error !=  msg ) {
        log << "Incorrect error message returned" << endl;
        log.ReportStringDiff(msg,error);
        return 1;
    }

    return 0;
}

int InvalidStock(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);
    Exchanges exchanges(api,cfg.VenuesConfig());
    BasicExchangeOrderValidator validator(cfg,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "NOTS";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    ExchangeOrder e_order(api,event_loop,1,order);

    std::string error;
    const std::string msg = "Invalid stock 'NOTS' supplied.";

    if ( validator.ValidateDraftOrder(e_order,error)) {
        log << "Invalid stock validated!" << endl;
        return 1;
    }

    if ( error !=  msg ) {
        log << "Incorrect error message returned" << endl;
        log.ReportStringDiff(msg,error);
        return 1;
    }

    return 0;
}

int InvalidOrderType(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);
    Exchanges exchanges(api,cfg.VenuesConfig());
    BasicExchangeOrderValidator validator(cfg,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction.SetValue("buy");
    order.type.SetValue("NOTTYPE");
    order.originalQuantity = 1;
    order.limitPrice = 1;

    ExchangeOrder e_order(api,event_loop,1,order);

    std::string error;
    const std::string msg = "Invalid order type supplied.";

    if ( validator.ValidateDraftOrder(e_order,error)) {
        log << "Invalid order type validated!" << endl;
        return 1;
    }

    if ( error !=  msg ) {
        log << "Incorrect error message returned" << endl;
        log.ReportStringDiff(msg,error);
        return 1;
    }

    return 0;
}

int InvalidDirection(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);
    Exchanges exchanges(api,cfg.VenuesConfig());
    BasicExchangeOrderValidator validator(cfg,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction.SetValue("NOTDIR");
    order.type.SetValue("limit");
    order.originalQuantity = 1; order.limitPrice = 1;

    ExchangeOrder e_order(api,event_loop,1,order);

    std::string error;
    const std::string msg = "Invalid direction supplied.";

    if ( validator.ValidateDraftOrder(e_order,error)) {
        log << "Invalid direction validated!" << endl;
        return 1;
    }

    if ( error !=  msg ) {
        log << "Incorrect error message returned" << endl;
        log.ReportStringDiff(msg,error);
        return 1;
    }

    return 0;
}

int InvalidQty(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);
    Exchanges exchanges(api,cfg.VenuesConfig());
    BasicExchangeOrderValidator validator(cfg,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction.SetValue("sell");
    order.type.SetValue("limit");
    order.originalQuantity = 0;
    order.limitPrice = 1;

    ExchangeOrder e_order(api,event_loop,1,order);

    std::string error;
    const std::string msg = "Invalid order quantity (0) supplied.";

    if ( validator.ValidateDraftOrder(e_order,error)) {
        log << "Invalid quantity validated!" << endl;
        return 1;
    }

    if ( error !=  msg ) {
        log << "Incorrect error message returned" << endl;
        log.ReportStringDiff(msg,error);
        return 1;
    }

    return 0;
}

int InvalidLimitPrice(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);
    Exchanges exchanges(api,cfg.VenuesConfig());
    BasicExchangeOrderValidator validator(cfg,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction.SetValue("sell");
    order.type.SetValue("limit");
    order.originalQuantity = 1;
    order.limitPrice = -1;

    ExchangeOrder e_order(api,event_loop,1,order);

    std::string error;
    const std::string msg = "Invalid limit price (-0.01) supplied.";

    if ( validator.ValidateDraftOrder(e_order,error)) {
        log << "Invalid limit price validated!" << endl;
        return 1;
    }

    if ( error !=  msg ) {
        log << "Incorrect error message returned" << endl;
        log.ReportStringDiff(msg,error);
        return 1;
    }

    return 0;
}

int MarketOrderLimitPrice(testLogger& log ) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);
    Exchanges exchanges(api,cfg.VenuesConfig());
    BasicExchangeOrderValidator validator(cfg,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction.SetValue("sell");
    order.type.SetValue("market");
    order.originalQuantity = 1;
    order.limitPrice = 12;

    ExchangeOrder e_order(api,event_loop,1,order);

    std::string error;
    const std::string msg = "Limit price supplied for market order.";

    if ( validator.ValidateDraftOrder(e_order,error)) {
        log << "Invalid limit price (market) validated!" << endl;
        return 1;
    }

    if ( error !=  msg ) {
        log << "Incorrect error message returned" << endl;
        log.ReportStringDiff(msg,error);
        return 1;
    }

    return 0;
}

int ValidMarketOrder(testLogger& log ) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);
    Exchanges exchanges(api,cfg.VenuesConfig());
    BasicExchangeOrderValidator validator(cfg,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction.SetValue("sell");
    order.type.SetValue("market");
    order.originalQuantity = 1;

    ExchangeOrder e_order(api,event_loop,1,order);

    std::string error;
    const std::string msg = "";

    if ( !validator.ValidateDraftOrder(e_order,error)) {
        log << "Valid market order did not validate!" << endl;
        return 1;
    }

    if ( error !=  msg ) {
        log << "Incorrect error message returned" << endl;
        log.ReportStringDiff(msg,error);
        return 1;
    }

    return 0;
}

int ValidLimitOrder(testLogger& log ) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);
    Exchanges exchanges(api,cfg.VenuesConfig());
    BasicExchangeOrderValidator validator(cfg,exchanges);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction.SetValue("sell");
    order.type.SetValue("limit");
    order.originalQuantity = 1;
    order.limitPrice = 1;

    ExchangeOrder e_order(api,event_loop,1,order);

    std::string error;
    const std::string msg = "";

    if ( !validator.ValidateDraftOrder(e_order,error)) {
        log << "Valid limit order did not validate!" << endl;
        return 1;
    }

    if ( error !=  msg ) {
        log << "Incorrect error message returned" << endl;
        log.ReportStringDiff(msg,error);
        return 1;
    }

    return 0;
}
