#ifndef STOCK_FIGHTER_MARKET_DATA_PROVIDER_H__
#define STOCK_FIGHTER_MARKET_DATA_PROVIDER_H__

#include <map>
#include <MarketData.h>
#include <ExchangeMarketDataProvider.h>
#include <Exchanges.h>


/**
 * Responsible for providing market data for all known stocks.
 * 
 * The market data provider should be initialised after the Exchanges static data has
 * completed initialisation.
 */

class MarketDataProvider {
public:

    /**
     * Initialise the market data provider, using the specified account, on the
     * given exchanges.
     */
    MarketDataProvider(
        StockFighterAPI& api,
        const AccountConfig& account,
        const Exchanges::ExchangeRefList& exchanges,
        IPostable& event_loop);

    /**
     * Return the market data client for a given stock. If the stock does not
     * exist for this exchange, the nullptr is returned.
     */
    MarketDataClient NewUpdatesClient(
                         const std::string& exch,
                         const std::string& stock);

private:
    const std::string account;
    typedef std::map<std::string,ExchangeMarketDataProvider> ExchangeProviderMap;

    ExchangeProviderMap  exchanges;
};

#endif
