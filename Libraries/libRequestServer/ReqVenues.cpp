#include <ReqVenues.h>

ReqVenues::ReqVenues(LevelConfig& config)
    : cfg(config)
{
}

ReqVenues::~ReqVenues() {
}

std::string ReqVenues::OnRequest(const char* req) {
    reply.Clear();

    for ( VenueConfig& ven: cfg.VenuesConfig()) {
        reply.Get<venues>().emplace_back();

        auto& venue = *reply.Get<venues>().rbegin();

        venue->Get<id>() = ven.ExchangeCode();
        venue->Get<name>() = ven.DisplayName();

    }

    return reply.GetJSONString();
}
