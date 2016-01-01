#include "ExchangeMarketDataProvider.h"

ExchangeMarketDataProvider::ExchangeMarketDataProvider(
        StockFighterAPI& api,
        const AccountConfig& account,
        Exchange& exchange,
        IPostable& _event_loop)
    : event_loop(_event_loop),
      marketQuoteClient(
          api.NewExchangeQuoteClient(exchange.Id(),account.AccountCode()))
{
    for (const Stock& stock: exchange.Stocks()) {
        stocks.emplace(std::piecewise_construct,
                       std::forward_as_tuple(stock.Ticker()),
                       std::forward_as_tuple());
    }

    onData = [this] () -> void {
        this->OnUnreadData();
    };

    onData();
}

void ExchangeMarketDataProvider::OnUnreadData() {
    QuoteSubscription::Quote quote;
    while (marketQuoteClient->GetNextMessage(quote)) {
        ApplyUpdate(quote);
    }

    marketQuoteClient->OnNextMessage(onData,&event_loop);
}

 void ExchangeMarketDataProvider::ApplyUpdate(QuoteSubscription::Quote& update)
 {
     auto it = stocks.find(update.symbol);

     if ( it != stocks.end() ) {
         it->second.ApplyUpdate(update);
     }
 }

MarketDataClient ExchangeMarketDataProvider::NewUpdatesClient(
                     const std::string& stock)
{
    MarketDataClient client(nullptr);

    auto it = stocks.find(stock);

    if (it != stocks.end()) {
        client = it->second.NewUpdatesClient();
    }

    return client;
}
