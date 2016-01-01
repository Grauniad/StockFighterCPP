#ifndef STOCK_FIGHER_LIBRARIES_ORDERS_VALIDATORS_H__
#define STOCK_FIGHER_LIBRARIES_ORDERS_VALIDATORS_H__

#include <LevelConfig.h>
#include <Exchanges.h>
#include <order.h>
#include <ExchangeOrder.h>
#include <string>

/**
 *  Exchange order validation interface.
 *
 *  By default all checks returns true, concrete implementations 
 *  should implement their own checks.
 */
class ExchangeOrderValidator {
public:
    /**
     * 
     */
    virtual bool ValidateDraftOrder(const ExchangeOrder& order,
                                    std::string& error) const;
};

/**
 * Basic, stateless, validator verifying actions can be 
 * performed on an ExchangeOrder.
 *
 */
class BasicExchangeOrderValidator: public ExchangeOrderValidator {
public:

     BasicExchangeOrderValidator(LevelConfig& levelConfig,
                                 Exchanges&   exchanges);

    /**
     * @param order   The order to validate.
     * @param error   Will be populated with a reason if validation fails
     *
     * @returns true if the order is ok, false otherwise
     */
    virtual bool ValidateDraftOrder(const ExchangeOrder& order,
                                    std::string& error) const;
private:
    /** 
     * Validate that the account specified on an order is valid.
     */
    bool ValidateAccount(const OrderDetails& order, std::string& error) const;

    /**
     * Validate that the exchange specified on an order is valid.
     */
    bool ValidateExchange(const OrderDetails& order, std::string& error) const;

    /**
     * Validate that the stock is valid for this exchange...
     */
    bool ValidateStock(const OrderDetails& order,
                       Exchange&           exchange,
                       std::string&        error) const;

    /**
     *  Validate the order type is ok.
     */
    bool ValidateOrderType(const OrderDetails& order,
                           std::string&        error) const;

    /**
     *  Validate the direction is ok.
     */
    bool ValidateDirection(const OrderDetails& order,
                           std::string&        error) const;

    /**
     *  Validate the quantity is ok.
     */
    bool ValidateQuantity(const OrderDetails& order,
                          std::string&        error) const;

    /**
     *  Validate the limit price is ok.
     */
    bool ValidateLimitPrice(const OrderDetails& order,
                            std::string&        error) const;

    LevelConfig& config;
    Exchanges&   exchanges;
};

#endif
