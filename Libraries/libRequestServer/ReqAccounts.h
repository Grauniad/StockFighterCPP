/*
 * ReqAccounts.h
 *
 *  Created on: 21 Dec 2015
 *      Author: lhumphreys
 */

#ifndef __STOCK_FIGHTER_LIBRARIES_REQUEST_SERVER_ACCOUNTS_H__
#define __STOCK_FIGHTER_LIBRARIES_REQUEST_SERVER_ACCOUNTS_H__

#include "ReqServer.h"
#include <SimpleJSON.h>
#include <LevelConfig.h>

/*
 * Request the accounts available for use.
 *
 * REQUEST:
 *    {
 *    }
 *
 * REPLY:
 *    {
 *         accounts: [
 *             {
 *                 name: "Display Name",
 *                 id:   "Unique identifier"
 *             },
 *             ...
 *         ]
 *    }
 */
class ReqAccounts: public RequestReplyHandler {
public:
    /**
     * Create a new instance of the accounts request handler.
     * 
     * NOTE: The cfg object must be valid for the lifetimee 
     *       of the server
     * 
     * @param cfg   The account configuration. 
     */
    ReqAccounts(LevelConfig& cfg);

    virtual std::string OnRequest(const char* req);

    virtual ~ReqAccounts();
private:
    NewStringField(id);
    NewStringField(name);

    typedef SimpleParsedJSON<
        id,
        name
    > Account;
    NewObjectArray(accounts, Account);

    typedef SimpleParsedJSON< accounts > Reply;

    Reply   reply;

    LevelConfig& cfg;
};

#endif /* DEV_TOOLS_CPP_LIBRARIES_LIBWEBSOCKETS_REQFILELIST_H_ */
