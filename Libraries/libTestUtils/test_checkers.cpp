#include "test_checkers.h"
#include <SubscriptionRequestMock.h>

bool TestChckers::QuotesEqual(testLogger& log,
                 const QuoteHandler::Quote& q1,
                 const MarketData& q2)
{
    bool ok = true;

    typedef QuoteHandler::Quote::Price QPrice;
    typedef MarketData::Price MPrice;

    auto price_check = [&log] (const std::string& priceName,
                               const QPrice& p1,
                               const MPrice& p2) -> bool {
        bool ok = true;
        if (p1.price != p2.price)
        {
            ok = false;
            log << "Price miss-match  for " << priceName << endl; 
            log << "Expected: " << p1.price << endl;
            log << "Actual  : " << p2.price << endl;
        }

        if (p1.qty != p2.qty)
        {
            ok = false;
            log << "Quantity miss-match  for " << priceName << endl; 
            log << "Expected: " << p1.qty << endl;
            log << "Actual  : " << p2.qty << endl;
        }

        return ok;
    };

    ok &= (price_check("bestBid", q1.bestBid,q2.bestBid));
    ok &= (price_check("bestAsk", q1.bestAsk,q2.bestAsk));
    ok &= (price_check("last", q1.last,q2.last));

    return ok;
}

bool TestChckers::CheckClient(testLogger& log,
                 const std::string& name,
                 MarketDataClient client, 
                 std::vector<QuoteHandler::Quote> quotes) 
{
    bool ok = true;

    MarketData q;

    size_t i = 0;
    for (QuoteHandler::Quote exp: quotes) {
        if ( !client->GetNextMessage(q) ) {
            log << endl << "Failed  to receive the quote messaage!" << endl;
            ok = false;
        } else if (!QuotesEqual(log,exp,q)) {
            log << endl << "Invalid quote received!" << endl;
            ok = false;
        } else {
            log << endl << "Messaage " << i << "matches!" << endl;
        }
        ++i;
    }

    q = client->CurrentQuote();

    if (quotes.size() > 0 ) {
        if (!QuotesEqual(log,quotes.back(),q)) {
            log << "Invalid current quote!" << endl;
            ok  = false;
        }
    }

    if ( client->GetNextMessage(q) ) {
        log << endl << "Unexpected extra message received!" << endl;
        ok = false;
    }

    if ( !ok) {
        log << "Client " << name << " did not match expected quote set" << endl;
    }

    return ok;
}

bool TestChckers::SubscriptionIsRejected(testLogger& log,
                 SubscriptionHandler& handler,
                 const std::string& request,
                 const std::string& reason,
                 const int errorCode)
{
    class Request: public SubRequestMock {
    public:
        Request(const std::string& req) : SubRequestMock(req) { }

        virtual void SendMessage(const std::string& msg) {
            throw InvalidCallToSendMessage{ }; 
        }
    };

    SubscriptionHandler::RequestHandle req(new Request(request));

    bool ok = true;
    bool raisedError = false;
    std::string caughtError = "";
    int caughtCode = 0;

    try {
        handler.OnRequest(req);
    } catch (SubscriptionHandler::InvalidRequestException& e) {
        raisedError = true;
        caughtCode = e.code;
        caughtError = e.errMsg;
    } catch (Request::InvalidCallToSendMessage& e) {
        log << "Invalid call to Send Message!" << endl;
        ok = false;
    }

    if (!raisedError) {
        log << "Subscription handler did not raise an error! " << endl;
        ok = false;
    } else {
        if ( caughtError != reason ) {
            log << "Wrong reason text thrown" << endl;
            log.ReportStringDiff(reason,caughtError);
            ok = false;
        }

        if (caughtCode != errorCode) {
            log << "Invalid error code: " << caughtCode << endl;
            log << "Expected: " << errorCode << endl;
            ok = false;
        }
    }

    if (!ok) {
        log << endl << endl 
            << "Subscription handler did not raise the correct error when parsing:" << endl
            << request << endl;
    }

    return ok;
}

bool TestChckers::ValidateOrderDetails(testLogger& log,
                          OrderDefinition& order,
                          ExchangeOrder& e_order,
                          size_t iid,
                          size_t eid,
                          const std::string& rejectReason)
{
    bool ok = true;

    if ( order.account != e_order.Details().account ) {
        log << "Invalid account!" << endl;
        log.ReportStringDiff(order.account,e_order.Details().account);
        ok = false;
    }

    if ( order.symbol != e_order.Details().symbol ) {
        log << "Invalid symbol!" << endl;
        log.ReportStringDiff(order.symbol,e_order.Details().symbol);
        ok = false;
    }

    if ( order.venue != e_order.Details().venue ) {
        log << "Invalid venue!" << endl;
        log.ReportStringDiff(order.venue,e_order.Details().venue);
        ok = false;
    }

    if ( order.direction != e_order.Details().direction ) {
        log << "Invalid direction!" << endl;
        log.ReportStringDiff(order.direction.StringValue(),e_order.Details().direction.StringValue());
        ok = false;
    }

    if ( order.type != e_order.Details().type ) {
        log << "Invalid type!" << endl;
        log.ReportStringDiff(order.type.StringValue(),e_order.Details().type.StringValue());
        ok = false;
    }

    if ( iid != e_order.Details().internalOrderId ) {
        log << "Invalid internal order id!" << endl;
        log << "Expected: " << iid << endl;
        log << "Actual  : " << e_order.Details().internalOrderId << endl;
        ok = false;
    }

    if ( eid != e_order.Details().exchangeOrderId ) {
        log << "Invalid exchange order id!" << endl;
        log << "Expected: " << eid << endl;
        log << "Actual  : " << e_order.Details().exchangeOrderId << endl;
        ok = false;
    }

    if ( order.originalQuantity != e_order.Details().originalQuantity ) {
        log << "Invalid qty!" << endl;
        log << "Expected: " << order.originalQuantity << endl;
        log << "Actual  : " << e_order.Details().originalQuantity << endl;
        ok = false;
    }
     
    if ( order.originalQuantity != e_order.Status().remainingQuantity ) {
        log << "Invalid remaining qty!" << endl;
        log << "Expected: " << order.originalQuantity << endl;
        log << "Actual  : " << e_order.Status().remainingQuantity << endl;
        ok = false;
    }

    if ( 0 != e_order.Status().totalFilled ) {
        log << "Invalid filled qty!" << endl;
        log << "Expected: " << 0 << endl;
        log << "Actual  : " << e_order.Status().totalFilled << endl;
        ok = false;
    }

    if ( order.limitPrice != e_order.Details().limitPrice ) {
        log << "Invalid price!" << endl;
        log << "Expected: " << order.limitPrice << endl;
        log << "Actual  : " << e_order.Details().limitPrice << endl;
        ok = false;
    }

    if ( e_order.Details().rejectionReason != rejectReason ) {
        log << "Invalid rejection reason!" << endl;
        log.ReportStringDiff(order.direction.StringValue(),e_order.Details().direction.StringValue());
        ok = false;
        ok = false;
    }

    return ok;
}

bool TestChckers::ValidateDraftOrder(testLogger& log, ExchangeOrder& e_order) {
    bool ok = true;

    if (e_order.Status().state != OrderStatus::ORDER_STATUS_DRAFT) {
        log << "Order is not in draft state: " << e_order.Status().state.StringValue() << endl;
        ok = false;
    } 

    if (e_order.Details().exchangeOrderId != 0) {
        log << "Draft order has an exchange id!" << e_order.Details().exchangeOrderId << endl;
        ok = false;
    } 

    if (e_order.Details().rejectionReason != "") {
        log << "Draft order has a rejection reason: " << e_order.Details().rejectionReason << endl;
        ok = false;
    } 

    return ok;
}

bool TestChckers::ValidateSentOrder(testLogger& log, ExchangeOrder& e_order) {
    bool ok = true;

    if (e_order.Status().state != OrderStatus::ORDER_STATUS_SENT_TO_EXCHANGE) {
        log << "Order is not in sent state: " << e_order.Status().state.StringValue() << endl;
        ok = false;
    } 

    if (e_order.Details().exchangeOrderId != 0) {
        log << "Sent order has an exchange id!" << e_order.Details().exchangeOrderId << endl;
        ok = false;
    } 

    if (e_order.Details().rejectionReason != "") {
        log << "Sent order has a rejection reason: " << e_order.Details().rejectionReason << endl;
        ok = false;
    } 

    return ok;
}

bool TestChckers::ValidateAcceptedOrder(testLogger& log,
                                        ExchangeOrder& e_order,
                                        size_t        eid)
{
    bool ok = true;

    if (e_order.Status().state != OrderStatus::ORDER_STATUS_ON_EXCHANGE) {
        log << "Order is not in accepted state: " << e_order.Status().state.StringValue() << endl;
        ok = false;
    } 

    if (e_order.Details().exchangeOrderId != eid) {
        log << "Accepted order has incorrect exchange id!" << e_order.Details().exchangeOrderId << endl;
        ok = false;
    } 

    if (e_order.Details().rejectionReason != "") {
        log << "Accepted order has a rejection reason: " << e_order.Details().rejectionReason << endl;
        ok = false;
    } 

    return ok;
}

bool TestChckers::ValidateRejectedOrder(testLogger& log,
                                        ExchangeOrder& e_order,
                                        const std::string& reason)
{
    bool ok = true;

    if (e_order.Status().state != OrderStatus::ORDER_STATUS_EXCHANGE_REJECTED) {
        log << "Order is not in rejected state: " << e_order.Status().state.StringValue() << endl;
        ok = false;
    } 

    if (e_order.Details().exchangeOrderId != 0) {
        log << "Rejected order has an exchange id!" << e_order.Details().exchangeOrderId << endl;
        ok = false;
    } 

    if (e_order.Details().rejectionReason != reason) {
        log << "Invalid rejection reason on rejected order!" << endl;
        log.ReportStringDiff(reason,e_order.Details().rejectionReason);
        ok = false;
    } 

    return ok;
}

bool TestChckers::ValidateSystemRejectedOrder(testLogger& log,
                                              ExchangeOrder& e_order,
                                              const std::string& reason)
{
    bool ok = true;

    if (e_order.Status().state != OrderStatus::ORDER_STATUS_SYSTEM_REJECTED) {
        log << "Order is not in system rejected (error) state: " << e_order.Status().state.StringValue() << endl;
        ok = false;
    } 

    if (e_order.Details().exchangeOrderId != 0) {
        log << "Rejected order has an exchange id!" << e_order.Details().exchangeOrderId << endl;
        ok = false;
    } 

    if (e_order.Details().rejectionReason != reason) {
        log << "Invalid rejection reason on rejected order!" << endl;
        log.ReportStringDiff(reason,e_order.Details().rejectionReason);
        ok = false;
    } 

    return ok;
}

bool TestChckers::ValidateUnfilledOrderUpdate(testLogger& log,
                                              OrderUpdateClient& client,
                                              OrderStatus::OrderState oldState,
                                              OrderStatus::OrderState newState,
                                              long   qty,
                                              size_t internal_id,
                                              size_t external_id)
{
    bool ok = true;
    ExchangeOrder::OrderUpdate update;
    if ( !client->GetNextMessage(update)) {
        log << "No order update to find!" << endl;
        ok = false;
    }

    if (update.internalOrderId != internal_id) {
        log << "Internal ID missmatch" << endl;
        log << "Expected: " << internal_id << endl;
        log << "Actual  : " << update.internalOrderId << endl;
        ok = false;
    }

    if (update.exchangeOrderId != external_id) {
        log << "External ID missmatch" << endl;
        log << "Expected: " << external_id << endl;
        log << "Actual  : " << update.exchangeOrderId << endl;
        ok = false;
    }

    auto validateEmptyOrder = [&log] (const OrderStatus& status, 
                                  OrderStatus::OrderState state,
                                  long qty) -> bool
    {
        bool ok = true;

        if ( status.state != state ) {
            log << "Invalid state on update!" << endl;
            log.ReportStringDiff(state.StringValue(),status.state.StringValue());
            ok = false;
        }

        if ( status.totalFilled != 0 ) {
            log << "Unexpected totalFilled: " << status.totalFilled << endl;
            ok = false;
        }

        if ( status.remainingQuantity != qty ) {
            log << "Unexpected quantity: " << endl;
            log << "Expected: " << qty << endl;
            log << "Actual  : " << status.remainingQuantity << endl;
            ok = false;
        }

        return ok;
    };

    if ( !validateEmptyOrder(update.oldStatus,oldState,qty)) {
        log << "Failed to validate old state! "  << endl;
        ok = false;
    }

    if ( !validateEmptyOrder(update.newStatus,newState,qty)) {
        log << "Failed to validate new state! "  << endl;
        ok = false;
    }
    return ok;
}

bool TestChckers::ValidateUpdatesClientEmpty(testLogger& log,
                                             OrderUpdateClient& client)
{
    bool ok = true;
    ExchangeOrder::OrderUpdate update;
    if ( client->GetNextMessage(update)) {
        log << "Order update queue was not empty!" << endl;
        ok = false;
    }
    return ok;
}
