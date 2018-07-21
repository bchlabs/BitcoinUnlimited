#ifndef CONTRACT_TOKENTXCHECK_H
#define CONTRACT_TOKENTXCHECK_H

#include <univalue.h>
#include "script/script.h"


enum TokenTxErrorCode
{
    TOKENOK =0,
    ERRORTOKENVIN =1
};

int CheckTokenVin(const UniValue& params);
bool IsTxidUnspent(const std::string &txid, const uint32_t vout);
CScript GetTokenScript(const std::string txid);

#endif
