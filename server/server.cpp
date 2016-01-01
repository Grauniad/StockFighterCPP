#include "LevelConfig.h"
#include <logger.h>
#include <iostream>
#include <io_thread.h>

#include <StockFighterConnection.h>
#include <Exchanges.h>
#include <TraderServer.h>
#include <MarketDataManager.h>
#include <OrderManager.h>
#include <OrderActionHandler.h>


using namespace std;

int main(int argc, const char *argv[])
{
    if ( argc != 2 ) {
        cout << "Usage: server level_config.json" << endl;
        return 1;
    }

    LevelConfigFile config(argv[1]);

    if ( !config.OK() ) {
        cout << "Failed to configure server correctly" << endl;
        return 1;
    }

    StockFighterConnection api(config.ApplicationConfig());
    Exchanges exchanges(api,config.VenuesConfig());
    TraderServer server(exchanges,config);
    MarketDataManager marketData(api,exchanges,server);
    OrderManager manager(api,server,config,exchanges);
    OrderActionHandler actionHandler(manager);
    OrderActionHandler::ValidatorHdl validator (
        new BasicExchangeOrderValidator(config,exchanges));
    actionHandler.InstallValidator(validator);
    server.InstallRequestHandlers(config,marketData,actionHandler);

    cout << "Server initialised, starting request handling..." << endl;
    server.Run();
    return 0;
}
