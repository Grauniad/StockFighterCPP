#ifndef __STOCK_FIGHTER_LIBS_TESTING_SUB_REQUEST_MOCK_H__
#define __STOCK_FIGHTER_LIBS_TESTING_SUB_REQUEST_MOCK_H__

#include <ReqServer.h>
#include <list>
#include <string>

class SubRequestMock: public SubscriptionHandler::SubRequest {
public:
    SubRequestMock(const std::string& req);

    /**
     * Implemented by default, but the derived class can override with 
     * an exception version if required.
     */
    struct InvalidCallToSendMessage { };
    virtual void SendMessage(const std::string& msg);

    struct InvalidCallToRequestMessage { };
    virtual const char* RequestMessasge();

    /**
     * Utility to pull messages sent via SendMessage off the queue.
     *
     * @param msg  Will be populated with the contents of the message
     *
     * @returns true if there was a message to get, false otherwise
     */
    virtual bool GetNextMessage(std::string& msg);

    /**
     * Clear the backlog of unread messages
     */
     virtual void ClearMessages();

     typedef std::shared_ptr<SubRequestMock> Handle;

     static Handle MakeHandle(const std::string& re);
private:
    std::string request;

    typedef std::list<std::string> MessageQueue;
    MessageQueue  messages;
};

#endif
