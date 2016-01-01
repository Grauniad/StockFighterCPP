#include "Stock.h" 

#include <ExchangeInterface.h>


Stock::Stock(Exchange& exch, Config& cfg) 
    : exchange(exch),
      displayName(cfg.Get<name>()),
      ticker(cfg.Get<symbol>())
{
}
