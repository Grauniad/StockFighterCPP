#ifndef STOCK_FIGHTER_STOCK_FIGHTER_INTERFACE_H__
#define STOCK_FIGHTER_STOCK_FIGHTER_INTERFACE_H__

#include <io_thread.h>
#include <StockFighterConfig.h>
#include <QuoteSubscriptionManager.h>

typedef std::shared_ptr<HTTPMessageProvider> StockFighterReqHdl;

#include <OrderPost.h>

class OrderDefinition;

/**
 * Virtual base class for mocking out API.
 * 
 * All of these functions trigger a network request so the added overhead 
 * of the vtable lookup is completely negligible
 */
class StockFighterAPI {
public:
    /***********************************************************
     *                     Static Data 
     ***********************************************************/
    virtual StockFighterReqHdl GetStocks(const std::string& exchange) = 0;

    /***********************************************************
     *                     Market Data 
     ***********************************************************/
    virtual QuoteSubscription::QuoteClient NewExchangeQuoteClient(
                                        const std::string& exch,
                                        const std::string& acc) = 0;

    /***********************************************************
     *                     Post Orders
     ***********************************************************/
     // Recomended Interface
     virtual OrderPostReqHdl PostOrder(const OrderDefinition& order) = 0;

     // Raw Interface use by OrderPostRequest - not recomended for use  
    virtual StockFighterReqHdl PostOrder(const std::string& venue,
                                         const std::string& symbol,
                                         const std::string& order) = 0;
};

/**
 * Our wrapper around the stock fighter API
 */
class StockFighterConnection: public StockFighterAPI {
public: 
    /**
     * Initialise the connection
     */
    StockFighterConnection(const StockFighterConfig& cfg);

    /**
     * Request a list of stocks on the exchange.
     *
     * @param exchange   The exchange to request the stocks for.
     */
    virtual StockFighterReqHdl GetStocks(const std::string& exchange);

    /**
     * Subscribe to quote updates from an exchange.
     */
     virtual QuoteSubscription::QuoteClient NewExchangeQuoteClient(
                                        const std::string& exch,
                                        const std::string& acc);

     // Recomended Interface
     virtual OrderPostReqHdl PostOrder(const OrderDefinition& order);

     // Raw Interface use by OrderPostRequest - not recomended for use  
    virtual StockFighterReqHdl PostOrder(const std::string& venue,
                                         const std::string& symbol,
                                         const std::string& order);


private:
    const std::string authKey;
    const std::string server;
    const std::string apiBase;

    IOThread          requestThread;

    QuoteSubscriptions quotes;
};

#endif
