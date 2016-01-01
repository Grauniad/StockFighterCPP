#include "SubMarketData.h"

#include <SimpleJSON.h>
#include <LevelConfig.h>
#include <MarketDataManager.h>

/******************************************************
 *              JSON Parsing
 ******************************************************/

namespace Parser {
    NewStringField(account);
    NewStringField(exchange);
    NewStringField(symbol);

    typedef SimpleParsedJSON<
        account,
        exchange,
        symbol
    > Request;

    NewDoubleField(bidPrice);
    NewDoubleField(askPrice);
    NewDoubleField(lastPrice);

    typedef SimpleParsedJSON<
        bidPrice,
        askPrice,
        lastPrice
    > Reply;

    thread_local Request request;
    thread_local Reply   reply;

    void PopulateReply(const MarketData& data) {
        reply.Clear();
        reply.Get<bidPrice>() = ((double)data.bestBid.price) / 100.0;
        reply.Get<askPrice>() = ((double)data.bestAsk.price) / 100.0;
        reply.Get<lastPrice>() = ((double)data.last.price) / 100.0;
    }
}

/******************************************************
 *              Request Handler
 ******************************************************/

SubMarketData::SubMarketData(LevelConfig& cfg, MarketDataManager& _marketData) 
    : marketDataManager(_marketData),
      config(cfg),
      account(nullptr),
      client(nullptr)
{
}

void SubMarketData::OnRequest(RequestHandle hdl) {
    Setup(hdl->RequestMessasge());

    StartSubscription(hdl);
}

void SubMarketData::Setup(const char* json_request) {
    Parser::request.Clear();
    Parser::reply.Clear();
    account = nullptr;
    client.reset();

    std::string error = "";

    if (!Parser::request.Parse(json_request, error)) {
        throw InvalidRequestException{0,error};
    }

    SetupAccount();
    SetupMarketDataProvider();
}

void SubMarketData::SetupAccount() {
    account = config.GetAccountConfig(Parser::request.Get<Parser::account>());

    if (account == nullptr) {
        throw InvalidRequestException{0,"Invalid account supplied."};
    }
}

void SubMarketData::SetupMarketDataProvider() {
    MarketDataProvider& marketData = marketDataManager.GetMarketDataProvider(*account);

    client = marketData.NewUpdatesClient(
                 Parser::request.Get<Parser::exchange>(),
                 Parser::request.Get<Parser::symbol>());

    if (client == nullptr) {
        throw InvalidRequestException{0,"Invalid exchange / stock combination supplied."};
    }
}

void SubMarketData::StartSubscription(RequestHandle& hdl) {
    activeSubs.emplace_back(hdl,client);
}

/******************************************************
 *                     Client
 ******************************************************/

SubMarketData::Client::Client(RequestHandle& h, MarketDataClient& data)
   : request(h), client(data)
{
    auto reqHdl = request;
    auto sendUpdate = [reqHdl](const MarketData& update) -> void {
        Parser::PopulateReply(update);
        reqHdl->SendMessage(Parser::reply.GetJSONString());
    };

    client->OnNewMessage(sendUpdate);
}
