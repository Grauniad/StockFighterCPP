#include "StockFighterConfig.h"

StockFighterConfig::StockFighterConfig(StockFighterConfig::Config& cfg) 
    : apiKey(cfg.Get<authKey>()),
      server(cfg.Get<apiServer>()),
      base_url(cfg.Get<apiServerBaseUrl>())
{
}

const std::string& StockFighterConfig::AuthKey() const {
    return apiKey;
}

const std::string& StockFighterConfig::APIServer() const {
    return server;
}

const std::string& StockFighterConfig::APIBaseUrl() const {
    return base_url;
}

