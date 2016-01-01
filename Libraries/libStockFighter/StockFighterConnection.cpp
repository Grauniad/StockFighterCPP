#include "StockFighterConnection.h"

StockFighterConnection::StockFighterConnection(const StockFighterConfig& cfg)
    : authKey(cfg.AuthKey()),
      server(cfg.APIServer()),
      apiBase(cfg.APIBaseUrl()),
      quotes(cfg)
{
}

StockFighterReqHdl StockFighterConnection::GetStocks(
                       const std::string& exchange) 
{
    std::string url = apiBase + "venues/" + exchange + "/stocks";
    return requestThread.HTTPSRequest(server,url);
}

QuoteSubscription::QuoteClient StockFighterConnection::NewExchangeQuoteClient(
                                   const std::string& exch,
                                   const std::string& acc)
{
    return quotes.NewClient(exch,acc);
}

StockFighterReqHdl StockFighterConnection::PostOrder(const std::string& venue,
                                                     const std::string& symbol,
                                                     const std::string& order)
{
    std::string url = apiBase + "venues/" + venue + "/stocks/";
    url += symbol + "/orders";

    return requestThread.HTTPSPOST(
               server,
               url,
               order,
               {
                   {"X-Starfighter-Authorization",authKey}
               });
}
OrderPostReqHdl StockFighterConnection::PostOrder(const OrderDefinition& order)
{
    OrderPostReqHdl hdl (new OrderPostRequest(*this,order));

    return hdl;
}
