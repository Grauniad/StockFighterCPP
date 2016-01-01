#ifndef __STOCK_FIGHTER_LIBS_TESTING_JSOSN_CHECKERS_H__
#define __STOCK_FIGHTER_LIBS_TESTING_JSOSN_CHECKERS_H__
#include <tuple>
#include <call_from_tupple.hpp>
#include <SubscriptionRequestMock.h>
#include <vector>
#include <string>
#include <ReqServer.h>
#include <tester.h>


namespace JSONCheckers {

    /****************************************************************
     *                     Internal Utilities
     **************************************************************/

    template <class ...Fields>
    bool AllTrue(Fields... args) {
        std::vector<bool> args_array = {args...};
        return *min_element(args_array.begin(),args_array.end());
    }

    /**
     * Validate that the specified field is supplied on the on a JSON and it
     * has the correct value.
     */
    template <class JSON, class Field>
    bool FieldMatch(testLogger& log, JSON& expected, JSON& actual, Field& field) {
        bool ok = true;

        if (!actual.template Supplied<Field>()) {
            log << "Field " << field.Name() << " was not supplied!" << endl;
            ok = false;
        }

        if (expected.template Get<Field>() != actual.template Get<Field>()) {
            log << "Field " << field.Name() << " has invalid value" << endl;
            log << "Expected: " << expected.template Get<Field>()  << endl;
            log << "Actual  : " << actual.template Get<Field>()  << endl;
            ok = false;
        }

        return ok;
    }

    template <class JSON, class ...Fields, int... Idxs>
    bool JSONFieldsMatch_Target(testLogger& log,
                                JSON& expected,
                                JSON& actual,
                                std::tuple<Fields...>& args,
                                CallFromTuple::Sequence<Idxs...> seq) {
        return AllTrue(FieldMatch(log,expected,actual,std::get<Idxs>(args))...);
    }

    /****************************************************************
     *                     External Functions  
     **************************************************************/

    /**
     * Shameless hacky abuse of variadiac template to save some lines of test code
     * at the cost of much computation. (We care not since we only use this in 
     * the unit tests)
     */
    template <class ...Fields, class JSON>
    bool JSONFieldsMatch(testLogger& log, JSON& expected, JSON& actual) {
        // Default construct an instance of each class
        std::tuple<Fields...> hargs;

        typename CallFromTuple::PackBuilder<sizeof...(Fields)>::ThePack idxs;
        return JSONFieldsMatch_Target(log,expected,actual,hargs,idxs);
    }

    template <class ...Fields, class ResponseParser>
    bool CheckSubscriptionMessage(testLogger&          log,
                     std::shared_ptr<SubRequestMock>&  request,
                     ResponseParser&                   parser,
                     ResponseParser&                   expectedResponse)
    {
        bool ok = true;

        parser.Clear();
        std::string error = "";
        std::string response = "";

        if (!request->GetNextMessage(response)) {
            ok = false;
            log << "No message received!" << endl;
        } else {
            log << ">> " << response << endl;
        }

        if (ok) {
            if ( !parser.Parse(response.c_str(),error)) {
                log << "Failed to parse response: " << error << endl;
                ok = false;
            } else {
                log << "Parsed JSON response: " << endl;
                log << "<< " << parser.GetPrettyJSONString() << endl;
            }
        }

        if (ok) {
            ok &= JSONFieldsMatch<Fields...>(log, expectedResponse, parser);
        }

        if (!ok) {
            log << endl << endl 
                << "Subscription handler did not return the expected result:" << endl
                << expectedResponse.GetPrettyJSONString() << endl
                << "for request: " << endl
                << request->RequestMessasge() << endl;
        }

        return ok;
    }

    template <class ...Fields, class ResponseParser>
    bool SubscriptionIsAccepted(testLogger&            log,
                     SubscriptionHandler&              handler,
                     std::shared_ptr<SubRequestMock>&  request,
                     ResponseParser&                   parser,
                     ResponseParser&                   expectedResponse)
    {
        bool ok = true;

        parser.Clear();
        request->ClearMessages();

        try {
            handler.OnRequest(request);
        } catch (SubscriptionHandler::InvalidRequestException& e) {
            ok = false;
            log << "Subscription handler raised an unexpeccted error: " << endl;
            log << "Error: " << e.errMsg << endl;
            log << "Code: " << e.code << endl;
        }

        if (ok) {
            ok &= CheckSubscriptionMessage<Fields...>(log,request,parser,expectedResponse);
        }

        if (!ok) {
            log << endl << endl 
                << "Subscription handler did not handle the initial response correctly when parsing:" << endl
                << request->RequestMessasge() << endl;
        }

        return ok;
    }

    bool NoMoreMessages(testLogger& log,
                        std::shared_ptr<SubRequestMock>& request);

    template <class ...Fields, class ResponseParser>
    bool CheckRequest(testLogger&                       log,
                      const std::string&                request,
                      RequestReplyHandler&              handler,
                      ResponseParser&                   parser,
                      ResponseParser&                   expectedResponse)
    {
        bool ok = true;

        parser.Clear();

        std::string result = "";
        try {
            result = handler.OnRequest(request.c_str());
        } catch (RequestReplyHandler::InvalidRequestException& e) {
            ok = false;
            log << "Request Reply handler raised an unexpeccted error: " << endl;
            log << "Error: " << e.errMsg << endl;
            log << "Code: " << e.code << endl;
        }

        if (ok) {
            std::string error = "";
            if ( !parser.Parse(result.c_str(),error)) {
                ok = false;
                log << "Failed to parse response: " << error << endl;
            }
        }

        if ( ok ) {
            ok &= JSONFieldsMatch<Fields...>(log, expectedResponse, parser);
        }

        if (!ok) {
            log << endl << endl 
                << "Request Reply handler did not handle the initial response correctly when parsing:" << endl
                << request << endl;
        }

        return ok;
    }

    bool CheckRequestRejected(
                      testLogger&             log,
                      const std::string&      request,
                      RequestReplyHandler&    handler,
                      const std::string&      reason,
                      int                     reason_code);

}

#endif
