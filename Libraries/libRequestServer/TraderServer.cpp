#include "TraderServer.h"

#include <ReqAccounts.h>
#include <ReqVenues.h>
#include <ReqStocks.h>
#include <SubMarketData.h>
#include <MarketDataManager.h>
#include <ReqNewOrder.h>

TraderServer::TraderServer(Exchanges& exchanges,LevelConfig& cfg) 
    : listenPort(cfg.ClientConnectionConfig().ListenPort()),
      exchangeInterface(exchanges)
{
}

void TraderServer::Run() {
    requestServer.HandleRequests(listenPort);
}

void TraderServer::InstallRequestHandlers(
             LevelConfig& cfg,
             MarketDataManager& mgr,
             OrderActionHandler& actionHandler)
{

    std::unique_ptr<ReqAccounts> accountsHandler(new ReqAccounts(cfg));

    requestServer.AddHandler("ReqTraderAccounts", std::move(accountsHandler));

    std::unique_ptr<ReqVenues> venuesHandler(new ReqVenues(cfg));

    requestServer.AddHandler("ReqTraderVenues", std::move(venuesHandler));

    std::unique_ptr<ReqStocks> stocksHandler(new ReqStocks(exchangeInterface));

    requestServer.AddHandler("ReqTraderStocks", std::move(stocksHandler));

    std::unique_ptr<SubMarketData> marketData(new SubMarketData(cfg,mgr));

    requestServer.AddHandler("SubMarketData", std::move(marketData));

    std::unique_ptr<ReqNewOrder> newOrder(new ReqNewOrder(actionHandler));

    requestServer.AddHandler("ReqNewOrder", std::move(newOrder));
}

void TraderServer::PostTask(const RequestServer::InteruptHandler& f) {
    requestServer.PostTask(f);
}
