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

bool WitnessTxidValid(const std::string &witness_txid,const uint32_t witness_vout);

bool TokenInputValid(const std::string &token_input_txid,const uint32_t token_input_vout);

#endif
