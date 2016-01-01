#include <tester.h>
#include <LevelConfig.h>
#include <ReqVenues.h>

int BasicRequest(testLogger& log);

int main(int argc, char** argv) {
    Test("Basic venues request...",BasicRequest).RunTest();
    return 0;
}

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
        },
        {
            "Name": "Alternative Account",
            "Code": "SDLKJFSDF"
        }
    ],

    "Venues" : [
            {
                "Name": "XXEU Beijing Exchange",
                "Code": "XXEUEX"
            },
            {
                "Name": "The test exchange.",
                "Code": "TESTEX"
            }
        ]
}
)";

namespace venues_fields {
    NewStringField(id);
    NewStringField(name);

    typedef SimpleParsedJSON<
        id,
        name
    > JSON;
}
NewObjectArray(venues, venues_fields::JSON);

typedef SimpleParsedJSON<
    venues
> VenuesReply;

bool ValidateAcccount(
          testLogger& log,
          venues_fields::JSON& venue,
          const std::string& name,
          const std::string& code)
{
    bool ok = true;

    if ( venue.Get<venues_fields::id>() != code ) {
        log << "Invalid venue id!" << endl;
        log.ReportStringDiff(venue.Get<venues_fields::id>(),code);
        ok = false;
    }

    if ( venue.Get<venues_fields::name>() != name ) {
        log << "Invalid venue name!" << endl;
        log.ReportStringDiff(venue.Get<venues_fields::name>(),name);
        ok = false;
    }

    if ( !ok) {
        log << "Failed to validate venue: " << name << endl;
    }

    return ok;
}
                     

int BasicRequest(testLogger& log) { 
    LevelConfig config(levelConfig);
    ReqVenues request(config);

    std::string response = request.OnRequest("");
    std::string error;


    VenuesReply json_response;
    if ( !json_response.Parse(response.c_str(),error)) {
        log << "Failed to parse response: :" << error << endl;
        return 1;
    }

    log << ">> " << response << endl;
    log << "<< " << json_response.GetPrettyJSONString() << endl;

    auto& accs = json_response.Get<venues>();

    if ( accs.size() != 2 ) {
        log << accs.size() << " venues returned!" << endl;
        return 1;
    }

    bool ok =  ValidateAcccount(log, *accs[1], "The test exchange.", "TESTEX");
    ok &= ValidateAcccount(log, *accs[0], "XXEU Beijing Exchange", "XXEUEX");

    if (!ok) {
        return 1;
    }

    return 0;
}
