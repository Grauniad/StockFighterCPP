#ifndef STOCK_FIGHTER_STOCK_MARKET_DATA_PROVIDER_H__
#define STOCK_FIGHTER_STOCK_MARKET_DATA_PROVIDER_H__

#include <MarketData.h>
#include <StockFighterConnection.h>
#include <QuoteSubscription.h>

/**
 * Responsible for providing market data for a given exchange.
 */

class StockMarketDataProvider {
public:
    StockMarketDataProvider();

    MarketDataClient NewUpdatesClient();

    void ApplyUpdate(const QuoteSubscription::Quote& update);
private:
    void Publish();
    MarketData            currentData;
    MarketDataPublisher   publisher;
};

#endif
