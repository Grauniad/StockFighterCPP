#include "StockFighterAPIMock.h"
#include <SimpleJSON.h>

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

namespace Request {
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
}

OrderPostReqHdl StockFighterAPIMockBase::PostOrder(const OrderDefinition& order) {
    OrderPostReqHdl orderReq(new OrderPostRequest(*this,order));

    return orderReq;
}

StockFighterReqHdl StockFighterAPIMockBase::PostOrder(const std::string& venue,
                                                      const std::string& symbol,
                                                      const std::string& order)
{
    lastPostOrderCall.venue = venue;
    lastPostOrderCall.symbol = symbol;
    lastPostOrderCall.order = order;

    std::shared_ptr<TestHTTPRequest> request(new TestHTTPRequest(event_loop));

    HTTPMessage& results = request->message;
    results.status_code = 200;

    if ( orderPostResponse != "" ) {
        results.content << orderPostResponse;
        orderPostResponse = "";
    } else {
        thread_local Request::Request request;
        thread_local Response::Response response;
        response.Clear();
        request.Clear();
        std::string error;
        request.Parse(order.c_str(),error);

        response.Get<Response::direction>() = request.Get<Request::direction>();
        response.Get<Response::error>() = "";
        response.Get<Response::id>() = ++nextId;
        response.Get<Response::ok>() = true;
        response.Get<Response::open>() = true;
        response.Get<Response::originalQty>() = request.Get<Request::qty>();
        response.Get<Response::price>() = request.Get<Request::price>();
        response.Get<Response::qty>() = request.Get<Request::qty>();
        response.Get<Response::symbol>() = request.Get<Request::stock>();
        response.Get<Response::type>() = request.Get<Request::orderType>();
        response.Get<Response::venue>() = request.Get<Request::venue>();

        results.content << response.GetPrettyJSONString();
    }

    return request;
}

void StockFighterAPIMockBase::SetNextOrderResponse(const std::string& response) {
    orderPostResponse = response;
}
