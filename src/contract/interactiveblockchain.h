#ifndef INTERACTIVEBLOCKCHAIN_H
#define INTERACTIVEBLOCKCHAIN_H
#include <string>
#include <vector>

struct ContractData;
struct TokenData;


class InteractiveBlockChain
{
public:
    InteractiveBlockChain();

    void Set_begin_pos(uint64_t beginPos);

    void  InitTokenTxData();

    void  UpdateTokenTxData();

    void  RetsetTokenTxData();

public:
    bool GetTokenNameAmount(const std::string & txid,int vout,std::string &token_name,uint64_t&amount);

    bool GetTokenAddress(const std::string & txid, int vout, std::vector<std::string> &vect_token_address);

protected:
    void AddTokenTxData(uint64_t begin,uint64_t end);

private:
    uint64_t begin_pos_;

    uint64_t end_pos_;

    std::vector<TokenData*> vect_mint_token_data_;
    std::vector<TokenData*> vect_transfer_token_data_;
};

#endif // INTERACTIVEBLOCKCHAIN_H
