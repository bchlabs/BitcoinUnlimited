#ifndef CONTRACT_TOKENTXCHECK_H
#define CONTRACT_TOKENTXCHECK_H
#include <univalue.h>

class CScript;

int CheckTokenVin(const UniValue& params);

bool IsTxidUnspent(const std::string &txid, const uint32_t vout);


bool WitnessValid(const std::vector<std::string>&witness_data,const std::vector<std::string>&witness_address,const std::string&org_message);

bool TokenInputValid(const std::string &token_input_txid,const uint32_t token_input_vout);

std::string signTokenTxid(const std::string &strAddress,const std::string &strMessage);

bool verifyWitness(const std::string &strSign, const std::string &strAddress, const std::string &strMessage);




#endif
