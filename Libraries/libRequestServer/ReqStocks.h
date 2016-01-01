/*
 * ReqStocks.h
 *
 *  Created on: 21 Dec 2015
 *      Author: lhumphreys
 */

#ifndef __STOCK_FIGHTER_LIBRARIES_REQUEST_SERVER_STOCKS_H__
#define __STOCK_FIGHTER_LIBRARIES_REQUEST_SERVER_STOCKS_H__

#include "ReqServer.h"
#include <SimpleJSON.h>
#include <Exchanges.h>

/*
 * Request the stocks available for use.
 *
 * REQUEST:
 *    {
 *        exchange: "Exchange ID",
 *    }
 *
 * REPLY:
 *    {
 *         stocks: [
 *             {
 *                 name: "Display Name",
 *                 id:   "Unique identifier"
 *             },
 *             ...
 *         ]
 *    }
 */
class ReqStocks: public RequestReplyHandler { public:
    /**
     * Create a new instance of the stocks request handler.
     * 
     * NOTE: The cfg object must be valid for the lifetimee 
     *       of the server
     * 
     * @param cfg   The stock configuration. 
     */
    ReqStocks(Exchanges& data);

    virtual std::string OnRequest(const char* req);

    virtual ~ReqStocks();
private:
    void Reset();
    void Setup(const char* req);
    void BuildReply();

    /*******************************************
     *          Message Definitions
     *******************************************/

    NewStringField(exchange);
    typedef SimpleParsedJSON<exchange> Request;

    NewStringField(id);
    NewStringField(name);

    typedef SimpleParsedJSON<
        id,
        name
    > StockJSON;
    NewObjectArray(stocks, StockJSON);

    typedef SimpleParsedJSON<stocks> Reply;

    /*******************************************
     *          Data
     *******************************************/
    Request request;
    Reply   reply;

    Exchanges& exchangeInterface;
    Exchange*  venue;
};

#endif /* DEV_TOOLS_CPP_LIBRARIES_LIBWEBSOCKETS_REQFILELIST_H_ */
