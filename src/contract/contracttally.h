#ifndef CONTRACTTALLY_H
#define CONTRACTTALLY_H
#include <vector>
struct ContractData
{
    long long chain_postion;
    int block_postion;
};

class ContractTally
{
public:
    ContractTally();
    ~ContractTally();

private:
    std::vector<ContractData> vect_contract_data_;
};

#endif // CONTRACTTALLY_H
