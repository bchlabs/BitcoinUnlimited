#include "contracttally.h"
#include "contract/interactiveblockchain.h"

ContractTally::ContractTally()
{
    contranct_catch_ = new InteractiveBlockChain();
}

ContractTally::~ContractTally()
{
    delete contranct_catch_;
}

bool ContractTally::GetTokenNameAmount(const std::__cxx11::string token_txid, int vout, std::__cxx11::string &token_name, uint64_t &token_amount)
{

    bool ret = true;
    return ret;
}
