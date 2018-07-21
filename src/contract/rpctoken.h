#ifndef CONSTRACT_RPCTOKEN_H
#define CONSTRACT_RPCTOKEN_H

#include <univalue.h>

enum TokenTxType
{
     TOKEN_ISSUE = 0x0a,
     TOKEN_TRANSACTION = 0x0b

};

struct TokenStruct
{
	std::string txid;
	std::string vout;
	std::string name;
	CAmount amount;
	std;:string address;
	
};

UniValue tokenmint(const UniValue& params, bool fHelp);
UniValue tokentransfer(const UniValue& params, bool fHelp);
UniValue listtokeninfo(const UniValue& params, bool fHelp);


#endif //CONSTACT_RPCTOKEN_H
