#include <ReqStocks.h>

ReqStocks::ReqStocks(Exchanges& data)
    : exchangeInterface(data),
      venue(nullptr)
{
}

ReqStocks::~ReqStocks() {
}

std::string ReqStocks::OnRequest(const char* req) {
    Reset();

    Setup(req);

    BuildReply();

    return reply.GetJSONString();
}

void ReqStocks::Reset() {
    request.Clear();
    reply.Clear();
    venue = nullptr;
}

void ReqStocks::Setup(const char* req) {

    std::string error;

    if (!request.Parse(req,error)) {
        throw InvalidRequestException{0,error};
    }

    venue = exchangeInterface.GetExchange(request.Get<exchange>());

    if (venue == nullptr) { 
        error = "No such exchange: ";
        error += request.Get<exchange>();
        throw InvalidRequestException{0,error};
    }
}

void ReqStocks::BuildReply() {
    for ( Stock& s: venue->Stocks()) {
        reply.Get<stocks>().emplace_back();

        auto& stock = *reply.Get<stocks>().rbegin();

        stock->Get<name>() = s.Name();
        stock->Get<id>() = s.Ticker();

    }
}
