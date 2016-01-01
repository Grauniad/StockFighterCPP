#ifndef STOCK_FIGHTER_EXCHANGE_ORDER_EXCHANGE_ORDER_H__
#define STOCK_FIGHTER_EXCHANGE_ORDER_EXCHANGE_ORDER_H__

#include <order.h>
#include <PipePublisher.h>
#include <PipeSubscriber.h>

class StockFighterAPI;

/**
 * Represents a single order to the exchangee.
 *
 * The public interface to the class should only be accessed from the request thread.
 */
class ExchangeOrder {
public:
    /**
     * Create a new draft order with the specified details.

     * @param api               API Implementation
     * @param event_loop        The event loop order responses should be 
     *                          processed on
     * @param internalOrderId   The new identifier for the order
     * @param order             The definition of the order
     */
    ExchangeOrder(
         StockFighterAPI&         api,
         IPostable&               event_loop,
         size_t                   internalOrderId,
         const OrderDefinition&   order);

    /**
     * Access to the current state of the order
     */
    const OrderStatus& Status() const;

    /**
     * Access to the order details
     */
    const OrderDetails& Details() const;
    /**
     * Send the order to the exchange.
     *
     * The order will be put in a sent state, where it will remain until the
     * exchange accepts or rejects the order.
     *
     * @returns true if the order was sent, false otherwise
     */
     bool SendOrder();

     /**
      * Do a system reject on the order.
      *
      * NOTE: System rejects may only be done on draft orders.
      *
      * @returns true if the order could be rejected, false otherwise.
      *
      */
     bool SystemReject(const std::string& reason);

     struct OrderUpdate {
         size_t internalOrderId;
         size_t exchangeOrderId;
         OrderStatus oldStatus;
         OrderStatus newStatus;
     };

     typedef std::shared_ptr<PipeSubscriber<OrderUpdate>> OrderUpdateClient;
     OrderUpdateClient NewUpdatesClient();

private:
    static OrderStatus TemplateDraftStatus(const OrderDefinition& order);

    void HandleResponse(const OrderPostRequest::OrderResponse& response);
    /**
     * Cannot move the order, otherwise callback will hit the wrong memory
     * location
     */
     ExchangeOrder(const ExchangeOrder& rhs) = delete;
     ExchangeOrder(ExchangeOrder&& rhs) = delete;
     ExchangeOrder& operator=(const ExchangeOrder& rhs) = delete;
     ExchangeOrder& operator=(ExchangeOrder&& rhs) = delete;

    StockFighterAPI& api; 
    IPostable&       event_loop;

    OrderDetails     details;
    OrderPostReqHdl  sendRequest;

    class StatusHandler {
    public:
         /**
          * Create a new status handler with the specified initial values.
          *
          * No update is published for the intiial state.
          */
         StatusHandler(const OrderStatus& initial);

         /**
          * Access to the current status
          */
         inline const OrderStatus& Get() const { return status; }

         /**
          * Update the current status, publishing the  update.
          */
         void Update(const OrderDetails& refFields,
                     const OrderStatus& newStatus);

         /**
          * Create a new subscriptioni to status updates.
          */
         OrderUpdateClient NewUpdatesClient();
    private:
         OrderStatus status;
         PipePublisher<OrderUpdate> publisher;
    };

    StatusHandler  status;
};

typedef ExchangeOrder::OrderUpdateClient  OrderUpdateClient;
typedef std::shared_ptr<ExchangeOrder> OrderHdl;

#endif
