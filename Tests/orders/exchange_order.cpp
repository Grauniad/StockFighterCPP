#include <LevelConfig.h>
#include <tester.h>
#include <DummyEventLoop.h>
#include <StockFighterAPIMock.h>
#include <ExchangeOrder.h>
#include <test_checkers.h>

using namespace TestChckers;

int DraftOrder(testLogger& log);
int SentOrder(testLogger& log);
int AcceptedOrder(testLogger& log);
int RejectedOrder(testLogger& log);
int SystemReject(testLogger& log);
int SystemRejectedNoSend(testLogger& log);
int SentOrderNoReject(testLogger& log);

int  main(int argc, char**argv) {
    Test("Create a new draft order", DraftOrder).RunTest();
    Test("Create a sent state order", SentOrder).RunTest();
    Test("Create an accepted order.", AcceptedOrder).RunTest();
    Test("Create a rejected order.", RejectedOrder).RunTest();
    Test("Create a system rejected order.", SystemReject).RunTest();
    Test("An order created by the system cannot be sent", SystemRejectedNoSend).RunTest();
    Test("An order already on the exchange cannot be rejected by the system", SentOrderNoReject).RunTest();
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

int DraftOrder(testLogger& log) {
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

    ExchangeOrder e_order(api,event_loop,1,order);
    auto client = e_order.NewUpdatesClient();

    if ( !ValidateOrderDetails(log,order,e_order,1)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if ( !ValidateDraftOrder(log,e_order)) {
        return 1;
    }

    if ( !ValidateUpdatesClientEmpty(log,client)) {
        return 1;
    }

    return 0;
}

int SystemReject(testLogger& log) {
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

    ExchangeOrder e_order(api,event_loop,1,order);
    auto client = e_order.NewUpdatesClient();

    const std::string reason = "The system didn't like this order.";

    e_order.SystemReject(reason);

    if ( !ValidateOrderDetails(log,order,e_order,1,0,reason)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if ( !ValidateSystemRejectedOrder(log,e_order,reason)) {
        return 1;
    }

    OrderStatus::OrderState oldState = OrderStatus::ORDER_STATUS_DRAFT;
    OrderStatus::OrderState newState = OrderStatus::ORDER_STATUS_SYSTEM_REJECTED;
    size_t qty = 1;
    if ( !ValidateUnfilledOrderUpdate(log,client,oldState,newState,qty,1, 0)) {
        log << "Failed to find rejected update!" << endl;
        return 1;
    }

    if ( !ValidateUpdatesClientEmpty(log,client)) {
        return 1;
    }

    return 0;
}

int SystemRejectedNoSend(testLogger& log) {
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

    ExchangeOrder e_order(api,event_loop,1,order);
    auto client = e_order.NewUpdatesClient();

    const std::string reason = "The system didn't like this order.";

    e_order.SystemReject(reason);

    if ( e_order.SendOrder() ) {
        log << "Send system rejected order to the exchange!" << endl;
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,e_order,1,0,reason)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if ( !ValidateSystemRejectedOrder(log,e_order,reason)) {
        return 1;
    }

    OrderStatus::OrderState oldState = OrderStatus::ORDER_STATUS_DRAFT;
    OrderStatus::OrderState newState = OrderStatus::ORDER_STATUS_SYSTEM_REJECTED;
    size_t qty = 1;
    if ( !ValidateUnfilledOrderUpdate(log,client,oldState,newState,qty,1, 0)) {
        log << "Failed to find rejected update!" << endl;
        return 1;
    }

    if ( !ValidateUpdatesClientEmpty(log,client)) {
        return 1;
    }

    return 0;
}

int SentOrder(testLogger& log) {
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

    ExchangeOrder e_order(api,event_loop,1,order);
    auto client = e_order.NewUpdatesClient();

    if ( !e_order.SendOrder() ) {
        log << "Failed to send order to the exchange!" << endl;
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,e_order,1)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if ( !ValidateSentOrder(log,e_order)) {
        return 1;
    }

    OrderStatus::OrderState oldState = OrderStatus::ORDER_STATUS_DRAFT;
    OrderStatus::OrderState newState = OrderStatus::ORDER_STATUS_SENT_TO_EXCHANGE;
    long qty = order.originalQuantity;
    size_t internal_id = api.LastAllocatedId();
    if ( !ValidateUnfilledOrderUpdate(log,client,oldState,newState,qty,internal_id)) {
        return 1;
    }

    if ( !ValidateUpdatesClientEmpty(log,client)) {
        return 1;
    }

    return 0;
}

int SentOrderNoReject(testLogger& log) {
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

    ExchangeOrder e_order(api,event_loop,1,order);
    auto client = e_order.NewUpdatesClient();

    if ( !e_order.SendOrder() ) {
        log << "Failed to send order to the exchange!" << endl;
        return 1;
    }

    if ( e_order.SystemReject("System rejects...")) {
        log << "Allowed system reject of sent order!" << endl;
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,e_order,1)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if ( !ValidateSentOrder(log,e_order)) {
        return 1;
    }

    OrderStatus::OrderState oldState = OrderStatus::ORDER_STATUS_DRAFT;
    OrderStatus::OrderState newState = OrderStatus::ORDER_STATUS_SENT_TO_EXCHANGE;
    long qty = order.originalQuantity;
    size_t internal_id = api.LastAllocatedId();
    if ( !ValidateUnfilledOrderUpdate(log,client,oldState,newState,qty,internal_id)) {
        return 1;
    }

    if ( !ValidateUpdatesClientEmpty(log,client)) {
        return 1;
    }

    return 0;
}

int AcceptedOrder(testLogger& log) {
    DummyEventLoop request_loop;
    DummyEventLoop io_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMockBase api(&io_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    ExchangeOrder e_order(api,request_loop,1,order);
    auto client = e_order.NewUpdatesClient();

    if ( !e_order.SendOrder() ) {
        log << "Failed to send order to the exchange!" << endl;
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,e_order,1)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if ( !ValidateSentOrder(log,e_order)) {
        return 1;
    }

    // Poll the IO "thread" to handle the response
    if (!io_loop.RunNext()) {
        log << "No IO event to handle!" << endl;
        return 1;
    }

    OrderStatus::OrderState oldState = OrderStatus::ORDER_STATUS_DRAFT;
    OrderStatus::OrderState newState = OrderStatus::ORDER_STATUS_SENT_TO_EXCHANGE;
    long qty = order.originalQuantity;
    size_t internal_id = api.LastAllocatedId();
    if ( !ValidateUnfilledOrderUpdate(log,client,oldState,newState,qty,internal_id)) {
        log << "Failed to find sent update" << endl;
        return 1;
    }

    // But we shouldn't be ready yet, since the request thread has not run
    // yet...
    if ( !ValidateUpdatesClientEmpty(log,client)) {
        log << "Not empty after send!" << endl;
        return 1;
    }

    if ( !ValidateSentOrder(log,e_order)) {
        return 1;
    }

    if (io_loop.RunNext()) {
        log << "Additional queued IO loop!" << endl;
        return 1;
    }

    // Now run the request thread - which should pick up the queued update
    if (!request_loop.RunNext()) {
        log << "No request event to handle!" << endl;
        return 1;
    }

    // Having handled the update we should have moved to an accepted state...
    if ( !ValidateAcceptedOrder(log,e_order,api.LastAllocatedId())) {
        return 1;
    }

    if (request_loop.RunNext()) {
        log << "Additional event on the request loop!" << endl;
        return 1;
    }
    oldState = OrderStatus::ORDER_STATUS_SENT_TO_EXCHANGE;
    newState = OrderStatus::ORDER_STATUS_ON_EXCHANGE;
    size_t external_id = api.LastAllocatedId();
    if ( !ValidateUnfilledOrderUpdate(log,client,oldState,newState,qty,internal_id, external_id)) {
        log << "Failed to find accepted update!" << endl;
        return 1;
    }

    if ( !ValidateUpdatesClientEmpty(log,client)) {
        log << "Not empty after accept!" << endl;
        return 1;
    }

    return 0;
}

int RejectedOrder(testLogger& log) {
    DummyEventLoop request_loop;
    DummyEventLoop io_loop;
    LevelConfig cfg(levelConfig);
    StockFighterAPIMockBase api(&io_loop);

    OrderDefinition order;
    order.account = "GB86011564";
    order.symbol = "FOOBAR";
    order.venue= "TESTEX";
    order.direction = OrderDefinition::DIRECTION_BUY;
    order.type = OrderDefinition::ORDER_TYPE_LIMIT;
    order.originalQuantity = 1;
    order.limitPrice = 1;

    api.SetNextOrderResponse(R"(
        {
            "ok": false,
            "error": "A descriptive error message telling you that the order you attempted to place was invalid and not processed by the stock exchange."
        }
    )");
    const std::string reason = "A descriptive error message telling you that the order you attempted to place was invalid and not processed by the stock exchange.";

    ExchangeOrder e_order(api,request_loop,1,order);
    auto client = e_order.NewUpdatesClient();

    if ( !e_order.SendOrder() ) {
        log << "Failed to send order to the exchange!" << endl;
        return 1;
    }

    if ( !ValidateOrderDetails(log,order,e_order,1)) {
        log << "Failed to validate order details!" << endl;
        return 1;
    }

    if ( !ValidateSentOrder(log,e_order)) {
        return 1;
    }

    // Poll the IO "thread" to handle the response
    if (!io_loop.RunNext()) {
        log << "No IO event to handle!" << endl;
        return 1;
    }

    OrderStatus::OrderState oldState = OrderStatus::ORDER_STATUS_DRAFT;
    OrderStatus::OrderState newState = OrderStatus::ORDER_STATUS_SENT_TO_EXCHANGE;
    long qty = order.originalQuantity;
    if ( !ValidateUnfilledOrderUpdate(log,client,oldState,newState,qty,1)) {
        log << "Failed to find sent update" << endl;
        return 1;
    }

    // But we shouldn't be ready yet, since the request thread has not run
    // yet...
    if ( !ValidateUpdatesClientEmpty(log,client)) {
        log << "Not empty after send!" << endl;
        return 1;
    }
    if ( !ValidateSentOrder(log,e_order)) {
        return 1;
    }

    if (io_loop.RunNext()) {
        log << "Additional queued IO loop!" << endl;
        return 1;
    }

    // Now run the request thread - which should pick up the queued update
    if (!request_loop.RunNext()) {
        log << "No request event to handle!" << endl;
        return 1;
    }

    // Having handled the update we should have moved to an accepted state...
    if ( !ValidateRejectedOrder(log,e_order,reason)) {
        return 1;
    }

    if (request_loop.RunNext()) {
        log << "Additional event on the request loop!" << endl;
        return 1;
    }

    oldState = OrderStatus::ORDER_STATUS_SENT_TO_EXCHANGE;
    newState = OrderStatus::ORDER_STATUS_EXCHANGE_REJECTED;
    size_t external_id = api.LastAllocatedId();
    if ( !ValidateUnfilledOrderUpdate(log,client,oldState,newState,qty,1, external_id)) {
        log << "Failed to find rejected update!" << endl;
        return 1;
    }

    if ( !ValidateUpdatesClientEmpty(log,client)) {
        log << "Not empty after reject!" << endl;
        return 1;
    }

    return 0;
}
