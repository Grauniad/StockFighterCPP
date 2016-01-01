#include "OrderPost.h"
#include <StockFighterConnection.h>
#include <SimpleJSON.h>
/**********************************************************
 *                       JSON Parsing
 **********************************************************/

namespace Response {
    NewStringField(account);
    NewStringField(direction);
    NewStringField(error);

    namespace fills_fields {
        NewUIntField(price);
        NewUIntField(qty);
        NewStringField(ts);

        typedef SimpleParsedJSON<
            price,
            qty,
            ts
        > JSON;
    }
    NewObjectArray(fills, fills_fields::JSON);

    NewUIntField(id);
    NewBoolField(ok);
    NewBoolField(open);
    NewUIntField(originalQty);
    NewUIntField(price);
    NewUIntField(qty);
    NewStringField(symbol);
    NewUIntField(totalFilled);
    NewStringField(ts);
    NewStringField(type);
    NewStringField(venue);

    typedef SimpleParsedJSON<
        account,
        direction,
        error,
        fills,
        id,
        ok,
        open,
        originalQty,
        price,
        qty,
        symbol,
        totalFilled,
        ts,
        type,
        venue
    > Response;
}


/**********************************************************
 *                  Order Definition
 **********************************************************/
OrderDefinition::OrderDefinition()
    : originalQuantity(0),
      limitPrice(0)
{
}

template<>
OrderDefinition::Direction::Map 
     OrderDefinition::Direction::string_map =
{ 
    {"buy", OrderDefinition::DIRECTION_BUY},
    {"sell", OrderDefinition::DIRECTION_SELL},
    {"", OrderDefinition::DIRECTION_ERROR},
};

template<>
OrderDefinition::OrderType::Map
    OrderDefinition::OrderType::string_map =
{ 
    {"limit", OrderDefinition::ORDER_TYPE_LIMIT},
    {"market", OrderDefinition::ORDER_TYPE_MARKET},
    {"fill-or-kill", OrderDefinition::ORDER_TYPE_FILL_OR_KILl},
    {"immediate-or-cancel", OrderDefinition::ORDER_TYPE_IMMEDIATE_OR_CANACEL},
    {"", OrderDefinition::ORDER_TYPE_ERROR}
};
/**********************************************************
 *                  Order Response
 **********************************************************/
OrderPostRequest::OrderResponse::OrderResponse() 
   : ok(false), exchangeOrderId(0)
{
}

/**********************************************************
 *                  Order Post Request
 **********************************************************/

OrderPostRequest::OrderPostRequest(StockFighterAPI& api, OrderDefinition order)
   : statusFuture(statusFlag.get_future()),
     ready(false),
     notify(false),
     notification(nullptr)
{
    std::string request = GetJSONRequest(order);
    postRequest = api.PostOrder(order.venue,order.symbol,request);

    //OrderResponse& target = response;
    auto on_response = [this] () -> void {
        const std::string& message = postRequest->WaitForMessage().content.str();

        ParseResponse(message,response);

        ready = true;

        statusFlag.set_value(1);

        std::unique_lock<std::mutex> lock(notifyMutex);

        if (notify) {
            notification(response);
        }
    };

    postRequest->OnMessage(on_response);
}

const OrderPostRequest::OrderResponse& OrderPostRequest::WaitForResponse() {
    statusFuture.wait();
    return response;
}

std::string OrderPostRequest::GetJSONRequest( const OrderDefinition& order) {
    thread_local SimpleJSONBuilder builder;
    builder.Clear();

    builder.Add("account", order.account);
    builder.Add("stock", order.symbol);
    builder.Add("venue", order.venue);
    builder.Add("qty", order.originalQuantity);
    builder.Add("direction", order.direction.StringValue());
    builder.Add("orderType", order.type.StringValue());

    if (order.type != OrderDefinition::ORDER_TYPE_MARKET) {
        builder.Add("price",order.limitPrice);
    }

    return builder.GetAndClear();
}

void OrderPostRequest::ParseResponse(const std::string& message,
                                     OrderResponse& response)
{
    thread_local Response::Response parser;
    parser.Clear();

    std::string error;
    if (parser.Parse(message.c_str(),error)) {
        response.ok = parser.Get<Response::ok>();
        response.error = parser.Get<Response::error>();
        response.exchangeOrderId = parser.Get<Response::id>();
    } else {
        response.ok = false;
        response.error = "Failed to parse exchange message: ";
        response.error += error;

    }
}


void OrderPostRequest::OnResponse(const Notification& _notification) {
    std::unique_lock<std::mutex> lock(notifyMutex);

    notify = true;
    notification = _notification;
}
