#include "interactiveblockchain.h"
#include "contract/contracttally.h"
#include "contract/contractconfig.h"
#include "contract/contractinterpeter.h"
#include "chain.h"
#include "main.h"

InteractiveBlockChain::InteractiveBlockChain()
{
    begin_pos_ = 0;
    end_pos_ = 0;
}

void InteractiveBlockChain::Set_begin_pos(uint64_t beginPos)
{
    begin_pos_ = beginPos;
}

void InteractiveBlockChain::InitTokenTxData()
{
    end_pos_ = chainActive.Height();
    AddTokenTxData(begin_pos_,end_pos_);
}

void InteractiveBlockChain::UpdateTokenTxData()
{

    uint64_t height = chainActive.Height();
    AddTokenTxData(end_pos_,height);
    end_pos_ = height;

}

void InteractiveBlockChain::RetsetTokenTxData()
{

    for (unsigned int i = 0; i < vect_mint_token_data_.size(); ++i)
    {
        delete vect_mint_token_data_.at(i);
    }

    for (unsigned int i = 0; i < vect_transfer_token_data_.size(); ++i)
    {
        delete vect_transfer_token_data_.at(i);
    }
    vect_mint_token_data_.clear();
    vect_transfer_token_data_.clear();
}

bool InteractiveBlockChain::GetTokenNameAmount(const std::string &txid, int vout, std::string &token_name, uint64_t &amount)
{
    unsigned int i = 0;
    TokenData* token_tx_mint = NULL;
    TokenData* token_tx_transfer = NULL;
    for (; i < vect_mint_token_data_.size(); ++i)
    {
        token_tx_mint = vect_mint_token_data_.at(i);
        if (token_tx_mint->txid == txid && token_tx_mint->vout == vout)
        {
            token_name = token_tx_mint->token_name;
            amount = token_tx_mint->token_amount;
            return true;
        }
    }

    i = 0;

    for (; i < vect_transfer_token_data_.size(); ++i)
    {
        token_tx_transfer = vect_transfer_token_data_.at(i);
        if (token_tx_transfer->txid == txid && token_tx_transfer->vout == vout)
        {
            token_name = token_tx_transfer->token_name;
            amount = token_tx_transfer->token_amount;
            return true;
        }
    }

    return false;

}

bool InteractiveBlockChain::GetTokenAddress(const std::string &txid, int vout, std::vector<std::string> &vect_token_address)
{

    unsigned int i = 0;
    TokenData* token_tx_mint = NULL;
    TokenData* token_tx_transfer = NULL;
    for (; i < vect_mint_token_data_.size(); ++i)
    {
        token_tx_mint = vect_mint_token_data_.at(i);
        if (token_tx_mint->txid == txid && token_tx_mint->vout == vout)
        {
            vect_token_address = token_tx_mint->vect_token_out_address;
            return true;
        }
    }

    i = 0;

    for (; i < vect_transfer_token_data_.size(); ++i)
    {
        token_tx_transfer = vect_transfer_token_data_.at(i);
        if (token_tx_transfer->txid == txid && token_tx_transfer->vout == vout)
        {
            vect_token_address = token_tx_transfer->vect_token_out_address;
            return true;
        }
    }

    return false;
}

void InteractiveBlockChain::AddTokenTxData(uint64_t begin, uint64_t end)
{
    uint64_t i = begin;
    CBlockIndex *pblockindex = NULL;
    ContractInterpeter token_validator;
    TokenData* token_tx = NULL;
    for (; i <= end; ++i)
    {
        pblockindex = chainActive[i];
        CBlock block;
        if (ReadBlockFromDisk(block, pblockindex, Params().GetConsensus()))
        {
            for ( unsigned int j = 0; j < block.vtx.size(); j++ )
            {
                std::shared_ptr<const CTransaction> tx = block.vtx.at(j);

                if (tx->IsCoinBase())
                {
                    continue;
                }

                for (unsigned int k = 0; k < tx->vout.size(); k++ )
                {
                    CTxOut out = tx->vout.at(k);
                    if (token_validator.TokenScriptVerify(out.scriptPubKey,false))
                    {

                        if ( token_validator.Get_token_tx_type()  == TOKEN_ISSUE
                             || token_validator.Get_token_tx_type() == TOKEN_TRANSACTION)
                        {
                            token_tx = new  TokenData;
                            token_tx->chain_postion = i ;
                            token_tx->block_postion = j ;
                            token_tx->txid = tx->GetHash().ToString();
                            token_tx->vout = k;
                            token_validator.Get_token_name(token_tx->token_name);
                            token_validator.Get_token_amount(token_tx->token_amount);
                            token_validator.Get_token_address(token_tx->vect_token_out_address);
                            if ( token_validator.Get_token_tx_type()  == TOKEN_ISSUE  )
                                vect_mint_token_data_.push_back(token_tx);
                            else
                                vect_transfer_token_data_.push_back(token_tx);
                            token_tx = NULL;
                        }
                    }
                }

            }
        }
    }
}





