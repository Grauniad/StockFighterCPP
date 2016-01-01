#ifndef STOCK_FIGHTER_SERVER_LEVEL_CONFIG_H__
#define STOCK_FIGHTER_SERVER_LEVEL_CONFIG_H__

#include <vector>
#include <AccountConfig.h>
#include <memory>
#include <VenueConfig.h>
#include <StockFighterConfig.h>
#include <ClientConfig.h>
#include <istream>

class LevelConfig {
public:
    LevelConfig(const std::string& config);

    const StockFighterConfig& ApplicationConfig() const;

    const ClientConfig& ClientConnectionConfig() const;

    std::vector<VenueConfig>& VenuesConfig() { return venues; } 

    std::vector<AccountConfig>& AccountsConfig() { return accounts; } 
    /**
     * Get the configuraion for the account with the specified code.
     *
     * If there is no account matching this code, the nullptr is returned
     */
    AccountConfig* GetAccountConfig(const std::string& code);

    bool OK () { return ok; }
private:
    /****************************************************
     *            Configuration Utilities
     ****************************************************/
     void InitialiseConfigObjects();

     void InitialiseStaticConfig();

     void InitialiseAccountsConfig();

     void InitialiseVenuesConfig();

    /****************************************************
     *            Config Parsing Code
     ****************************************************/

    NewObjectArray(Accounts, AccountConfig::Config);
    NewObjectArray(Venues, VenueConfig::Config);
    NewEmbededObject(StockFighter, StockFighterConfig::Config);
    NewEmbededObject(Client, ClientConfig::Config);
    typedef SimpleParsedJSON<
                Accounts,
                Venues,
                StockFighter,
                Client
             > Parser;

    Parser parser;

    /****************************************************
     *               Config Values
     ****************************************************/

    std::vector<AccountConfig>            accounts;
    std::map<std::string,AccountConfig*>  accountMap;
    std::vector<VenueConfig>              venues;
    std::unique_ptr<StockFighterConfig>   stock_fighter;
    std::unique_ptr<ClientConfig>         client;

    /****************************************************
     *               State Data
     ****************************************************/
    bool ok;
};

class LevelConfigFile: public LevelConfig {
public:
    LevelConfigFile(const std::string&fname);
private:
    static const std::string FileContents(const std::string& fname);

};

#endif
