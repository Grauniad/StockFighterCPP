/*global $, console */

/**
 * Web Socket Request-Reply module.
 * --------------------------------
 *
 *  (c) Luke Humphreys 2015
 *
 *  This module provides a simple wrapper around 
 */
function wsrequest_NewRequestManager(url) {
    "use strict";
    return function () {
        "use strict";
        var reqNum = 0, activeRequests = [];

        function deleteRequest(id) {
            "use strict";
            activeRequests.splice(id,1);
        }

        function getNextId() { 
            "use strict";
            reqNum += 1;
            return reqNum;
        }

        return {
            newRequest: function (reqName, msg, context) { 
                "use strict";
                var reqid = getNextId();
                context._onDone = function () { deleteRequest(reqid); }
                context._server = url;
                context._one_shot = true;
                activeRequests[reqid] = new WSRequest(reqName, msg, context);
                return reqid;
            },

            newSubscription: function (reqName, msg, context) { 
                "use strict";
                var reqid = getNextId(), request = null;
                context._onDone = function () { deleteRequest(reqid); }
                context._server = url;
                context._one_shot = false;
                request = new WSRequest(reqName, msg, context);
                activeRequests[reqid] = request;
                return {
                    requestId: reqid,

                    stop: function () {
                        "use strict";
                        request.close();
                    }
                };
            },
        };
    }();
}

function WSRequest(reqName, msg, context) {
    "use strict";
    this._request = reqName + " " + JSON.stringify(msg);
    if (context) {
        extend(this, context);
    }
    this.startRequest();
}

WSRequest.prototype = {
    onResponse: null,
    onError: null,
    // Used by WSRequest_NewRequestManager
    _onDone: null,
    _server: "",
    _one_shot: true,
    // Internal data
    _socket: null,
    _request: null,
    _closing: false,

    startRequest: function () {
        "use strict";
        this._socket = new WebSocket(this._server);
        this._socket.onopen = $.proxy(this._onOpen, this);
        this._socket.onmessage = $.proxy(this._onMessage, this);
        this._socket.onclose = $.proxy(this._onClose, this);
        this._socket.onerror = $.proxy(this._onError, this);
    },

    close: function() {
        "use strict";
        this._socket.close();
        this._socket = null;
        this._closing = true;
        if (Logging.debugLogging) {
            Logging.log_debug_msg("WSRequest.close", "closing connection: ");
        }
    },

    _onOpen: function (evt) {
        "use strict";
        this._socket.send(this._request);
    },

    _onClose: function (evt) {
        "use strict";

        if(this.closing === false) {
            this._reportError(0,"Server connection lost!");
        }

        if(this._onDone) { 
            this._onDone();
        }
    },

    _onError: function (evt) {
        "use strict";
        this._reportError(-1,evt.data);

        this.close();
    },

    _onMessage: function(evt) {
        "use strict";
        var msg = evt.data, err = "";


        if (msg.substring(0,6) === "ERROR ") {
            err = JSON.parse(msg.substring(6));
            this._reportError(err.errorNumber, err.errorText);
        } else {
            if ( this.onResponse ) {
                this.onResponse(JSON.parse(msg));
            }
        }

        if (this._one_shot === true) {
            this.close();
        }
    },

    _reportError: function(code, err) {
        if (this.onError) {
            this.onError(code,err);
        }
    }
};
