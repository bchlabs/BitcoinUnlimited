#ifndef CONSTRACT_RPCTOKEN_H
#define CONSTRACT_RPCTOKEN_H

#include <univalue.h>

enum TokenTxType
{
     TOKEN_ISSUE = 0x0a,
     TOKEN_TRANSACTION = 0x0b

};
UniValue tokenissue(const UniValue& params, bool fHelp);
UniValue tokentransaction(const UniValue& params, bool fHelp);


#endif //CONSTACT_RPCTOKEN_H
