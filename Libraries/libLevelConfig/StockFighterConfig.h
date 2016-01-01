#ifndef STOCK_FIGHTER_SERVER_STOCK_FIGHTER_CONFIG_H__
#define STOCK_FIGHTER_SERVER_STOCK_FIGHTER_CONFIG_H__
#include <string>
#include <SimpleJSON.h>

class StockFighterConfig {
public:
    NewStringField(authKey);
    NewStringField(apiServer);
    NewStringField(apiServerBaseUrl);

    typedef SimpleParsedJSON<
                authKey,
                apiServer,
                apiServerBaseUrl
            > Config;

    StockFighterConfig(Config& cfg);
    StockFighterConfig(StockFighterConfig&& move_from) = default;

    const std::string& AuthKey() const;

    const std::string& APIServer() const;
    const std::string& APIBaseUrl() const;

private:
   const std::string apiKey;
   const std::string server;
   const std::string base_url;
};

#endif
