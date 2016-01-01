#ifndef __STOCK_FIGHTER_LIBRARIES_REQUEST_SERVER_TRADER_H__
#define __STOCK_FIGHTER_LIBRARIES_REQUEST_SERVER_TRADER_H__

#include <ReqServer.h>
#include <LevelConfig.h>
#include <Exchanges.h>
#include <IPostable.h>
#include <OrderActionHandler.h>

class MarketDataManager;

/**
 * Responsible for handling all requests from the trader client.
 * 
 * The TraderSerer runs its own event loop, and all data interrogation will be
 * done from the thread which called Run();
 */
class TraderServer: public IPostable {
public:
    TraderServer(Exchanges& exchanges,LevelConfig& cfg);

    /**
     * Start listening on the configured port and handle any incoming
     * connections.
     * 
     * MOTE: This function drops into its own event loop
     */
    void Run();

    void PostTask(const RequestServer::InteruptHandler& f);

    /**
     * Install handlers
     */
    void InstallRequestHandlers(
             LevelConfig& cfg,
             MarketDataManager& mgr,
             OrderActionHandler& actionHandler);
private:

    const short     listenPort;
    RequestServer   requestServer;
    Exchanges&      exchangeInterface;
};

#endif 
