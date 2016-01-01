#ifndef STOCK_FIGHTER_MARKET_DATA_MANAGER_H__
#define STOCK_FIGHTER_MARKET_DATA_MANAGER_H__

#include <map>
#include <MarketDataProvider.h>
#include <AccountConfig.h>

class MarketDataManager {
public:

    /**
     * Initialise the market data manager, ready to server up new 
     * market data providerse as they are required.
     */
    MarketDataManager(
        StockFighterAPI& api,
        Exchanges& exchanges,
        IPostable& event_loop);

    /**
     * Get the market data provider for the given trading account.
     *
     * If the market data provider does not yet exist, it will be created.
     *
     * @param account   The account to returtn the market data for.
     */
     MarketDataProvider& GetMarketDataProvider(const AccountConfig& account);

private:
    StockFighterAPI&   api;
    Exchanges&         exchanges;
    IPostable&         event_loop;
     
    typedef std::map<std::string,MarketDataProvider> ProviderMap;

    ProviderMap  providers;
};

#endif
