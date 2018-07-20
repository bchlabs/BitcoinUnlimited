#ifndef CONTRACT_TOKENTXCHECK_H
#define CONTRACT_TOKENTXCHECK_H
#include <univalue.h>


enum TokenTxErrorCode
{
    TOKENOK =0,
    ERRORTOKENVIN =1
};

int CheckTokenVin(const UniValue& params);
bool IsTxidUnspent(const std::string txid, const int vout);

#endif
