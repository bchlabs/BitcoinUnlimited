#include "contracttally.h"
#include "contract/interactiveblockchain.h"
ContractTally * ContractTally::gs_single_tally_ = NULL;
ContractTally::ContractTally()
{
    contranct_catch_ = new InteractiveBlockChain();
}

ContractTally::~ContractTally()
{
    delete contranct_catch_;
}

void ContractTally::UpdateBlockChainToTally()
{
    contranct_catch_->UpdateTokenTxData();
}

bool ContractTally::GetTokenNameAmount(const std::string& token_txid, int vout, std::string &token_name, uint64_t &token_amount)
{

    if (!contranct_catch_->GetTokenNameAmount(token_txid,vout,token_name,token_amount))
        return false;

    return true;
}

bool ContractTally::GetTokenTxAddress(const std::string &token_txid, int vout, std::vector<std::string> &vect_token_address)
{
    if (!contranct_catch_->GetTokenAddress(token_txid,vout,vect_token_address))
        return false;
    return true;
}

ContractTally *ContractTally::Instance()
{
    //lock();
    if (gs_single_tally_ == NULL)
    {
        gs_single_tally_ = new ContractTally();
    }
    //unlock();
    return gs_single_tally_ ;
}
