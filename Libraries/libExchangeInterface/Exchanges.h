#ifndef STOCK_FIGHTER_EXCHANGES_CONFIG_H__
#define STOCK_FIGHTER_EXCHANGES_CONFIG_H__

#include <ExchangeInterface.h>
#include <map>

class Exchanges { 
public:

    /**
     * Initialise all exchanges
     */
    Exchanges(StockFighterAPI& api, std::vector<VenueConfig>& venues);

    Exchanges(Exchanges&& move_fom) = default;

    typedef std::vector<Exchange*> ExchangeRefList;

    /**
     * Return the full set of exchanges
     */
    const ExchangeRefList& ExchangeList() {
        return exchanges;
    }

    /**
     * Return the exchange with the specified code.
     *
     * If the exchange does not exist, nullptr is returned.
     *
     * @returns The exchange, or nullptr
     */
    Exchange* GetExchange(const std::string& code);

private:
    /**
     * Lookup all exchange, but do not wait for them to initialise 
     */
    void SetupExchanges(
             StockFighterAPI& api,
             std::vector<VenueConfig>& venues);

    /**
     * Wait for all exchanges to initialise
     */
    void WaitForInitialiseAll();

    std::vector<Exchange*> exchanges;
    typedef std::map<std::string, Exchange> ExchMap;
    ExchMap   exchangeMap;
};

#endif

