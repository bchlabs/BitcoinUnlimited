#ifndef CONTRACT_TOKENTXCHECK_H
#define CONTRACT_TOKENTXCHECK_H
#include <univalue.h>

class CScript;

int CheckTokenVin(const UniValue& params);

bool IsTxidUnspent(const std::string &txid, const uint32_t vout);

CScript GetTokenScript(const std::string txid);

bool WitnessTxidValid(const std::string &witness_txid,const uint32_t witness_vout);

bool TokenInputValid(const std::string &token_input_txid,const uint32_t token_input_vout);

std::string signTokenTxid(const std::string &strAddress,const std::string &strMessage);

bool verifytokentxid(const std::string &strAddress,const std::string &strSign,const std::string &strMessage);




#endif
