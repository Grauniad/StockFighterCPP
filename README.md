# StockFighterCPP
A C++ framework for implementing algos on the StockFigher.io game.

##Implementation
<a name="implementation"></a>
- [Library Summary](#libraries)
- [Key Objects](#objects)
- [Threads and event loop integration](#threads)
- [Publishers and Clients](#Publishers)

The server runs as an extendible single process, multi-threaded
application, which can handle JSON requests from a client.

###Component Libraries
<a name="libraries"></a>
[Back to Implementation.](#implementation)
- **libLevelConfig:** 
   Represents the level specific configuration as well stock figher connection details.
- **libStockFighter:**
      Our wrapper around the StockFighter JSON API
- **libExchangeInterface:**
      Exchange STATIC data. (Available exchanges,  available stocks)
- **libMarketData:**
      Publishes market data objects for any given stock.
- **libOrder:**
      The core library - models orders on the exchange, publishes
      updates and allows actions to be performed on them.
- **libRequestServer:**
      Expose a custom JSON interface to build our own web client.
- **libTestUtils:**
      Unit testing tools specific to the project.

###Some key data objects:
<a name="objects"></a>
[Back to Implementation.](#implementation)

####LevelConfig
**Code:** Libraries/libLevelConfig/LevelConfig.h

The configuration object is responsible for reading in the
configuration file that defines the current level. The
configuration defines how the server should talk to StockFighter:

- API location
- Account Key (user authorization - DONT COMMIT THIS TO GITHUB)
- Account name (Specified per level)
- Exchange Config (Again specified per level)

The server retrieves any information it can (such as available
stocks) through the API, rather than configuration.

**See also:** level1.json

####Exchanges / Exchange:

**Code:** Libraries/libExchangeInterface/Exchanges.h

**Code:** Libraries/libExchangeInterface/ExchangeInterface.h

This represents the static data for an exchange:

- The set of exchanges available to trade on, and any static data associated with them (e.g name, id).
- The set of stocks available to trade on the exchange, and any static data associated with the stock.

The Exchanges object is fully initialised using the API before the
"dynamic" components of the server are started, guaranteeing the
static data is always available.

####StockFighterConnection
**Code:** Libraries/libStockFighter/StockFighterConnection.h

Access point to the StockFighter API.

Currently the following parts of the API are implemented:

- Get the list of stocks on an exchange. (GET request)
- Subscribe to quote updates on an exchange.
   (PipeSubscriber). This is just a C++ wrapper around the
   API, most applications will want to use the
   MarketDataProvider classes to access the data.
- Post an order (POST request)

####MarketDataProvider
**Code:** Libraries/libMarketData/MarketDataProvider.h

Publishes market data updates for any stock on any exchange an
account has access to. Clients subscribe to updates on a given
stock.

If multiple accounts are enabled the global set of
MarketDataProviders is managed by the MarketDataManager.

####ExchangeOrder (libOrder):
**Code:** Libraries/libOrder/ExchangeOrder.h

Represents an order on the exchange, and publishes updates to
the order.

**NOTE:** Clients should not use this raw interface for updating an
      order, instead the *OrderActionHandler* should be used.

####OrderManager:
**Code:** Libraries/libOrder/OrderManager.h

Manages all orders known to the server. Existing orders can be
looked up via their order id.

####OrderActionHandler:
**Code:** Libraries/libOrder/OrderActionHandler.h

Interface for updating orders. Validators may be dynamically
installed to restrict actions which can be performed.

###<a name="Publishers"></a>Publishers/ Subscribers:
[Back to Implementation.](#implementation)
**Code:** DEV_TOOLS/CPP/Libraries/ThreadComms/PipePublisher.h

**Code:** DEV_TOOLS/CPP/Libraries/ThreadComms/PipeSubscriber.h

Where appropriate classes "publish" updates, to any other object who has dynamically registered their interest.
This allows classes to be built in isolation and then clicked togethor in an application.


There are two methods a client can use to receive updates:

1. **Queued**

  This is the default mode. Each updates is copied into a thread safe,
  lockless queue, which can be polled later by the client to pick up
  updates.

  Optionally the client may use the OnNextMessage method to register a
  task to be performed when data is ready on the queue. The task will
  only be fired once, and acts as a "unread data" notification. This
  is typically used to post a task to an event_loop to poll the
  queue. Execution of the callback occurs under lock, but pushes to
  the queue remain lockless.
  
2. **Notified**

  In this mode, enabled by registering a callback via OnNewMessage, a
  task is triggered for every published message. This has the
  advantage of not having to copy construct the data.

  NOTE: The callback occurs on the publisher's thread so you may need
        to post it to the correct event loop.

  This notification is fired under lock, during which changes to the
  publisher may not be made.

###<a name="threads"></a>Threads and Event Loops
[Back to Implementation.](#implementation)

All application logic should occur on the main "REQUEST_THREAD",
which is responsible for maintaining all application data. There are
however several work threads to be aware of.

- **REQUEST_THREAD**

  This is the main event_loop. All access, or modification, to system
  data must be made on this thread, and it is responsible for handling
  requests from the client.
  
  Tasks can be posted to this thread via TraderServer::PostTask.

- **IO_THREAD**

  **Code:** DEV_TOOLS/CPP/Libraries/libWebSockets/http_request.h

  POST / GET requests are handed off to the IO_THREAD by the API
  implementation. In addition to running the boost::asio event
  loop for these requests this thread can be used to do any JSON
  parsing before posting data to the main REQUEST_THREAD.
  
  **NOTE:** It is not possible to post tasks to the IO_THREAD;
  
  Tasks on the IO_THREAD are created by the API implementation, and
  managed via an HTTPRequest handle. There are two ways to get the
  result:

  1. Execute HTTPRequest::WaitForMessage() on any thread. 
  
    This will block the current thread until a response has been received, and then
    return it.
     
    **NOTE:** This is not recommended on the REQUEST_THREAD.
   
  2. Setup a callback function using the HTTPRequest::OnMessage handler. 
  
    When a response is received the task will be fired
     immediately on the IO_THREAD. This can be used to do any
     initial parsing and then either publish the data or post an
     event to the REQUEST_THREAD.
    
     **NOTE:** Don't forget to modify any data you will need to post a task back to the REQUEST_THREAD

- **QUOTE_SUB** (one per account/exchange pair):

    Dedicated to receiving quote updates through the StockFighter
    websockets API. Parsing of the JSON into the MarketData struct is done
    here, which is then published.
    
    **NOTE:** A client should not have to know about the existence of these
          threads. The MarkatDataProvider manages the consumption of this
          data and re-publishes it (per stock) on the request thread.

- **TRADE_SUB** (one per account/exchange pair):

       ** NOT YET IMPLEMENTED.**
    Much like the quote-sub thread except that it handles trades
    rather than quotes.
    
    **NOTE:** Again clients should use the data as it is republished in
         the request thread by the ExchangeOrder.
