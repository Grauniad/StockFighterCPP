#include "MarketData.h"

MarketData::MarketData()
   : bestBid({0,0}), bestAsk({0,0}), last({0,0})
{
}

MarketDataSubscriber::MarketDataSubscriber(
        PipePublisher<MarketData>* parent,
        size_t maxSize)
    : PipeSubscriber(parent,maxSize)
{
}

const MarketData& MarketDataSubscriber::CurrentQuote() const {
    return current;
}

void MarketDataSubscriber::PushMessage(const MarketData& msg) {
    current = msg;
    PipeSubscriber<MarketData>::PushMessage(msg);
}
