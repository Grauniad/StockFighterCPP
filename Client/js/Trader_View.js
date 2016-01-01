/*global $, document, navbar, Select, Application, alertify, Spinner, Logging */

var Start_View = {
    /********************************************************************
     *                         View Interface
     ********************************************************************/
    setup: function () {
        "use strict";
        this.initialiseAccountControl();
        this.initialiseVenueControl();
        this.initialiseStockControl();
        this.initialiseTradeButton();
        Application.viewCreated();
    },
    
    /********************************************************************
     *                         Accessors
     ********************************************************************/
    
    getTradeButton: function () {
        "use strict";
        return Select.selectExactly("Trader Page - Trade Button", "#trader-trader-button", 1);
    },

    getAccountsControl: function() {
        "uses strict";

        return Select.selectExactly("Trader Page - Accounts control", "#account-selector", 1);
    },

    getVenuesControl: function() {
        "uses strict";

        return Select.selectExactly("Trader Page - Venues control", "#venue-selector", 1);
    },

    getStocksControl: function() {
        "uses strict";

        return Select.selectExactly("Trader Page - Stocks control", "#stock-selector", 1);
    },

    getBidPriceSpan: function() {
        "uses strict";

        return Select.selectExactly("Trader Page - Bid Price", "span.market-data-title-bid", 1);
    },

    getAskPriceSpan: function() {
        "uses strict";

        return Select.selectExactly("Trader Page - Bid Price", "span.market-data-title-ask", 1);
    },

    getLastPriceSpan: function() {
        "uses strict";

        return Select.selectExactly("Trader Page - Last Price", "span.market-data-title-last", 1);
    },

    getSideControl: function() {
        "uses strict";

        return Select.selectExactly("Trader Page - Side Selector", "#side-selector", 1);
    },

    getTypeControl: function() {
        "uses strict";

        return Select.selectExactly("Trader Page - Type Selector", "#type-selector", 1);
    },

    getQuantityField: function() {
        "uses strict";

        return Select.selectExactly("Trader Page - Quantity Field", "#qty-field", 1);
    },

    getPriceField: function() {
        "uses strict";

        return Select.selectExactly("Trader Page - Price Field", "#price-field", 1);
    },
    
    /********************************************************************
     *                         Accounts
     ********************************************************************/

    initialiseAccountControl: function() {
        "use strict";

        var control = this.getAccountsControl();

        control.on("change",function () {
            Application.presenter.accountChanged(control.val());
        });
    },
    
    setTradingAccounts: function (accounts) {
        "use strict";
        var i = 0, control = null;
        control = this.getAccountsControl();
        control.empty();
        for (i = 0; i < accounts.length; i = i +1) {
            this.addAccount(accounts[i]);
        }
        Application.presenter.accountChanged(control.val());
    },
    
    addAccount: function (acc) {
        "use strict";
        var accEl = $("<option></option>");
        accEl.val(acc.id);
        accEl.text(acc.name);
        this.getAccountsControl().append(accEl);

    },

    /********************************************************************
     *                         Venues
     ********************************************************************/
    
    initialiseVenueControl: function() {
        "use strict";

        var control = this.getVenuesControl();

        control.on("change",function () {
            Application.presenter.venueChanged(control.val());
        });
    },

    setTradingVenues: function (venues) {
        "use strict";
        var i = 0, control = this.getVenuesControl();
        control.empty();
        for (i = 0; i < venues.length; i = i +1) {
            this.addVenue(venues[i]);
        }
        Application.presenter.venueChanged(control.val());
    },
    
    addVenue: function (acc) {
        "use strict";
        var accEl = $("<option></option>");
        accEl.val(acc.id);
        accEl.text(acc.name);
        this.getVenuesControl().append(accEl);

    },

    /********************************************************************
     *                         Stocks
     ********************************************************************/
    
    initialiseStockControl: function() {
        "use strict";

        var control = this.getStocksControl();

        control.on("change",function () {
            Application.presenter.stockChanged(control.val());
        });
    },

    setTradingStocks: function (stocks) {
        "use strict";
        var i = 0, control = this.getStocksControl();
        control.empty();
        for (i = 0; i < stocks.length; i = i +1) {
            this.addStock(stocks[i]);
        }
        Application.presenter.stockChanged(control.val());
    },
    
    addStock: function (stk) {
        "use strict";
        var stkEl = $("<option></option>");
        stkEl.val(stk.id);
        stkEl.text(stk.name);
        this.getStocksControl().append(stkEl);
    },

    /********************************************************************
     *                         Order Form
     ********************************************************************/

    initialiseTradeButton: function() {
        "use strict";

        var control = this.getTradeButton();

        control.on("click",function () {
            Application.presenter.newOrderButton();
        });
    },

    getOrderDetails: function () {
        "use strict";
        var details = {
            side: this.getSideControl().val(),
            type: this.getTypeControl().val(),
            price: parseInt(this.getPriceField().val()),
            quantity: parseInt(this.getQuantityField().val())
        };

        // Intercept NaNs
        if ( details.price !== details.price ) {
            details.price = -1;
        }

        if ( details.quantity !== details.quantity ) {
            details.quantity = -1;
        }

        return details;
    },

    /********************************************************************
     *                         Market Data
     ********************************************************************/

    updateMarketData: function (marketData) {
        "use strict";
        this.getLastPriceSpan().text(marketData.lastPrice);
        this.getBidPriceSpan().text(marketData.bidPrice);
        this.getAskPriceSpan().text(marketData.askPrice);
    },

};
