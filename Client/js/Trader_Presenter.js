/*global $, document, NavBar, Application, extend, console, self */

/*
 * The presenter is responsible for handling "actions" by the user, 
 * and apply their resulting changes to the model.
 *
 * It is also responsible for handling the "start" event fired from the application
 */
var Start_Presenter = {
    loading: false,
    /********************************************************************
     *                         Installation
     ********************************************************************/
    
    start: function () {
        "use strict";
        // Application started...
    },
    
    /********************************************************************
     *                         Accounts
     ********************************************************************/
    
    accountListChanged: function () {
        "use strict";
        Application.view.setTradingAccounts(
            Application.model.getAccounts()
        );
    },

    accountChanged: function (account) {
        "use strict";
        Application.model.setAccount(account);
    },

    /********************************************************************
     *                         Venues
     ********************************************************************/

    venueListChanged: function () {
        "use strict";
        Application.view.setTradingVenues(
            Application.model.getVenues()
        );
    },

    venueChanged: function (venue) {
        "use strict";
        Application.model.setVenue(venue);
    },

    /********************************************************************
     *                         Stocks
     ********************************************************************/

    stockListChanged: function () {
        "use strict";
        Application.view.setTradingStocks(
            Application.model.getStocks()
        );
    },

    stockChanged: function (stock) {
        "use strict";
        Application.model.setStock(stock);
    },

    /********************************************************************
     *                         Market Data
     ********************************************************************/

    updateMarketData: function (marketData) {
        "use strict";
        Application.view.updateMarketData(marketData);
    },

    /********************************************************************
     *                         New Order
     ********************************************************************/

    newOrderButton: function () {
        "use strict";
        var details = Application.view.getOrderDetails();
        Application.model.reqNewOrder(details.side,
                                      details.type,
                                      details.price,
                                      details.quantity);
    },
};
