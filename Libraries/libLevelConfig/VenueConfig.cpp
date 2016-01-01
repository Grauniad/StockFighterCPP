#include "VenueConfig.h"

VenueConfig::VenueConfig(VenueConfig::Config& cfg)
    : id(cfg.Get<Code>()),
      name(cfg.Get<NameField>())
{
}

const std::string& VenueConfig::DisplayName() const {
    return name;
}

const std::string& VenueConfig::ExchangeCode() const {
    return id;
}
