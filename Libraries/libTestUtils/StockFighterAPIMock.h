#ifndef __STOCK_FIGHTER_LIBS_TESTING_API_MOCK_BASE_H__
#define __STOCK_FIGHTER_LIBS_TESTING_API_MOCK_BASE_H__

#include <StockFighterConnection.h>
#include <http_request.h>
#include <string>

class StockFighterAPIMockBase: public StockFighterAPI {
public:
    struct raw_stock {
        std::string name;
        std::string id;
    };
    
    StockFighterAPIMockBase(IPostable* event_loop = nullptr);

    class TestHTTPRequest: public HTTPMessageProvider {
    public:
        TestHTTPRequest(IPostable* e = nullptr) : event_loop(e) { }
        virtual const HTTPMessage& WaitForMessage() {
            return message;
        }

        struct InvalidCallToPostResponse {};
        virtual void OnMessage(const Task& notification)
        {
            if ( event_loop != nullptr) {
                event_loop->PostTask(notification);
            } else {
                throw InvalidCallToPostResponse{};
            }
        }

        HTTPMessage message;
    private:
        IPostable* event_loop;
    };

    /**
     * To be implemented by the test class if this function is to be used.
     */
    struct InvalidCallToGetStocks { };
    virtual StockFighterReqHdl GetStocks(const std::string& exchange) {
        throw InvalidCallToGetStocks{};
    }

    /**
     * Implemented by default, test class may override with exception version
     * if this class is not to be used.
     */
    struct InvalidCallToNewExchangeQuoteClient { };
    QuoteSubscription::QuoteClient NewExchangeQuoteClient(
                                        const std::string& exch,
                                        const std::string& acc);

    /**
     * Utility method to publish a quote for a given stock.
     */
    virtual void PublishQuote(const std::string& exch, const QuoteHandler::Quote& quote);

    /**
     * Implemented by default, test class may override with exception version
     * if this class is not to be used.
     */
    struct InvalidCallToPostOrder_External { };
    virtual OrderPostReqHdl PostOrder(const OrderDefinition& order);

    /**
     * By default this will return details matching the provided order, but the
     * response can be explicitly overriden with SetNextOrderResponse.
     */
    struct InvalidCallToPostOrder_Internal { };
    virtual StockFighterReqHdl PostOrder(const std::string& venue,
                                         const std::string& symbol,
                                         const std::string& order);

    /**
     * Set the response to be returned by the next call to PostOrder (internal)
     */
    virtual void SetNextOrderResponse(const std::string& response);

    /**
     * Get the last arguments passed to PostOrder (internal)
     */
    struct PostOrderCall { 
         std::string venue; std::string symbol; std::string order;
    };
    const PostOrderCall& LastCallTo_PostOrder() const {
        return lastPostOrderCall;
    }

    /**
     * Return the last allocated ID
     */
     const size_t& LastAllocatedId() const {
         return nextId;
     }
protected:

    /**
     * Utility method to convert a list of raw stocks into 
     * the expected JSON format.
     */
    std::string StocksJSON(std::vector<raw_stock> stocks);

    typedef PipePublisher<QuoteHandler::Quote> Publisher;

    /**
     * Get the Mock publisher for the specified exchange.
     */
    Publisher& GetPublisher(const std::string& exch);

private:
    std::map<std::string,Publisher> exchanges;
    std::string  orderPostResponse;
    size_t       nextId;
    IPostable*   event_loop;

    PostOrderCall lastPostOrderCall;
};

#endif
