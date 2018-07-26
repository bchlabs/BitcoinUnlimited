#ifndef CONTRACTTALLY_H
#define CONTRACTTALLY_H
#include <vector>
#include <string>
struct ContractData
{
    uint64_t chain_postion;
    int block_postion;
};

class ContractTally
{
public:
    ContractTally();
    ~ContractTally();

    static bool GetTokenName(const std::string token_txid,int vout ,std::string &token_name);

private:
    std::vector<ContractData> vect_contract_data_;
};

#endif // CONTRACTTALLY_H
