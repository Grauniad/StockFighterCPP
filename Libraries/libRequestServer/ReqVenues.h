/*
 * ReqVenues.h
 *
 *  Created on: 21 Dec 2015
 *      Author: lhumphreys
 */

#ifndef __STOCK_FIGHTER_LIBRARIES_REQUEST_SERVER_VENUES_H__
#define __STOCK_FIGHTER_LIBRARIES_REQUEST_SERVER_VENUES_H__

#include "ReqServer.h"
#include <SimpleJSON.h>
#include <LevelConfig.h>

/*
 * Request the venues available for use.
 *
 * REQUEST:
 *    {
 *    }
 *
 * REPLY:
 *    {
 *         venues: [
 *             {
 *                 name: "Display Name",
 *                 id:   "Unique identifier"
 *             },
 *             ...
 *         ]
 *    }
 */
class ReqVenues: public RequestReplyHandler {
public:
    /**
     * Create a new instance of the venues request handler.
     * 
     * NOTE: The cfg object must be valid for the lifetimee 
     *       of the server
     * 
     * @param cfg   The venue configuration. 
     */
    ReqVenues(LevelConfig& cfg);

    virtual std::string OnRequest(const char* req);

    virtual ~ReqVenues();
private:
    NewStringField(id);
    NewStringField(name);

    typedef SimpleParsedJSON<
        id,
        name
    > Venue;
    NewObjectArray(venues, Venue);

    typedef SimpleParsedJSON< venues > Reply;

    Reply   reply;

    LevelConfig& cfg;
};

#endif /* DEV_TOOLS_CPP_LIBRARIES_LIBWEBSOCKETS_REQFILELIST_H_ */
