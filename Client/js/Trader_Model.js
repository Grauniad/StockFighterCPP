
/*global $, document, alert, Application, console, Logging */

/*
 * Code for interacting with the C++ Application before loading a file
 */
var StartPage = {
    
    /********************************************************************
     *                    Model Interface
     *           Implement methods on the model interface
     ********************************************************************/
    /*
     * Setup the model
     */
    initialise: function () {
        "use strict";
        this.initialiseRequestManager();
        this._requestAccounts();
        this._requestVenues();
        Application.modelReady();
    },
    
    /********************************************************************
     *                    Accounts
     ********************************************************************/
    
    accounts: [],
    selectedAccount: "",
    
    getAccounts: function () {
        "use strict";
        return this.accounts;
    },
    
    _requestAccounts: function () {
        "use strict";

        this.startRequest(
            "ReqTraderAccounts",
            { },
            this.onAccountsSuccess,
            this.onAccountsError
        );
    },
    
    onAccountsSuccess: function (response) {
        "use strict";
        var self;
        self = Application.model;
        
        self.accounts = response.accounts;
        Application.presenter.accountListChanged();
    },
    
    onAccountsError: function (code, reason) {
        "use strict";
        
        var self = null, msg = "Failed to get accounts because:\n" + reason;
        
        self = Application.model;
        
        Logging.warning("TraderServer_Model.onAccountsError", msg);
    },

    setAccount: function(account) {
        "use strict";
        this.selectedAccount = account;
        if (Logging.debugLogging) {

            Logging.log_debug_msg(
                "TraderServer_Model.setAccount",
                 "New exchange selected: " + account);
        }
    },

    /********************************************************************
     *                    Venues
     ********************************************************************/

    selectedVenue: "",
    venues: [],
    
    getVenues: function () {
        "use strict";
        return this.venues;
    },

    setVenue: function(venue) {
        "use strict";
        this.selectedVenue = venue;
        if (Logging.debugLogging) {

            Logging.log_debug_msg(
                "TraderServer_Model.setVenue",
                 "New exchange selected: " + venue);
        }

        this.updateStocks();
    },
    
    _requestVenues: function () {
        "use strict";

        this.startRequest(
            "ReqTraderVenues",
            { },
            this.onVenuesSuccess,
            this.onVenuesError
        );
    },
    
    onVenuesSuccess: function (response) {
        "use strict";
        var self;
        self = Application.model;
        
        self.venues = response.venues;
        Application.presenter.venueListChanged();
    },
    
    onVenuesError: function (code, reason) {
        "use strict";
        
        var self = null, msg = "Failed to get venues because:\n" + reason;
        
        self = Application.model;
        
        Logging.warning("TraderServer_Model.onVenuesError", msg);
    },

    /********************************************************************
     *                    Stocks
     ********************************************************************/

    selectedStock: "",
    stocks: [],
    currentStockSearch: null,
    nextStockSearch: null,
    
    getStocks: function () {
        "use strict";
        return this.stocks;
    },

    setStock: function(stock) {
        "use strict";
        this.selectedStock = stock;
        if (Logging.debugLogging) {

            Logging.log_debug_msg(
                "TraderServer_Model.setStock",
                 "New stock selected: " + stock);
        }

        this.updateMarketData();
    },

    updateStocks: function () {
        "use strict";
        if (this.currentStockSearch) {
            // a get currently in progress...
            if (this.selectedVenue !== this.currentStockSearch) {
                this.nextStockSearch = this.selectedVenue;
            }
        } else {
            this.nextStockSearch = this.selectedVenue;
        }
        
        this.startNextStocksRequest();
    },
    
    startNextStocksRequest: function () {
        "use strict";
        
        if (!this.currentStockSearch && this.nextStockSearch) {
            this.currentStockSearch = this.nextStockSearch;
            this.nextStockSearch = null;
            
            this.startRequest(
                "ReqTraderStocks",
                {
                    exchange: this.currentStockSearch
                },
                this.onStocksSuccess,
                this.onStocksError
            );
            Logging.log_debug_msg(
                "StartPage_Model.startNextStocksRequest",
                "Started a new request!"
            );
        } else {
            Logging.log_debug_msg(
                "StartPage_Model.startNextStocksRequest",
                "Cannot schedule a reques at this time..."
            );
        }
    },
    
    onStocksSuccess: function (response) {
        "use strict";
        var self;
        self = Application.model;
        
        self.stocks = response.stocks;
        Application.presenter.stockListChanged();
    },
    
    onStocksError: function (code, reason) {
        "use strict";
        
        var self = null, msg = "Failed to get stocks because:\n" + reason;
        
        self = Application.model;
        
        Logging.warning("TraderServer_Model.onStocksError", msg);
    },

    /********************************************************************
     *                    Market Data
     ********************************************************************/
     lastPrice: 0,
     lastQty: 0,
     marketDataSubscription: null,

     updateMarketData: function () {
         "use strict";

         if (this.marketDataSubscription) {
             this.marketDataSubscription.stop();
             this.marketDataSubscription = null;
         }

         this.marketDataSubscription = this.startSubscription(
             "SubMarketData",
             { 
                 exchange: this.selectedVenue,
                 account: this.selectedAccount,
                 symbol: this.selectedStock
             },
             this.onMarketDataUpdate,
             this.onMarketDataError);
     },

     onMarketDataUpdate: function (update)  {
         "use strict";

         Application.presenter.updateMarketData(update);
     },

     onMarketDataError: function (code, reason) {
         "use strict";

         var msg = "Failed to get market data because:\n" + reason;

         Logging.warning("TraderServer_Model.onMarketDataError", msg);
     },

    /********************************************************************
     *                    Create Order
     ********************************************************************/
    reqNewOrder: function (side,type,price,qty) {
        "use strict";

        this.startRequest(
            "ReqNewOrder",
            {
                 exchange: this.selectedVenue,
                 account: this.selectedAccount,
                 stock: this.selectedStock,
                 type: type,
                 direction: side,
                 quantity: qty,
                 limit_price: price
            },
            this.onOrderSuccess,
            this.onOrderError
        );
    },

    onOrderSuccess(resonse) {
        "use strict";
    },

    onOrderError(code,reason) {
        "use strict";
        var error = "Failed to create order: " + reason;

        Logging.warning("TraderServer_Model.onOrderError", error);

        Application.view.sendErrorNotification(error);
    },

};
