#ifndef STOCK_FIGHTER_SERVER_ACCOUNT_CONFIG_H__
#define STOCK_FIGHTER_SERVER_ACCOUNT_CONFIG_H__
#include <string>
#include <SimpleJSON.h>

class AccountConfig {
public:
    NewStringField(Code);

    struct NameField: public StringField  { 
        const char * Name() { 
            return "Name"; 
        } 
    };

    typedef SimpleParsedJSON<Code,NameField> Config;

    AccountConfig(Config& cfg);
    AccountConfig(AccountConfig&& move_from) = default;

    const std::string& DisplayName() const;
    const std::string& AccountCode() const;

private:
   const std::string id;
   const std::string name;
};

#endif
