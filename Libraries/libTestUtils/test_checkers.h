#ifndef __STOCK_FIGHTER_LIBS_TESTING_API_CHECKERS_H__
#define __STOCK_FIGHTER_LIBS_TESTING_API_CHECKERS_H__

#include <tester.h>
#include <MarketDataProvider.h>
#include <LevelConfig.h>
#include <ReqServer.h>
#include <ExchangeOrder.h>

namespace TestChckers {
    /**********************************************************************
     *                      Market Data
     *********************************************************************/ 
    /**
     * Validate that a market data update published by the market data provider
     * matches a quote received by the quote subscriber.
     */
    bool QuotesEqual(testLogger& log,
                     const QuoteHandler::Quote& q1,
                     const MarketData& q2);

    /**
     * Validate that a market data client has EXACTLY the updates
     * defined in the quotes array.
     */
    bool CheckClient(testLogger& log,
                     const std::string& name,
                     MarketDataClient client, 
                     std::vector<QuoteHandler::Quote> quotes);
    /**********************************************************************
     *                      Request Server
     *********************************************************************/ 
    /**
     * Validate that a request to a subscription handler is rejected,
     * with the correct error and code.
     */
    bool SubscriptionIsRejected(testLogger& log,
                     SubscriptionHandler& handler,
                     const std::string& request,
                     const std::string& reason,
                     const int errorCode);

    /**********************************************************************
     *               Exchange Orders: Core Details
     *********************************************************************/ 

    bool ValidateOrderDetails(testLogger& log,
                              OrderDefinition& order,
                              ExchangeOrder& e_order,
                              size_t iid,
                              size_t eid = 0,
                              const std::string& rejectReason = "");

    /**********************************************************************
     *               Exchange Orders: Status Fields
     *********************************************************************/ 


    bool ValidateDraftOrder(testLogger& log, ExchangeOrder& e_order);

    bool ValidateSentOrder(testLogger& log, ExchangeOrder& e_order);

    bool ValidateAcceptedOrder(testLogger& log, ExchangeOrder& e_order, size_t eid);

    bool ValidateRejectedOrder(testLogger& log,
                               ExchangeOrder& e_order,
                               const std::string& reason);

    bool ValidateSystemRejectedOrder(
             testLogger& log,
             ExchangeOrder& e_order,
             const std::string& reason);
    /**********************************************************************
     *               Exchange Orders: Status Updates
     *********************************************************************/ 

    bool ValidateUnfilledOrderUpdate(testLogger& log,
                                     OrderUpdateClient& client,
                                     OrderStatus::OrderState oldState,
                                     OrderStatus::OrderState newState,
                                     long   qty,
                                     size_t internal_id,
                                     size_t external_id = 0);

    bool ValidateUpdatesClientEmpty(testLogger& log,OrderUpdateClient& client);
}

#endif
