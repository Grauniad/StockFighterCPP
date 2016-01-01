#include <ReqAccounts.h>

ReqAccounts::ReqAccounts(LevelConfig& config)
    : cfg(config)
{
}

ReqAccounts::~ReqAccounts() {
}

std::string ReqAccounts::OnRequest(const char* req) {
    reply.Clear();

    for ( AccountConfig& account: cfg.AccountsConfig()) {
        reply.Get<accounts>().emplace_back();

        auto& acc = *reply.Get<accounts>().rbegin();

        acc->Get<id>() = account.AccountCode();
        acc->Get<name>() = account.DisplayName();

    }

    return reply.GetJSONString();
}
