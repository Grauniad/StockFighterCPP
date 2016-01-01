#include <json_tests.h>

bool JSONCheckers::CheckRequestRejected(
              testLogger&             log,
              const std::string&      request,
              RequestReplyHandler&    handler,
              const std::string&      reason,
              int                     reason_code)
{
    bool ok = true;
    bool errored = false;
    std::string error = "";
    int code = 0;

    try {
        handler.OnRequest(request.c_str());
    } catch (RequestReplyHandler::InvalidRequestException& e) {
        errored = true;
        code = e.code;
        error = e.errMsg;
    }

    if ( !errored) {
        log << "No error thrown!" << endl;
        ok = false;
    }

    if ( code != reason_code) {
        ok = false;
        log << "Missmatch in error code!" << endl;
        log << "Expected: " << reason_code << endl;
        log << "Actual  : " << code << endl;
    }

    if ( reason != error ) {
        log << "Missmatch in erorr text" << endl;
        log.ReportStringDiff(reason,error);
        ok = false;
    }

    if (!ok) {
        log << endl << endl 
            << "Request Reply handler did not throw the expected error:" << endl
            << reason << endl
            << "when parsing:" << endl
            << request << endl;
    }

    return ok;
}

bool JSONCheckers::NoMoreMessages(testLogger& log,
                    std::shared_ptr<SubRequestMock>& request)
{
    bool ok = true;
    std::string message = "";
    if (request->GetNextMessage(message)) {
        ok = false;
        log << "Unexpected message received: " << endl;
        log << message << endl;
        log << "On request: " << endl;
        log << request->RequestMessasge() << endl;
    }

    return ok;
}
