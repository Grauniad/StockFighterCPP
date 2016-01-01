#ifndef STOCK_FIGHTER_STOCK_FIGHTER_TRADE_SUBSCRIPER_MANAGER_H__
#define STOCK_FIGHTER_STOCK_FIGHTER_TRADE_SUBSCRIPER_MANAGER_H__

#include <QuoteSubscription.h>
#include <StockFighterConfig.h>

#include <map>

class QuoteSubscriptions {
public:
    QuoteSubscriptions(const StockFighterConfig& cfg);

    QuoteSubscription::QuoteClient NewClient(const std::string& exchange,
                                             const std::string& account);

private:
    QuoteSubscription& GetPublisher(const std::string& exchange,
                                    const std::string& account);
    struct Key { 
        const std::string exch;
        const std::string acc;

        bool operator<(const Key& rhs) const;
    };

    typedef std::map<Key,QuoteSubscription*> PublisherMap;

    PublisherMap publishers;
    const StockFighterConfig& cfg;

};

#endif
