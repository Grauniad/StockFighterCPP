#include "QuoteSubscription.h"
#include <iostream>
#include <logger.h>
using namespace std;

namespace ParsedQuote {
    NewBoolField(ok);

    NewI64Field(ask);
    NewI64Field(askDepth);
    NewI64Field(askSize);
    NewI64Field(bid);
    NewI64Field(bidDepth);
    NewI64Field(bidSize);
    NewI64Field(last);
    NewI64Field(lastSize);
    NewStringField(lastTrade);
    NewStringField(quoteTime);
    NewStringField(symbol);
    NewStringField(venue);

    typedef SimpleParsedJSON<
        ask,
        askDepth,
        askSize,
        bid,
        bidDepth,
        bidSize,
        last,
        lastSize,
        lastTrade,
        quoteTime,
        symbol,
        venue
    > QuoteJSON;
    NewEmbededObject(quote, QuoteJSON);

    typedef SimpleParsedJSON< ok, quote > Message;

};

QuoteSubscription::QuoteSubscription(
        const std::string& exch,
        const std::string& acc,
        const StockFighterConfig& cfg)
    : StreamClientThread(MakeUrl(exch,acc,cfg))
{
}

QuoteSubscription::QuoteClient QuoteHandler::NewClient() {
    QuoteClient client(publisher.NewClient(10240));

    return client;
}

std::string QuoteHandler::MakeUrl(
        const std::string& exch,
        const std::string& acc,
        const StockFighterConfig& cfg)
{
    std::string url = "wss://";
    url.reserve(128);
    url += cfg.APIServer();
    url += cfg.APIBaseUrl() + "ws/";
    url += acc;
    url += "/venues/";
    url += exch;
    url += "/tickertape";
    return url;
}

void QuoteSubscription::OnMessage(const std::string& json) {
    HandleQuoteMessage(json);
}

void QuoteHandler::HandleQuoteMessage(const std::string& json) {
    thread_local ParsedQuote::Message msg;
    msg.Clear();
    std::string error;

    if (msg.Parse(json.c_str(),error)) { 
        if ( msg.Get<ParsedQuote::ok>() ) {
            ParsedQuote::QuoteJSON& quote = msg.Get<ParsedQuote::quote>();
            Quote  q = {
                quote.Get<ParsedQuote::symbol>(),
                {quote.Get<ParsedQuote::bid>(), quote.Get<ParsedQuote::bidSize>()},
                {quote.Get<ParsedQuote::ask>(), quote.Get<ParsedQuote::askSize>()},
                {quote.Get<ParsedQuote::last>(), quote.Get<ParsedQuote::lastSize>()}
            };
            SLOG_FROM(LOG_VERY_VERBOSE,"QuoteSubscription::OnMessage",
               "Publishing Quote:" << endl << 
               "    Symbol: " << q.symbol << endl <<
               "    Best Bid: {" << q.bestBid.price << ", " << q.bestBid.qty << "}" << endl << 
               "    Best Ask: {" << q.bestAsk.price << ", " << q.bestAsk.qty << "}" << endl <<
               "    Last    : {" << q.last.price << ", " << q.last.qty << "}" << endl
            );
            publisher.Publish(q);
        } else {
            SLOG_FROM(LOG_VERY_VERBOSE,"QuoteSubscription::OnMessage",
               "Not Publishing Bad Quote:" << endl << 
               msg.GetPrettyJSONString() << endl
            );
        }
    } else {
        SLOG_FROM(LOG_WARNING,"QuoteSubscription::OnMessage",
           "Invalid message received: " << endl << 
           json << endl <<
           "Error was: " << error);
    }
}
