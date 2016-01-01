#include "StockMarketDataProvider.h"

StockMarketDataProvider::StockMarketDataProvider() {
    // Nothing to do...
}

MarketDataClient StockMarketDataProvider::NewUpdatesClient() {
    MarketDataClient client(publisher.NewClient<MarketDataClient::element_type>(10240));

    client->PushMessage(currentData);

    return client;
}


void StockMarketDataProvider::ApplyUpdate(
         const QuoteSubscription::Quote& update)
{
    currentData.bestBid.price = update.bestBid.price;
    currentData.bestBid.qty   = update.bestBid.qty;
    currentData.bestAsk.price = update.bestAsk.price;
    currentData.bestAsk.qty   = update.bestAsk.qty;
    currentData.last.price    = update.last.price;
    currentData.last.qty      = update.last.qty;

    Publish();
}

void StockMarketDataProvider::Publish() {
    publisher.Publish(currentData);
}
