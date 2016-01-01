#include "AccountConfig.h"

AccountConfig::AccountConfig(AccountConfig::Config& cfg)
    : id(cfg.Get<Code>()),
      name(cfg.Get<NameField>())
{
}

const std::string& AccountConfig::DisplayName() const {
    return name;
}

const std::string& AccountConfig::AccountCode() const {
    return id;
}
