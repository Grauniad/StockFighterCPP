#include <QuoteSubscription.h>
#include <LevelConfig.h>
#include <tester.h>

int NoClients(testLogger& log);
int SingleClientSingleUpdate(testLogger& log);
int MultipleClientsMultipleUpdates(testLogger& log);
int InvalidMessage(testLogger& log);
int URL(testLogger& log);

int  main(int argc, char**argv) {
    Test("Pushing updates with no client", NoClients).RunTest();
    Test("Single update to single client", SingleClientSingleUpdate).RunTest();
    Test("Sending updates too multiple clients", MultipleClientsMultipleUpdates).RunTest();
    Test("Ignore invalid messages", InvalidMessage).RunTest();
    Test("URL Generation", URL).RunTest();
    return 0;
}

std::vector<std::string> quote_messages = {
    R"(
    {
      "ok": true,
      "quote": { 
        "ask": 5125, 
        "askDepth": 2237, 
        "askSize": 711, 
        "bid": 5100, 
        "bidDepth": 2748, 
        "bidSize": 392, 
        "last": 5125, 
        "lastSize": 52, 
        "lastTrade": "2015-07-13T05:38:17.33640392Z", 
        "quoteTime": "2015-07-13T05:38:17.33640392Z",
        "symbol": "FAC",
        "venue": "OGEX"
      }
    }
    )",
    R"(
    {
      "ok": true,
      "quote": { 
        "symbol": "LUKEH",
        "venue": "OGEX",
        "bid": 5123, 
        "ask": 5124, 
        "bidSize": 245098, 
        "askSize": 234, 
        "bidDepth": 2748, 
        "askDepth": 2237, 
        "last": 234, 
        "lastSize": 2304, 
        "lastTrade": "2015-07-13T05:38:18.33640392Z", 
        "quoteTime": "2015-07-13T05:38:18.33640392Z" 
      }
    }
    )"
};

const std::string levelConfig = R"(
{
    "Client": {
        "port"   : 8081
    },
    "StockFighter": {
        "authKey"   : "7f60efa6b0e4a5894ea0845d9f3b287940a7eeed",
        "apiServer" : "test.api.stockfighter.io",
        "apiServerBaseUrl": "/testing/ob/api/"
    },

    "Accounts" : [
        {
            "Name": "Trading Account",
            "Code": "GB86011564"
        }
    ],

    "Venues" : [
            {
                "Name": " XXEU Beijing Exchange",
                "Code": "XXEUEX"
            }
        ]
}
)";

class TestQuoteSub: public QuoteHandler {
public:
    using QuoteHandler::HandleQuoteMessage;
    using QuoteHandler::MakeUrl;
};

std::vector<QuoteHandler::Quote> quotes = {
    {
        "FAC",
        {5100,392},
        {5125,711},
        {5125,52}
    },
    {
        "LUKEH",
        {5123,245098},
        {5124,234},
        {234,2304}
    },
};

bool QuotesEqual(testLogger& log,
                 const QuoteHandler::Quote& q1,
                 const QuoteHandler::Quote& q2)
{
    bool ok = true;

    if ( (q1.symbol != q2.symbol) ) {
        log << "Stock miss-match" << endl;
        log.ReportStringDiff(q1.symbol,q2.symbol);
        ok = false;
    }

    typedef QuoteHandler::Quote::Price Price;

    auto price_check = [&log] (const std::string& priceName,
                               const Price& p1,
                               const Price& p2) -> bool {
        bool ok = true;
        if (p1.price != p2.price)
        {
            ok = false;
            log << "Price miss-match  for " << priceName << endl; 
            log << "Expected: " << p1.price << endl;
            log << "Actual  : " << p2.price << endl;
        }

        if (p1.qty != p2.qty)
        {
            ok = false;
            log << "Quantity miss-match  for " << priceName << endl; 
            log << "Expected: " << p1.qty << endl;
            log << "Actual  : " << p2.qty << endl;
        }

        return ok;
    };

    ok &= (price_check("bestBid", q1.bestBid,q2.bestBid));
    ok &= (price_check("bestAsk", q1.bestAsk,q2.bestAsk));
    ok &= (price_check("last", q1.last,q2.last));

    return ok;
}

bool CheckClient(testLogger& log,
                 const std::string& name,
                 QuoteHandler::QuoteClient& client, 
                 std::vector<QuoteHandler::Quote> quotes) 
{
    bool ok = true;

    QuoteHandler::Quote q;

    size_t i = 0;
    for (QuoteHandler::Quote exp: quotes) {
        if ( !client->GetNextMessage(q) ) {
            log << endl << "Failed  to receive the quote messaage!" << endl;
            ok = false;
        } else if (!QuotesEqual(log,exp,q)) {
            log << endl << "Invalid quote received!" << endl;
            ok = false;
        } else {
            log << endl << "Messaage " << i << "matches!" << endl;
        }
        ++i;
    }

    if ( client->GetNextMessage(q) ) {
        log << endl << "Unexpected extra message received!" << endl;
        ok = false;
    }

    if ( !ok) {
        log << "Client " << name << " did not match expected quote set" << endl;
    }

    return ok;
}

int NoClients(testLogger& log) {
    TestQuoteSub quotedStock;

    quotedStock.HandleQuoteMessage(quote_messages[0]);

    QuoteHandler::Quote q;
    auto  client = quotedStock.NewClient();
    if ( client->GetNextMessage(q) ) {
        log << "Unexpected message in new quote client!" << endl;
        return 1;
    }
    return 0;
}

int SingleClientSingleUpdate(testLogger& log) {
    TestQuoteSub quotedStock;

    auto  client = quotedStock.NewClient();

    quotedStock.HandleQuoteMessage(quote_messages[0]);

    std::vector<QuoteHandler::Quote> expected = {
        quotes[0]
    };

    if (!CheckClient(log,"",client,expected)) {
        return 1;
    }

    return 0;
}

int MultipleClientsMultipleUpdates(testLogger& log) {
    TestQuoteSub quotedStock;

    auto  client = quotedStock.NewClient();

    quotedStock.HandleQuoteMessage(quote_messages[0]);

    auto  client2 = quotedStock.NewClient();

    quotedStock.HandleQuoteMessage(quote_messages[1]);

    std::vector<QuoteHandler::Quote> expected1 = {
        quotes[0],
        quotes[1]
    };

    std::vector<QuoteHandler::Quote> expected2 = {
        quotes[1]
    };

    if (!CheckClient(log,"client 1",client,expected1)) {
        return 1;
    }

    if (!CheckClient(log,"client 2",client2,expected2)) {
        return 1;
    }
    return 0;
}

int InvalidMessage(testLogger& log) {
    TestQuoteSub quotedStock;

    auto  client = quotedStock.NewClient();

    quotedStock.HandleQuoteMessage(R"({"ok": false})");
    quotedStock.HandleQuoteMessage(R"({"unknown field": "ERROR!" })");
    quotedStock.HandleQuoteMessage(quote_messages[0]);

    std::vector<QuoteHandler::Quote> expected = {
        quotes[0]
    };

    if (!CheckClient(log,"",client,expected)) {
        return 1;
    }

    return 0;
}

int URL(testLogger& log) {
    LevelConfig cfg(levelConfig);

    std::string url = TestQuoteSub::MakeUrl("TESTEX","TESTACC",cfg.ApplicationConfig());

    std::string eurl = 
       "wss://test.api.stockfighter.io/testing/ob/api/ws/TESTACC/venues/TESTEX/tickertape";

    if ( url != eurl) {
        log.ReportStringDiff(eurl,url);
        return 1;
    }

    return 0;
}
