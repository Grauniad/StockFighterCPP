#include "SubscriptionRequestMock.h"

SubRequestMock::SubRequestMock(const std::string& req) 
    : request(req)
{
}

const char* SubRequestMock::RequestMessasge() {
    return request.c_str();
}

void SubRequestMock::SendMessage(const std::string& msg) {
    messages.push_front(msg);
}

bool SubRequestMock::GetNextMessage(std::string& msg) {
    bool found = false;

    if ( messages.size() > 0 ) {
        msg = messages.back();
        messages.pop_back();
        found = true;
    }

    return found;
}

void SubRequestMock::ClearMessages() {
    messages.clear();
}

SubRequestMock::Handle SubRequestMock::MakeHandle(const std::string& req) {
    Handle h(new SubRequestMock(req));

    return h;
}
