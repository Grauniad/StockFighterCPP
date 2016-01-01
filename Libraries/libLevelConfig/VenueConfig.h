#ifndef STOCK_FIGHTER_SERVER_VENUE_CONFIG_H__
#define STOCK_FIGHTER_SERVER_VENUE_CONFIG_H__
#include <string>
#include <SimpleJSON.h>

class VenueConfig {
public:
    NewStringField(Code);

    struct NameField: public StringField  { 
        const char * Name() { 
            return "Name"; 
        } 
    };

    typedef SimpleParsedJSON<Code,NameField> Config;

    VenueConfig(Config& cfg);
    VenueConfig(VenueConfig&& move_from) = default;

    const std::string& DisplayName() const;
    const std::string& ExchangeCode() const;

private:
   const std::string id;
   const std::string name;
};

#endif
