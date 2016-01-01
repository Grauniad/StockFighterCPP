#include "ExchangeOrder.h"
#include <StockFighterConnection.h>
#include <logger.h>

/**********************************************
 *         Exchange Order
 **********************************************/

ExchangeOrder::ExchangeOrder(
         StockFighterAPI&         _api,
         IPostable&               _event_loop,
         size_t                   internalOrderId,
         const OrderDefinition&   order)
    : api(_api),
      event_loop(_event_loop), 
      details(internalOrderId,0,order),
      sendRequest(nullptr),
      status(TemplateDraftStatus(order))
{
}

OrderStatus ExchangeOrder::TemplateDraftStatus(const OrderDefinition& order) {
    static OrderStatus draft;
    draft.state = OrderStatus::ORDER_STATUS_DRAFT;
    draft.remainingQuantity = order.originalQuantity;
    return draft;
}

const OrderStatus& ExchangeOrder::Status() const {
    return status.Get();
}

const OrderDetails& ExchangeOrder::Details() const {
    return details;
}

bool ExchangeOrder::SendOrder() {
    bool sent = false;

    OrderStatus orderStatus = status.Get(); 
    if ( orderStatus.state == OrderStatus::ORDER_STATUS_DRAFT) {

        orderStatus.state = OrderStatus::ORDER_STATUS_SENT_TO_EXCHANGE;
        status.Update(details,orderStatus);
        sendRequest = api.PostOrder(details);
        sent = true;

        auto handler = [this] (const OrderPostRequest::OrderResponse& r) -> void {
            // We're on the IO thread - need to post back to the request thread in order
            // to be able to modify our data.
            OrderPostRequest::OrderResponse resp = r;
            auto forward = [this,resp] () -> void {
                this->HandleResponse(resp);
            };
            this->event_loop.PostTask(forward);
        };

        sendRequest->OnResponse(handler);
    }

    return sent;
}
 
bool ExchangeOrder::SystemReject(const std::string& reason) {
    bool rejected = false;

    OrderStatus orderStatus = status.Get(); 

    if ( orderStatus.state == OrderStatus::ORDER_STATUS_DRAFT) {
        orderStatus.state = OrderStatus::ORDER_STATUS_SYSTEM_REJECTED;
        details.rejectionReason = reason;
        status.Update(details,orderStatus);
        rejected = true;
    }

    return rejected;
}

void ExchangeOrder::HandleResponse(
         const OrderPostRequest::OrderResponse& response)
{
    OrderStatus orderStatus = status.Get();
    if ( response.ok ) {
        orderStatus.state = OrderStatus::ORDER_STATUS_ON_EXCHANGE;
        details.exchangeOrderId = response.exchangeOrderId;
    } else {
        orderStatus.state = OrderStatus::ORDER_STATUS_EXCHANGE_REJECTED;
        details.rejectionReason = response.error;
    }

    status.Update(details,orderStatus);
}

OrderUpdateClient ExchangeOrder::NewUpdatesClient() {
    return status.NewUpdatesClient();
}
/**********************************************
 *         Status Handler
 **********************************************/
ExchangeOrder::StatusHandler::StatusHandler(const OrderStatus& initial)
    : status(initial)
{
}

void ExchangeOrder::StatusHandler::Update(const OrderDetails& refFields,
                                          const OrderStatus& newStatus)
{
    thread_local OrderUpdate update;
    update.internalOrderId = refFields.internalOrderId;
    update.exchangeOrderId = refFields.exchangeOrderId;
    update.oldStatus = status;
    update.newStatus = newStatus;

    status = newStatus;
    publisher.Publish(update);
}

OrderUpdateClient ExchangeOrder::StatusHandler::NewUpdatesClient() {
    OrderUpdateClient client (publisher.NewClient(10240));

    return client;
}

