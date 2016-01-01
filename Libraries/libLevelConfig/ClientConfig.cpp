#include "ClientConfig.h"

ClientConfig::ClientConfig(Config& cfg) 
   : listenPort(cfg.Get<port>())
{
}

short ClientConfig::ListenPort() const {
    return listenPort;
}
