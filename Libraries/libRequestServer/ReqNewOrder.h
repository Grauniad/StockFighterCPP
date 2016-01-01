/*
 * ReqStocks.h
 *
 *  Created on: 31 Dec 2015
 *      Author: lhumphreys
 */

#ifndef __STOCK_FIGHTER_LIBRARIES_REQUEST_SERVER_NEW_ORDER_H__
#define __STOCK_FIGHTER_LIBRARIES_REQUEST_SERVER_NEW_ORDER_H__

#include <OrderActionHandler.h>
#include <ReqServer.h>
#include <SimpleJSON.h>

/*
 * Request the creation of a new order
 *
 * REQUEST:
 *    {
 *        account:      "Account ID",
 *        exchange:     "Exchange ID",
 *        stock:        "Symbol",
 *        type:         "limit",
 *        direction:    "buy",
 *        quantity:     100,
 *        limit_price:  100
 *    }
 *
 * REPLY (ok):
 *    {
 *         ok: true,,
 *         order_id: 2
 *    }
 */
class ReqNewOrder: public RequestReplyHandler {
public:
    /**
     * Create a new instance of the new order request
     * 
     */
    ReqNewOrder(OrderActionHandler& actionHandler);

    virtual std::string OnRequest(const char* req);

    virtual ~ReqNewOrder();
private:
    void Reset();
    void Setup(const char* req, OrderDefinition& order);

    OrderHdl CreateOrder(OrderDefinition& order); 

    std::string BuildReply(OrderHdl order);

    /*******************************************
     *          Message Definitions
     *******************************************/

    NewStringField(account);
    NewStringField(direction);
    NewStringField(exchange);
    NewI64Field(limit_price);
    NewI64Field(quantity);
    NewStringField(stock);
    NewStringField(type);
    typedef SimpleParsedJSON<
         account,
         direction,
         exchange,
         limit_price,
         quantity,
         stock,
         type
    > Request;

    NewBoolField(ok);
    NewI64Field(order_id);

    typedef SimpleParsedJSON<
        ok,
        order_id
    > Reply;

    /*******************************************
     *          Data
     *******************************************/
    Request request;
    Reply   reply;

    OrderActionHandler&  actionHandler;
};

#endif
