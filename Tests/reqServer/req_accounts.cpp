#include <tester.h>
#include <LevelConfig.h>
#include <ReqAccounts.h>

int BasicRequest(testLogger& log);

int main(int argc, char** argv) {
    Test("Basic accounts request...",BasicRequest).RunTest();
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
        },
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

namespace accounts_fields {
    NewStringField(id);
    NewStringField(name);

    typedef SimpleParsedJSON<
        id,
        name
    > JSON;
}
NewObjectArray(accounts, accounts_fields::JSON);

typedef SimpleParsedJSON<
    accounts
> AccountsReply;

bool ValidateAcccount(
          testLogger& log,
          accounts_fields::JSON& account,
          const std::string& name,
          const std::string& code)
{
    bool ok = true;

    if ( account.Get<accounts_fields::id>() != code ) {
        log << "Invalid account id!" << endl;
        log.ReportStringDiff(account.Get<accounts_fields::id>(),code);
        ok = false;
    }

    if ( account.Get<accounts_fields::name>() != name ) {
        log << "Invalid account name!" << endl;
        log.ReportStringDiff(account.Get<accounts_fields::name>(),name);
        ok = false;
    }

    if ( !ok) {
        log << "Failed to validate account: " << name << endl;
    }

    return ok;
}
                     

int BasicRequest(testLogger& log) { 
    LevelConfig config(levelConfig);
    ReqAccounts request(config);

    std::string response = request.OnRequest("");
    std::string error;

    AccountsReply json_response;
    if ( !json_response.Parse(response.c_str(),error)) {
        log << "Failed to parse response: :" << error << endl;
        return 1;
    }

    auto& accs = json_response.Get<accounts>();

    if ( accs.size() != 2 ) {
        log << accs.size() << " accounts returned!" << endl;
        return 1;
    }

    bool ok =  ValidateAcccount(log, *accs[0], "Trading Account", "GB86011564");
    ok &= ValidateAcccount(log, *accs[1], "Alternative Account", "SDLKJFSDF");

    if (!ok) {
        return 1;
    }

    return 0;
}
