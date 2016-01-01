#include <tester.h>
#include <DummyEventLoop.h>
#include <StockFighterAPIMock.h>
#include <ExchangeOrder.h>
#include <test_checkers.h>
#include <OrderActionHandler.h>
#include <ReqNewOrder.h>
#include <json_tests.h>

using namespace JSONCheckers;

int InvalidOrderRejected(testLogger& log);
int InvalidStock(testLogger& log);
int ValidOrder(testLogger& log);

int main(int argc, const char *argv[])
{
    Test("Rejected order returns error text",InvalidOrderRejected).RunTest();
    Test("Invalid stock returns error text",InvalidStock).RunTest();
    Test("Valid order returns the correct response",ValidOrder).RunTest();
    return 0;
}

class RejectAll: public ExchangeOrderValidator {
public:
    bool ValidateDraftOrder(const ExchangeOrder& order,
                            std::string& error) const
    {
        error = "No orders today!";
        return false;
    }
};

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


int InvalidOrderRejected(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    Exchanges exchanges(api,cfg.VenuesConfig());
    OrderManager manager(api,event_loop,cfg,exchanges);

    OrderActionHandler actionHandler(manager);
    OrderActionHandler::ValidatorHdl rejectAll(new RejectAll);
    actionHandler.InstallValidator(rejectAll);

    std::string request = R"({
        "account":      "GB86011564",
        "exchange":     "TESTEX",
        "stock":        "FOOBAR",
        "type":         "limit",
        "direction":    "buy",
        "quantity":     100,
        "limit_price":  100
    })";

    ReqNewOrder handler(actionHandler);

    std::string error = "No orders today!";

    if (!CheckRequestRejected(log,request,handler,error,0)) {
        return 1;
    }

    return 0;
}

int InvalidStock(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    Exchanges exchanges(api,cfg.VenuesConfig());
    OrderManager manager(api,event_loop,cfg,exchanges);

    OrderActionHandler actionHandler(manager);

    std::string request = R"({
        "account":      "GB86011564",
        "exchange":     "NOTEX",
        "stock":        "FOOBAR",
        "type":         "limit",
        "direction":    "buy",
        "quantity":     100,
        "limit_price":  100
    })";

    ReqNewOrder handler(actionHandler);

    std::string error = "Account 'GB86011564' may not trade 'FOOBAR' on 'NOTEX'";

    if (!CheckRequestRejected(log,request,handler,error,0)) {
        return 1;
    }

    return 0;
}
NewI64Field(order_id);
NewBoolField(ok);
typedef SimpleParsedJSON<ok,order_id> Reply;

int ValidOrder(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMock api(&event_loop);

    Exchanges exchanges(api,cfg.VenuesConfig());
    OrderManager manager(api,event_loop,cfg,exchanges);

    OrderActionHandler actionHandler(manager);

    std::string request = R"({
        "account":      "GB86011564",
        "exchange":     "TESTEX",
        "stock":        "FOOBAR",
        "type":         "limit",
        "direction":    "buy",
        "quantity":     100,
        "limit_price":  100
    })";

    ReqNewOrder handler(actionHandler);

    Reply parser, expected;
    expected.Get<ok>() = true;
    expected.Get<order_id>() = 1;

    if (!CheckRequest<ok,order_id>(log,request,handler,parser,expected)) {
        return 1;
    }
    return 0;
}
