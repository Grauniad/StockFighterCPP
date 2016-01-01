#include "QuoteSubscriptionManager.h"

QuoteSubscriptions::QuoteSubscriptions(const StockFighterConfig& _cfg)
    : cfg(_cfg)
{
}

QuoteSubscription::QuoteClient QuoteSubscriptions::NewClient(
                                   const std::string& exchange,
                                   const std::string& account)
{
    return GetPublisher(exchange,account).NewClient();
}

QuoteSubscription& QuoteSubscriptions::GetPublisher(
                                   const std::string& exchange,
                                   const std::string& account)
{
    Key key = { exchange, account};
    auto it = publishers.find(key);
    if ( it == publishers.end() ) {
        QuoteSubscription* pub = new QuoteSubscription(exchange,account,cfg);
        it = publishers.emplace(key,pub).first;
    }

    return *(it->second);
}

bool QuoteSubscriptions::Key::operator<(
          const QuoteSubscriptions::Key& rhs) const
{
    if ( exch == rhs.exch) {
        return acc < rhs.acc;
    } else {
        return exch < rhs.exch;
    }
}
