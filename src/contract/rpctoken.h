#ifndef CONSTRACT_RPCTOKEN_H
#define CONSTRACT_RPCTOKEN_H
#include <univalue.h>

struct TokenStruct
{
	std::string txid;
    int vout;
	std::string name;
    uint64_t amount;
    std::string address;
    bool sign_ok;
};

UniValue tokenmint(const UniValue& params, bool fHelp);

UniValue tokentransfer(const UniValue& params, bool fHelp);

UniValue listtokeninfo(const UniValue& params, bool fHelp);

UniValue minttoken(const UniValue& params, bool fHelp);

UniValue transfertoken(const UniValue& params, bool fHelp);


#endif //CONSTACT_RPCTOKEN_H
