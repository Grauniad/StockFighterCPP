/*
 * ReqAccounts.h
 *
 *  Created on: 27 Dec 2015
 *      Author: lhumphreys
 */

#ifndef __STOCK_FIGHTER_LIBRARIES_REQUEST_SERVER_MARKET_DATA_SUB_H__
#define __STOCK_FIGHTER_LIBRARIES_REQUEST_SERVER_MARKET_DATA_SUB_H__

#include "ReqServer.h"
#include <MarketData.h>

class AccountConfig;
class LevelConfig;
class MarketDataManager;
class MarketDataProvider;

/*
 * Subscribe to market data updates for a given instrument.
 *
 * REQUEST:
 *    {
 *        account:  "Trading Account",
 *        exchange: "Exchange",
 *        symbol:   "Stock ID"
 *    }
 *
 * UPDATE:
 *    {
 *         bidPrice:  100.0,
 *         askPrice:  100.0,
 *         lastPrice: 100.0
 *    }
 */
class SubMarketData: public SubscriptionHandler {
public:
    SubMarketData(LevelConfig& config, MarketDataManager& marketData);

    class Client {
    public:
        Client(RequestHandle& h, MarketDataClient& data);

        // Can't have two copies of the subscription...
        Client(const Client& rhs) = delete;
        Client& operator=(const Client& rhs) = delete;

        // ...but moving the references is fine.
        Client(Client&& rhs) = default;
        Client& operator=(Client&& rhs) = default;
    private:
        RequestHandle    request;
        MarketDataClient client;
    };

    virtual void OnRequest(RequestHandle hdl);
private:
    void Setup(const char* request);

    void SetupAccount();
    void SetupMarketDataProvider(); 
    void StartSubscription(RequestHandle& hdl);

    MarketDataManager& marketDataManager;
    LevelConfig&       config;

    AccountConfig*      account;
    MarketDataClient    client;

    std::vector<Client> activeSubs;
};

#endif /* DEV_TOOLS_CPP_LIBRARIES_LIBWEBSOCKETS_REQFILELIST_H_ */
