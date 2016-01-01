#include <OrderActionHandler.h>

OrderActionHandler::OrderActionHandler(OrderManager& mgr)
   : manager(mgr)
{
}

OrderHdl OrderActionHandler::NewOrder(const OrderDefinition& defn,
                                      std::string& error)
{
    OrderHdl order(manager.NewOrder(defn));

    if ( order ) {
        bool ok = true;
        for (auto it = validators.begin(); ok && it != validators.end(); ++it)
        {
            const ValidatorHdl& validator = *it;
            ok = ok && validator->ValidateDraftOrder(*order,error);
        }
        
        if ( ok ) {
            order->SendOrder();
        } else {
            order->SystemReject(error);
        }
    } else {
         error =  "Account '";
         error += defn.account + "'";
         error += " may not trade '";
         error += defn.symbol + "'"; 
         error += " on '";
         error += defn.venue + "'";
    }

    return order;
}

void OrderActionHandler::InstallValidator(ValidatorHdl& validator) {
    validators.insert(validator);
}
