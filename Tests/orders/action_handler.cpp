#include <LevelConfig.h>
#include <tester.h>
#include <DummyEventLoop.h>
#include <StockFighterAPIMock.h>
#include <ExchangeOrder.h>
#include <test_checkers.h>
#include <OrderActionHandler.h>

using namespace TestChckers;

int DefaultCreateValidOrder(testLogger& log);
int ValidatorsCreateValid(testLogger& log);
int InvalidExchange(testLogger& log);
int ValidatorFail(testLogger& log);

int  main(int argc, char**argv) {
    Test("Create a new valid order...", DefaultCreateValidOrder).RunTest();
    Test("Create a new order, which passes validation...", ValidatorsCreateValid).RunTest();
    Test("Creating an order for a non-existent exchange returns nullptr",InvalidExchange).RunTest();
    Test("Validator failure results in rejected order",ValidatorFail).RunTest();
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

int DefaultCreateValidOrder(testLogger& log) {
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
    OrderActionHandler actionHandler(manager);


    std::string error;
    auto porder = actionHandler.NewOrder(order,error);

    if ( porder == nullptr ) {
        log << "valid order returned a nullptr!" << endl;
        return 1;
    }

    auto client = porder->NewUpdatesClient();

    if ( !ValidateOrderDetails(log,order,*porder,1)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if ( !ValidateSentOrder(log,*porder)) {
        return 1;
    }

    if ( !ValidateUpdatesClientEmpty(log,client)) {
        return 1;
    }

    return 0;
}

int ValidatorsCreateValid(testLogger& log) {
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
    OrderActionHandler actionHandler(manager);

    OrderActionHandler::ValidatorHdl basic(
        new BasicExchangeOrderValidator(cfg,exchanges));

    class BuyOnlyValidator: public ExchangeOrderValidator {
    public:
        bool ValidateDraftOrder(const ExchangeOrder& order,
                                std::string& error) const
        {
            bool ok = true;
            if (order.Details().direction != OrderDefinition::DIRECTION_BUY) {
                error = "Only buy orders allowed";
                ok = false;
            }
            return ok;
        }
    };
    OrderActionHandler::ValidatorHdl buy_only(new BuyOnlyValidator);
    actionHandler.InstallValidator(basic);
    actionHandler.InstallValidator(buy_only);

    std::string error;
    auto porder = actionHandler.NewOrder(order,error);

    if ( porder == nullptr ) {
        log << "valid order returned a nullptr!" << endl;
        return 1;
    }

    auto client = porder->NewUpdatesClient();

    if ( !ValidateOrderDetails(log,order,*porder,1)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if ( !ValidateSentOrder(log,*porder)) {
        return 1;
    }

    if ( !ValidateUpdatesClientEmpty(log,client)) {
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

    Exchanges exchanges(api,cfg.VenuesConfig());
    OrderManager manager(api,event_loop,cfg,exchanges);
    OrderActionHandler actionHandler(manager);

    std::string error;
    auto porder = actionHandler.NewOrder(order,error);

    if ( porder != nullptr ) {
        log << "Invalid exchange did not return nullptr" << endl;
        return 1;
    }

    const std::string reason = "Account 'GB86011564' may not trade 'FOOBAR' on 'NOTEX'";
    if ( error != reason) {
        log << "Invalid error msg" << endl;
        log.ReportStringDiff(reason,error);
        return 1;
    }

    return 0;
}

int ValidatorFail(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction.SetValue("sell");
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    Exchanges exchanges(api,cfg.VenuesConfig());
    OrderManager manager(api,event_loop,cfg,exchanges);
    OrderActionHandler actionHandler(manager);
    OrderActionHandler::ValidatorHdl basic(
        new BasicExchangeOrderValidator(cfg,exchanges));

    class BuyOnlyValidator: public ExchangeOrderValidator {
    public:
        bool ValidateDraftOrder(const ExchangeOrder& order,
                                std::string& error) const
        {
            bool ok = true;
            if (order.Details().direction != OrderDefinition::DIRECTION_BUY) {
                error = "Only buy orders allowed";
                ok = false;
            }
            return ok;
        }
    };
    OrderActionHandler::ValidatorHdl buy_only(new BuyOnlyValidator);
    actionHandler.InstallValidator(basic);
    actionHandler.InstallValidator(buy_only);

    std::string error;
    auto porder = actionHandler.NewOrder(order,error);

    if ( porder == nullptr ) {
        log << "Invalid direction resulted in nullptr" << endl;
        return 1;
    }

    const std::string reason = "Only buy orders allowed";
    if ( error != reason) {
        log << "Invalid error msg" << endl;
        log.ReportStringDiff(reason,error);
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,*porder,1,0,reason)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if ( !ValidateSystemRejectedOrder(log,*porder,reason)) {
        return 1;
    }

    return 0;
}
