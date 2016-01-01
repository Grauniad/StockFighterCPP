#ifndef STOCK_FIGHTER_SERVER_CLIENT_CONFIG_H__
#define STOCK_FIGHTER_SERVER_CLIENT_CONFIG_H__
#include <string>
#include <SimpleJSON.h>

class ClientConfig {
public:
    NewIntField(port);

    typedef SimpleParsedJSON<
                port
            > Config;

    ClientConfig(Config& cfg);
    ClientConfig(ClientConfig&& move_from) = default;

    short ListenPort() const;

private:
   const short listenPort;
};

#endif
