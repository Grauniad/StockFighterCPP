#ifndef STOCK_FIGHTER_EXCHANGE_MARKET_DATA_PROVIDER_H__
#define STOCK_FIGHTER_EXCHANGE_MARKET_DATA_PROVIDER_H__

#include <MarketData.h>
#include <StockFighterConnection.h>
#include <QuoteSubscription.h>
#include <map>
#include <AccountConfig.h> 
#include <ExchangeInterface.h> 
#include <functional>
#include <StockMarketDataProvider.h>

class TraderServer;

/**
 * Responsible for providing, and maintaining,  market data for a given
 * exchange.
 */

class ExchangeMarketDataProvider {
public:
     /**
      * Start the subscription, and register update handlers in the event loop.
      *
      * @param api          The stock-figherr API connection
      * @param account      The account to register for market data with
      * @param exchange     The exchange to register for market data.
      * @param event_loop   The event-loop where market updates should be handled.
      *
      */
     ExchangeMarketDataProvider(
         StockFighterAPI& api,
         const AccountConfig& account,
         Exchange& exchange,
         IPostable& event_loop);

    /**
     * Get market data updates for a given a given stock on this exchange.
     *
     * If the stock does not exist a nullptr is returned.
     */
    MarketDataClient NewUpdatesClient(const std::string& stock);
private:
    /**
     * Pull all outstanding updates from the quote client, and re-register the
     * unread data callback.
     */
    void OnUnreadData();

    /**
     * Process a market data update from the quote subcription service
     */
     void ApplyUpdate(QuoteSubscription::Quote& update);

    // DATA
    IPostable&  event_loop;
    QuoteSubscription::QuoteClient  marketQuoteClient;

    typedef std::map<std::string,StockMarketDataProvider> StockProviderMap;

    StockProviderMap stocks;
    std::function<void()> onData; // call-back on the request thread to handle MD updates
};

#endif
