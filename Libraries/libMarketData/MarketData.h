#ifndef STOCK_FIGHTER_MARKET_DATA_H__
#define STOCK_FIGHTER_MARKET_DATA_H__

#include <PipePublisher.h>
#include <PipeSubscriber.h>

struct MarketData {
    MarketData();

    // Support data copy
    MarketData(const MarketData& rhs) = default;
    MarketData& operator=(const MarketData& rhs) = default;

    MarketData(MarketData&& rhs) = default;
    MarketData& operator=(MarketData&& rhs) = default;

    struct Price {
        long   price;
        long   qty;
    };
    Price bestBid;
    Price bestAsk;
    Price last;
};

typedef PipePublisher<MarketData> MarketDataPublisher;
class MarketDataSubscriber: public PipeSubscriber<MarketData> {
public: 
    MarketDataSubscriber(PipePublisher<MarketData>* parent, size_t maxSize);

    /**
     * The latest market data update
     */
    const MarketData& CurrentQuote() const;

    /**
     * Intercept the push so we can update the current data.
     */
    virtual void PushMessage(const MarketData& msg); 
private:
    MarketData current;
};
typedef std::shared_ptr<MarketDataSubscriber> MarketDataClient;

#endif
