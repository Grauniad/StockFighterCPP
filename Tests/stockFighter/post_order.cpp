#include <QuoteSubscription.h>
#include <LevelConfig.h>
#include <tester.h>
#include <DummyEventLoop.h>
#include <StockFighterAPIMock.h>
#include <atomic>
#include <thread>

int LimitOrderMessaage(testLogger& log);
int FOKOrderMessaage(testLogger& log);
int IOCOrderMessaage(testLogger& log);
int MarketOrderMessaage(testLogger& log);
int SellOrder(testLogger& log);
int BuyOrder(testLogger& log);
int NoWaitWhenReady(testLogger& log);
int WaitForResponse(testLogger& log);
int NotifyResponse(testLogger& log);
int OrderRejected(testLogger& log);

int  main(int argc, char**argv) {
    Test("Validating message sent for limit order.", LimitOrderMessaage).RunTest();
    Test("Validating message sent for market order.", MarketOrderMessaage).RunTest();
    Test("Validating message sent for fill or kill order.", FOKOrderMessaage).RunTest();
    Test("Validating message sent for immediate or cancel order.", IOCOrderMessaage).RunTest();
    Test("Validating message sent for buy order", BuyOrder).RunTest();
    Test("Validating message sent for sell order", SellOrder).RunTest();
    Test("Validating no wait if response already available", NoWaitWhenReady).RunTest();
    Test("Validating wait if response is not yet available", WaitForResponse).RunTest();
    Test("Validating response notification", NotifyResponse).RunTest();
    Test("Validating response for rejected order", OrderRejected).RunTest();
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
                "Name": " XXEU Beijing Exchange",
                "Code": "XXEUEX"
            }
        ]
}
)";

bool ValidatePostOrderCall(testLogger& log,
                           const OrderDefinition& order,
                           const StockFighterAPIMockBase::PostOrderCall& details)
{
    bool ok = true;

    if (order.venue != details.venue)  {
        log << "Invalid venue on post order!" << endl;
        log.ReportStringDiff(order.venue,details.venue);
        ok = false;
    }

    if (order.symbol != details.symbol)  {
        log << "Invalid stock on post order!" << endl;
        log.ReportStringDiff(order.symbol,details.symbol);
        ok = false;
    }

    NewStringField(account);
    NewStringField(direction);
    NewStringField(orderType);
    NewUIntField(price);
    NewUIntField(qty);
    NewStringField(stock);
    NewStringField(venue);

    typedef SimpleParsedJSON<
        account,
        direction,
        orderType,
        price,
        qty,
        stock,
        venue
    > Request;

    std::string error;
    Request request;

    if ( !request.Parse(details.order.c_str(),error)) {
        log << "Failed to parse order post request: " << error << endl;
        ok = false;
    } else {
        log << ">> " << details.order << endl;
        log << "<< " << request.GetPrettyJSONString() << endl;
    }

    if ( order.account != request.Get<account>()) {
        log << "Account missmatch" << endl;
        ok = false;
    }

    std::string expDir = "";
    if ( order.direction == order.DIRECTION_BUY ) {
        expDir = "buy";
    } else if (order.direction == order.DIRECTION_SELL) {
        expDir = "sell";
    }

    if ( expDir != request.Get<direction>()) {
        log << "Direction missmatch!" << endl;
        log.ReportStringDiff(expDir,request.Get<direction>());
        ok = false;
    }

    std::string expType = "";

    if ( order.type == order.ORDER_TYPE_LIMIT ) {
        expType = "limit";
    } else if  (order.type == order.ORDER_TYPE_MARKET ) {
        expType = "market";
    } else if  (order.type == order.ORDER_TYPE_IMMEDIATE_OR_CANACEL ) {
        expType = "immediate-or-cancel";
    } else if  (order.type == order.ORDER_TYPE_FILL_OR_KILl ) {
        expType = "fill-or-kill";
    }

    if ( expType != request.Get<orderType>()) {
        log << "Order type missmatch!" << endl;
        log.ReportStringDiff(expType,request.Get<orderType>());
        ok = false;
    }

    if ( order.limitPrice != request.Get<price>() ) {
        log << "Price missmatch! " << endl;
        ok = false;
    }

    if ( order.originalQuantity != request.Get<qty>() ) {
        log << "Qty missmatch! " << endl;
        ok = false;
    }

    if ( order.symbol != request.Get<stock>()) {
        log << "Stock missmatch!" << endl;
        log.ReportStringDiff(order.symbol,request.Get<stock>());
        ok = false;
    }

    if ( order.venue != request.Get<venue>()) {
        log << "Stock missmatch!" << endl;
        log.ReportStringDiff(order.venue,request.Get<venue>());
        ok = false;
    }

    return ok;
}

bool ValidateOrderResponseDetails(
         testLogger& log,
         StockFighterAPIMockBase& api,
         OrderPostRequest::OrderResponse& response)
{
    bool ok = true;

    if (response.ok != true ) {
        log << "Order response was not ok!" << endl; 
        ok = false;
    }

    if (response.error != "" ) {
        log << "Error was not blank: " << response.error << endl; 
        ok = false;
    }

    if (response.exchangeOrderId != api.LastAllocatedId() ) {
        log << "Exchange ID miss match!" << endl;
        log << "Expected: " << api.LastAllocatedId() << endl;
        log << "Actual  : " << response.exchangeOrderId << endl;
        ok = false;
    }

    return ok;
}

bool ValidateRejectedOrderResponseDetails(
         testLogger& log,
         OrderPostRequest::OrderResponse& response,
         const std::string& reason)
{
    bool ok = true;

    if (response.ok != false ) {
        log << "Order response was ok!" << endl; 
        ok = false;
    }

    if (response.error != reason ) {
        log << "Invalid error received!" << endl;
        log.ReportStringDiff(reason,response.error);
        ok = false;
    }

    if (response.exchangeOrderId != 0 ) {
        log << "Exchange ID miss match!" << endl;
        log << "Expected: " << 0 << endl;
        log << "Actual  : " << response.exchangeOrderId << endl;
        ok = false;
    }

    return ok;
}

bool ValidateOrderResponse(testLogger& log,
                           StockFighterAPIMockBase& api,
                           DummyEventLoop&          event_loop,
                           OrderPostReqHdl&         hdl)
{
    bool ok = true;

    if ( !event_loop.RunNext() ) {
        log << "No event queued to run on the event loop!" << endl;
        ok = false;
    }

    auto response = hdl->WaitForResponse();

    ok &= ValidateOrderResponseDetails(log,api,response);
    return ok;
}

int LimitOrderMessaage(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMockBase api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    auto hdl = api.PostOrder(order);

    auto details = api.LastCallTo_PostOrder();

    if ( !ValidatePostOrderCall(log,order,details)) {
        return 1;
    }

    return 0;
}

int MarketOrderMessaage(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMockBase api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_MARKET;
    order.originalQuantity = 1;
    order.limitPrice = 0;

    auto hdl = api.PostOrder(order);

    auto details = api.LastCallTo_PostOrder();

    if ( !ValidatePostOrderCall(log,order,details)) {
        return 1;
    }

    return 0;
}

int IOCOrderMessaage(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMockBase api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_IMMEDIATE_OR_CANACEL;
    order.originalQuantity = 1;
    order.limitPrice = 2;

    auto hdl = api.PostOrder(order);

    auto details = api.LastCallTo_PostOrder();

    if ( !ValidatePostOrderCall(log,order,details)) {
        return 1;
    }

    return 0;
}

int FOKOrderMessaage(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMockBase api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_FILL_OR_KILl;
    order.originalQuantity = 1;
    order.limitPrice = 2;

    auto hdl = api.PostOrder(order);

    auto details = api.LastCallTo_PostOrder();

    if ( !ValidatePostOrderCall(log,order,details)) {
        return 1;
    }

    return 0;
}

int BuyOrder(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMockBase api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_FILL_OR_KILl;
    order.originalQuantity = 1;
    order.limitPrice = 2;

    auto hdl = api.PostOrder(order);

    auto details = api.LastCallTo_PostOrder();

    if ( !ValidatePostOrderCall(log,order,details)) {
        return 1;
    }

    return 0;
}

int SellOrder(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMockBase api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_SELL;
    order.type = OrderDefinition::ORDER_TYPE_FILL_OR_KILl;
    order.originalQuantity = 1;
    order.limitPrice = 2;

    auto hdl = api.PostOrder(order);

    auto details = api.LastCallTo_PostOrder();

    if ( !ValidatePostOrderCall(log,order,details)) {
        return 1;
    }

    return 0;
}

int NoWaitWhenReady(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMockBase api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_SELL;
    order.type = OrderDefinition::ORDER_TYPE_FILL_OR_KILl;
    order.originalQuantity = 1;
    order.limitPrice = 2;

    auto hdl = api.PostOrder(order);

    auto details = api.LastCallTo_PostOrder();

    if ( !ValidateOrderResponse(log,api,event_loop,hdl)) {
        return 1;
    }
    return 0;
}

int WaitForResponse(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMockBase api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_SELL;
    order.type = OrderDefinition::ORDER_TYPE_FILL_OR_KILl;
    order.originalQuantity = 1;
    order.limitPrice = 2;

    auto hdl = api.PostOrder(order);

    std::atomic<bool> ready;
    ready = false;
    OrderPostRequest::OrderResponse response;

    auto wait = [&hdl,&ready,&response] () -> void {
        response = hdl->WaitForResponse();
        ready = true;
    };

    std::thread waiter(wait);

    std::this_thread::yield();

    if ( ready ) {
        log << "Wait has returned before response is ready!" << endl;
        event_loop.RunNext();
        return 1;
    }

    event_loop.RunNext();

    waiter.join();

    if ( !ready) {
        log << "Waited for response, but we're not ready!" << endl;
        return 1;
    }

    if ( !ValidateOrderResponseDetails(log,api,response)) {
        return 1;
    }
    return 0;
}

int NotifyResponse(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMockBase api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_SELL;
    order.type = OrderDefinition::ORDER_TYPE_FILL_OR_KILl;
    order.originalQuantity = 1;
    order.limitPrice = 2;

    auto hdl = api.PostOrder(order);

    std::atomic<bool> ready;
    ready = false;
    OrderPostRequest::OrderResponse response;

    auto onReady = [&hdl,&ready,&response] (const OrderPostRequest::OrderResponse& src) -> void {
        response = src;
        ready = true;
    };

    hdl->OnResponse(onReady);

    if ( ready ) {
        log << "Wait has returned before response is ready!" << endl;
        event_loop.RunNext();
        return 1;
    }

    event_loop.RunNext();

    if ( !ready) {
        log << "Waited for response, but we're not ready!" << endl;
        return 1;
    }

    if ( !ValidateOrderResponseDetails(log,api,response)) {
        return 1;
    }
    return 0;
}

int OrderRejected(testLogger& log) {
    DummyEventLoop event_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMockBase api(&event_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_SELL;
    order.type = OrderDefinition::ORDER_TYPE_FILL_OR_KILl;
    order.originalQuantity = 1;
    order.limitPrice = 2;

    api.SetNextOrderResponse(R"(
        {
            "ok": false,
            "error": "A descriptive error message telling you that the order you attempted to place was invalid and not processed by the stock exchange."
        }
    )");

    auto hdl = api.PostOrder(order);

    event_loop.RunNext();
    auto response = hdl->WaitForResponse();

    const std::string reason = "A descriptive error message telling you that the order you attempted to place was invalid and not processed by the stock exchange.";

    if ( !ValidateRejectedOrderResponseDetails(log,response,reason)) {
        return 1;
    }

    return 0;
}
