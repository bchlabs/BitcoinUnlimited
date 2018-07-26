#ifndef CONTRACTTALLY_H
#define CONTRACTTALLY_H
#include <vector>
#include <string>

struct ContractData
{
    uint64_t chain_postion;
    int block_postion;
    int vout;

};

struct TokenData : public ContractData
{
    std::string token_name;
    uint64_t token_amount;
};

class  InteractiveBlockChain;

class ContractTally
{
public:
    ContractTally();
    ~ContractTally();

    void  AddTokenHead(const std::string&txid,int vout);

    static bool GetTokenNameAmount(const std::string token_txid,int vout ,std::string &token_name,uint64_t&token_amount);

private:
    InteractiveBlockChain*  contranct_catch_;
};

#endif // CONTRACTTALLY_H
