#include "LevelConfig.h"
#include <fstream>
#include <logger.h>

using namespace std;

LevelConfig::LevelConfig(const std::string& config) 
   : stock_fighter(nullptr), 
     client(nullptr),
     ok(false)
{
    std::string error;
    ok = parser.Parse(config.c_str(),error);

    if ( ok ) {
        InitialiseConfigObjects();

        SLOG_FROM(LOG_VERBOSE, "LevelConfig", 
           "Read configuration: " << parser.GetPrettyJSONString());
    } else {
        SLOG_FROM(LOG_ERROR, "LevelConfig", 
           "Invalid configuration: " << error);
    }
}

LevelConfigFile::LevelConfigFile(const std::string& fname) 
   : LevelConfig(FileContents(fname))
{
}

const std::string LevelConfigFile::FileContents(const std::string& fname) {
    std::ifstream config_file(fname);
    return std::string((std::istreambuf_iterator<char>(config_file)),
                        std::istreambuf_iterator<char>());
}

void LevelConfig::InitialiseConfigObjects() {
    InitialiseStaticConfig();

    InitialiseAccountsConfig();

    InitialiseVenuesConfig();
}

void LevelConfig::InitialiseStaticConfig() {
    stock_fighter.reset(new StockFighterConfig(parser.Get<StockFighter>()));
    client.reset(new ClientConfig(parser.Get<Client>()));
}

void LevelConfig::InitialiseAccountsConfig() {
    auto& cfg_accounts = parser.Get<Accounts>();
    accounts.reserve(cfg_accounts.size());
    for (auto& p_acc: cfg_accounts) {
        accounts.emplace_back(*p_acc);
        // TODO: This sucks, fix it. We're only getting away with this because
        //       we have reservered the space in std::vector, so it won't 
        //       resize.
        AccountConfig* pacc = &accounts.back();
        accountMap[pacc->AccountCode()] = pacc;
    }
}

AccountConfig* LevelConfig::GetAccountConfig(const std::string& code) {
    AccountConfig* acc = nullptr;
    auto it = accountMap.find(code);

    if (it != accountMap.end()) {
        acc = it->second;
    }

    return acc;
}

void LevelConfig::InitialiseVenuesConfig() {
    auto& cfg_venues = parser.Get<Venues>();
    venues.reserve(cfg_venues.size());
    for (auto& p_acc: cfg_venues) {
        venues.emplace_back(*p_acc);
    }
}

const StockFighterConfig& LevelConfig::ApplicationConfig() const {
    return *stock_fighter;
}

const ClientConfig& LevelConfig::ClientConnectionConfig() const {
    return *client;
}
