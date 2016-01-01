#ifndef STOCK_FIGHTER_STOCK__H__
#define STOCK_FIGHTER_STOCK__H__

#include <io_thread.h>
#include <VenueConfig.h>
#include <StockFighterConnection.h>
#include <atomic>
#include <string>
#include <vector>

class Exchange;

class Stock {
public:
    /*********************************************
     *       Define Configuration Message
     *********************************************/
    NewStringField(name);
    NewStringField(symbol);
    typedef SimpleParsedJSON<name,symbol> Config;

    /*********************************************
     *       C'tor
     *********************************************/
    Stock(Exchange& exch, Config& cfg);

    /*********************************************
     *       Accessors
     *********************************************/

    /**
     * The user friendly name for the stock
     */
    const std::string& Name() const { return displayName; } 

    /**
     * The ID the exchange knows this instrument by.
     */
    const std::string& Ticker() const { return ticker; } 
private:
    /*********************************************
     *       Data
     *********************************************/
    Exchange&         exchange;
    const std::string displayName;
    const std::string ticker;
};

#endif
