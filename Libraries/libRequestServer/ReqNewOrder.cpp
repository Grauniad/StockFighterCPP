#include "ReqNewOrder.h"

ReqNewOrder::ReqNewOrder(OrderActionHandler& hdlr)
   : actionHandler(hdlr)
{
}

ReqNewOrder::~ReqNewOrder()
{
}

std::string ReqNewOrder::OnRequest(const char* req) {
    Reset();
    OrderDefinition order;

    Setup(req,order);
    auto hdl = CreateOrder(order);

    return BuildReply(hdl);
}

void ReqNewOrder::Reset() {
    request.Clear();
    reply.Clear();
}

void ReqNewOrder::Setup(const char* req, OrderDefinition& order) {
    std::string error;

    if ( !request.Parse(req,error)) {
        throw InvalidRequestException{0,error};
    }

    order.account = request.Get<account>();
    order.direction.SetValue(request.Get<direction>());
    order.venue = request.Get<exchange>();
    order.limitPrice = request.Get<limit_price>();
    order.originalQuantity = request.Get<quantity>();
    order.symbol = request.Get<stock>();
    order.type.SetValue(request.Get<type>());
}

OrderHdl ReqNewOrder::CreateOrder(OrderDefinition& order) {
    std::string error;
    OrderHdl hdl = actionHandler.NewOrder(order,error);

    if ( hdl == nullptr || 
         hdl->Status().state == OrderStatus::ORDER_STATUS_SYSTEM_REJECTED)
    {
        throw InvalidRequestException{0,error};
    } 

    return hdl;
}

std::string ReqNewOrder::BuildReply(OrderHdl order) {
    reply.Get<ok>() = true;
    reply.Get<order_id>() = order->Details().internalOrderId;

    return reply.GetJSONString();
}
