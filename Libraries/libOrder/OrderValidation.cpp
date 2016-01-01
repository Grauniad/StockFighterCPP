#include "OrderValidation.h"
#include <sstream>
/********************************************************
 *               Interface
 ******************************************************/
bool ExchangeOrderValidator::ValidateDraftOrder(
         const ExchangeOrder& order,
         std::string& error) const
{
    return true;
}

/********************************************************
 *               "Basic" Validator
 ******************************************************/

BasicExchangeOrderValidator::BasicExchangeOrderValidator(
        LevelConfig& levelConfig,
        Exchanges&   _exchanges)
    : config(levelConfig),
      exchanges(_exchanges)
{
}

bool BasicExchangeOrderValidator::ValidateDraftOrder(const ExchangeOrder& order,
                                                     std::string& error) const
{
    bool ok = true;

    const OrderDetails& details = order.Details();

    ok = ok && ValidateAccount(details,error);

    ok = ok && ValidateExchange(details,error);

    ok = ok && ValidateOrderType(details,error);

    ok = ok && ValidateDirection(details,error);

    ok = ok && ValidateQuantity(details,error);

    ok = ok && ValidateLimitPrice(details,error);

    return ok;
}

bool BasicExchangeOrderValidator::ValidateAccount(const OrderDetails& order,
                                                  std::string& error) const
{
    bool ok = true;
    
    auto account = config.GetAccountConfig(order.account);

    if (account == nullptr) {
        ok = false;
        error = "Invalid account '";
        error += order.account;
        error += "' supplied.";
    }
    
    return ok;
}

bool BasicExchangeOrderValidator::ValidateExchange(const OrderDetails& order,
                                                  std::string& error) const
{
    bool ok = true;

    auto exchange = exchanges.GetExchange(order.venue);

    if (exchange == nullptr) {
        error = "Invalid exchange '";
        error += order.venue;
        error += "' supplied.";
        ok = false;
    } else {
        ok = ok && ValidateStock(order, *exchange, error);
    }

    
    return ok;
}

bool BasicExchangeOrderValidator::ValidateStock(
         const OrderDetails& order,
         Exchange&           exchange,
         std::string&        error) const
{
    bool ok = true;

    auto stock = exchange.GetStock(order.symbol);

    if (stock == nullptr) {
        error = "Invalid stock '";
        error += order.symbol;
        error += "' supplied.";
        ok = false;
    }
    
    return ok;
}


bool BasicExchangeOrderValidator::ValidateOrderType(
         const OrderDetails& order,
         std::string&        error) const
{
   bool ok = order.type.IsOk();

   if (!ok) {
       error = "Invalid order type supplied.";
   }

   return ok;
}

bool BasicExchangeOrderValidator::ValidateDirection(
         const OrderDetails& order,
         std::string&        error) const
{
   bool ok = order.direction.IsOk();

   if (!ok) {
       error = "Invalid direction supplied.";
   }

   return ok;
}

bool BasicExchangeOrderValidator::ValidateQuantity(
         const OrderDetails& order,
         std::string&        error) const
{
    bool ok = true;

    if (order.originalQuantity <= 0) {
        thread_local std::stringstream buf;
        buf.clear();
        buf.str("");
        buf << "Invalid order quantity (";
        buf << order.originalQuantity;
        buf << ") supplied.";
        error = buf.str();
        ok = false;
    }
    
    return ok;
}

bool BasicExchangeOrderValidator::ValidateLimitPrice(
         const OrderDetails& order,
         std::string&        error) const
{
    bool ok = true;

    if ( order.type == OrderDetails::ORDER_TYPE_MARKET ) {
        if ( order.limitPrice != 0 ) {
            error = "Limit price supplied for market order.";
            ok = false;
        }
    } else {
        if (order.limitPrice <= 0) {
            thread_local std::stringstream buf;
            buf.clear();
            buf.str("");
            buf << "Invalid limit price (";
            buf << std::setprecision(2) << (1.0 * order.limitPrice / 100.0);
            buf << ") supplied.";
            error = buf.str();
            ok = false;
        }
    }

    
    return ok;
}
