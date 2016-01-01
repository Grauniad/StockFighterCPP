#include "ExchangeInterface.h"
#include <SimpleJSON.h>
#include <logger.h>

namespace Parsers{
    NewBoolField(ok);

    NewObjectArray(symbols, Stock::Config);

    typedef SimpleParsedJSON<ok, symbols> StocksResponse;
}

Exchange::Exchange(StockFighterAPI& conn, const VenueConfig& exch)
    : ready(false),
      ok(false),
      exchName(exch.DisplayName()),
      exchId(exch.ExchangeCode()),
      api(conn),
      stocksRequest(nullptr)
{
    stocksRequest = api.GetStocks(exchId);
}

bool Exchange::Ready() const {
    return ready;
}

bool Exchange::OK() const {
    return ok;
}

bool Exchange::WaitForInitialisation() {
    if ( !ready)  {
        auto& msg = stocksRequest->WaitForMessage();
        ParseStocks(msg.content.str());
        ready = true;
    }
    return OK();
}

void Exchange::ParseStocks(const std::string& response) {
    Parsers::StocksResponse parser;
    std::string error;
    ok = parser.Parse(response.c_str(),error);

    if (ok) {
        SLOG_FROM(LOG_VERBOSE,"Exchange::ParseStocks",
           "Configuring exchange from : " << parser.GetPrettyJSONString());
        ok = parser.Get<Parsers::ok>();
        if ( ok ) {
            auto& stocksArr = parser.Get<Parsers::symbols>();
            stocks.reserve(stocksArr.size());
            for (auto& stockCfg: stocksArr) {
                stocks.emplace_back(*this,*stockCfg);
                // TODO, this sucks. We are only getting away with it
                //       because of the reserve above. This is fragile and 
                //       should be fixed with an interface change.
                Stock* stock = &stocks.back();
                stockMap[stock->Ticker()] = stock;
            }
        }
    } else {
        SLOG_FROM(LOG_ERROR,"Exchange::ParseStocks",
           "Failed to parse server response: " << error);
    }
}

const Stock* Exchange::GetStock(const std::string& symbol) {
    Stock* stock = nullptr;

    auto it = stockMap.find(symbol);

    if ( it != stockMap.end()) {
        stock = it->second;
    }

    return stock;
}
