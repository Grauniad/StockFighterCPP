#ifndef STOCK_FIGHTER_EXCHANGE_INTERFACE_CONFIG_H__
#define STOCK_FIGHTER_EXCHANGE_INTERFACE_CONFIG_H__

#include <VenueConfig.h>
#include <StockFighterConnection.h>
#include <atomic>
#include <string>
#include <vector>
#include <Stock.h>

class Exchange {
public:
    Exchange(StockFighterAPI& api, const VenueConfig& exch);

    /**
     * Indicates if the exchange has completed all initialisation requests.
     *
     * NOTE: OK should be checked to determine if the exchange initialised
     *       correctly.
     *
     */
    bool Ready() const;

    /**
     * Check if the exchange initialised ok.
     */
    bool OK() const;

    /**
     * Wait synchronously for initialisation requests to complete.
     *
     * @returns The value of OK() following completion of initialisation.
     */
    bool WaitForInitialisation();

    std::vector<Stock>& Stocks() { return stocks; } 

    /**
     * Return the given stock on this exchange.
     *
     * If no such stock exists, a nullptr is returned.
     *
     * WARNING: Calling this before Ready returns true will result in a
     *          syncrhonous wait.
     */
    const Stock* GetStock(const std::string& symbol);

    const std::string& Name() const { return exchName; } 
    const std::string& Id() const { return exchId; } 
private:
    /**
     * Parse the result of a stocks request
     */
    void ParseStocks(const std::string& response);

    std::atomic<bool> ready;
    std::atomic<bool> ok;
    const std::string exchName;
    const std::string exchId;
    std::vector<Stock> stocks;
    std::map<std::string, Stock*> stockMap;

    StockFighterAPI& api;

    StockFighterReqHdl stocksRequest;
};

#endif
