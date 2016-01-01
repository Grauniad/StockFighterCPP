#include "StockFighterAPIMock.h"
#include <SimpleJSON.h>

StockFighterAPIMockBase::StockFighterAPIMockBase(IPostable* _event_loop)
    : nextId(0),
      event_loop(_event_loop)
{
}

std::string StockFighterAPIMockBase::StocksJSON(std::vector<raw_stock> stocks) {
    SimpleJSONPrettyBuilder builder;
    builder.Add("ok",true);
    builder.StartArray("symbols");
    for (raw_stock& s : stocks ) {
        builder.StartAnonymousObject();
        builder.Add("name",s.name);
        builder.Add("symbol",s.id);
        builder.EndObject();
    }
    builder.EndArray();

    return builder.GetAndClear();
}

void StockFighterAPIMockBase::PublishQuote(const std::string& exch, const QuoteHandler::Quote& quote)
{
    GetPublisher(exch).Publish(quote);
}

QuoteSubscription::QuoteClient StockFighterAPIMockBase::NewExchangeQuoteClient(
                                    const std::string& exch,
                                    const std::string& acc)
{
    Publisher& pub = GetPublisher(exch);
    QuoteSubscription::QuoteClient client (pub.NewClient(10240));

    return client;
}

StockFighterAPIMockBase::Publisher& StockFighterAPIMockBase::GetPublisher(const std::string& exch) {
    auto it = exchanges.find(exch);
    if (it == exchanges.end()) {
        it = exchanges.emplace(std::piecewise_construct,
                          std::forward_as_tuple(exch),
                          std::forward_as_tuple()).first;
    }

    return it->second;
}
