#ifndef STOCK_FIGHTER_STOCK_FIGHTER_TRADE_SUBSCRIBER_H__
#define STOCK_FIGHTER_STOCK_FIGHTER_TRADE_SUBSCRIBER_H__

#include <StockFighterConfig.h>
#include <string>
#include <stream_client.h>
#include <PipePublisher.h>

/**
 * Implementation of quote handling logic, isolated from the websockets testing
 * to allow unit testing.
 *
 * Applications should use the QuoteSubscription class, which implements the 
 * websocket interface and kicks up the subscription thread.
 */
class QuoteHandler {
public:
    struct Quote {
        struct Price {
            long   price;
            long   qty;
        };
        std::string symbol;
        Price bestBid;
        Price bestAsk;
        Price last;
    };

    typedef std::shared_ptr<PipeSubscriber<Quote>> QuoteClient;

    /**
     * Subscribe to quote notifications.
     */
    QuoteClient NewClient();

protected:
    /**
     * Handler used by QuoteSubscription to parse a new quote message from the
     * exchange.
     */
    void HandleQuoteMessage(const std::string& msg);

    static std::string MakeUrl(
        const std::string& exch,
        const std::string& acc,
        const StockFighterConfig& cfg);
private:

    PipePublisher<Quote> publisher;
};

/**
 * Subscribe to trade notifications on a given exchange.
 */
class QuoteSubscription: public StreamClientThread, public QuoteHandler {
public: 
    /**
     * Initialise the connection
     */
    QuoteSubscription(
        const std::string& exch,
        const std::string& acc,
        const StockFighterConfig& cfg);

protected:
    /**
     * A new trade notification.
     */
    virtual void OnMessage(const std::string& msg);
private:

};

#endif
