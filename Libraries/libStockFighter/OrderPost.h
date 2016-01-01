#ifndef STOCK_FIGHTER_API_POST_ORDER_H__
#define STOCK_FIGHTER_API_POST_ORDER_H__

class StockFighterAPI;

#include <http_request.h>
#include <functional>
#include <atomic>
#include <future>
#include <mutex>
#include <enum_value.h>

struct OrderDefinition {

    /************************************************************
     *                    Type Definitions
     ************************************************************/
    enum DIRECTION {
        DIRECTION_BUY,
        DIRECTION_SELL,
        DIRECTION_ERROR
    };
    typedef EnumValue<DIRECTION,DIRECTION_ERROR> Direction;

    enum ORDER_TYPE {
        ORDER_TYPE_LIMIT,
        ORDER_TYPE_MARKET,
        ORDER_TYPE_FILL_OR_KILl,
        ORDER_TYPE_IMMEDIATE_OR_CANACEL,
        ORDER_TYPE_ERROR
    };
    typedef EnumValue<ORDER_TYPE,ORDER_TYPE_ERROR> OrderType;

    /************************************************************
     *                    Data
     ************************************************************/

    std::string account;
    std::string symbol;
    std::string venue;
    OrderType   type;
    Direction   direction;
    long        originalQuantity;
    long        limitPrice;

    /************************************************************
     *                    Initialisation
     ************************************************************/

    OrderDefinition();
};

class OrderPostRequest {
public:
    OrderPostRequest(StockFighterAPI& api,
                     OrderDefinition order);

    struct OrderResponse {
        OrderResponse();
        bool        ok;
        std::string error; // Error text, if any
        size_t      exchangeOrderId;
    };

    /**
     * Block until the exchange has responded to our request.
     */
    const OrderResponse& WaitForResponse();

    /**
     * Trigger a call-back on the IO thread when the exchange has responded.
     *
     * (This would usually be used to post a task back to the request thread)
     */
    typedef std::function<void (const OrderResponse& response)> Notification;
    void OnResponse(const Notification& notification);

private:
    static std::string GetJSONRequest(const OrderDefinition& order);
    static void ParseResponse(const std::string& message, OrderResponse& response);

    /**
     * Data cannot move, otherwise callback task will be malformed
     */
     OrderPostRequest(const OrderPostRequest& rhs) = delete;;
     OrderPostRequest(OrderPostRequest&& rhs) = delete;;
     OrderPostRequest& operator=(const OrderPostRequest& rhs) = delete; 
     OrderPostRequest& operator=(OrderPostRequest&& rhs) = delete; 

    /**
     * Data
     */
    std::shared_ptr<HTTPMessageProvider> postRequest;
    OrderResponse      response;

    std::promise<int> statusFlag;
    std::future<int>  statusFuture;

    std::atomic<bool> ready;

    // Data only to be modified under mutex lock
    std::mutex        notifyMutex;
    std::atomic<bool> notify;
    Notification      notification;
};

typedef std::shared_ptr<OrderPostRequest> OrderPostReqHdl; 

#endif
