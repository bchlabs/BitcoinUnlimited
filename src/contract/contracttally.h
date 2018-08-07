#ifndef CONTRACTTALLY_H
#define CONTRACTTALLY_H
#include <vector>
#include <string>

struct ContractData
{
    uint64_t chain_postion;
    int block_postion;
    std::string txid;
    int vout;

};

struct TokenData : public ContractData
{
    std::string token_name;
    uint64_t token_amount;
    std::vector<std::string> vect_token_out_address;
};

class  InteractiveBlockChain;

class ContractTally
{
public:
    ContractTally();
    ~ContractTally();

    void  UpdateBlockChainToTally();

    bool GetTokenNameAmount(const std::string &token_txid, int vout , std::string &token_name, uint64_t&token_amount);

    bool GetTokenTxAddress(const std::string &token_txid, int vout ,std::vector<std::string>&vect_token_address);

    static ContractTally* Instance();
private:
    InteractiveBlockChain*  contranct_catch_;
    static ContractTally * gs_single_tally_;
};

#endif // CONTRACTTALLY_H
