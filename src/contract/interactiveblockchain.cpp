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

    for ( unsigned int i = 0; i < vect_mint_token_data_.size() ; ++i )
    {
        delete vect_mint_token_data_.at(i);
    }

    for ( unsigned int i = 0; i < vect_transfer_token_data_.size() ; ++i )
    {
        delete vect_transfer_token_data_.at(i);
    }
    vect_mint_token_data_.clear();
    vect_transfer_token_data_.clear();
}

bool InteractiveBlockChain::GetTokenNameAmount(const std::__cxx11::string &txid, int vout, std::__cxx11::string &token_name, uint64_t &amount)
{
    unsigned int i = 0;


    std::string token_txid;
    //int token_vout = 0;
    for (; i < vect_mint_token_data_.size(); ++i)
    {
//        GetTokenMintTxidVout(i,token_txid,token_vout);

//        if (txid == token_txid && token_vout == vout)
//        {
//            GetTokenMintNameAmount(i,token_name,amount);
//            return true;
//        }
    }

    i = 0;

    for (; i < vect_transfer_token_data_.size(); ++i)
    {
//        GetTokenTransferTxidVout(i,token_txid,token_vout);

//        if (txid == token_txid && token_vout == vout)
//        {
//            GetTokenTransferNameAmount(i,token_name,amount);
//            return true;
//        }
    }

    return true;

}

void InteractiveBlockChain::GetTokenTransferInfo(int pos, std::__cxx11::string &token_txid, int &vout, std::__cxx11::string &token_name, uint64_t &amount)
{
    CBlockIndex *pblockindex = NULL;
    TokenData* token_tx_mint = vect_mint_token_data_.at(pos);
    pblockindex = chainActive[ token_tx_mint->chain_postion ];
    CBlock block;
    if (ReadBlockFromDisk(block, pblockindex, Params().GetConsensus()))
    {
        std::shared_ptr<const CTransaction> tx = block.vtx.at(token_tx_mint->block_postion);
        token_txid = tx->GetHash().ToString();
    }
    vout = token_tx_mint->vout;
}



void InteractiveBlockChain::GetTokenMintNameAmount(int pos, std::__cxx11::string &token_name, uint64_t &amount)
{
    TokenData* token_tx_mint = vect_mint_token_data_.at(pos);
    token_name = token_tx_mint->token_name;
    amount = token_tx_mint->token_amount;
}



void InteractiveBlockChain::AddTokenTxData(uint64_t begin, uint64_t end)
{
    uint64_t i = begin;
    CBlockIndex *pblockindex = NULL;
    ContractInterpeter token_validator;
    TokenData* token_tx_mint = NULL;
    ContractData* token_tx_transfer = NULL;
    for ( ; i <= end; ++i)
    {
        pblockindex = chainActive[i];
        CBlock block;
        if (ReadBlockFromDisk(block, pblockindex, Params().GetConsensus()))
        {
            for ( unsigned int j = 0; j < block.vtx.size(); j++ )
            {
                std::shared_ptr<const CTransaction> tx = block.vtx.at(j);

                if ( tx->IsCoinBase() )
                {
                    continue;
                }

                for ( unsigned int k = 0; k < tx->vout.size(); k++ )
                {
                    CTxOut out = tx->vout.at(k);
                    if ( token_validator.TokenScriptVerify( out.scriptPubKey ) )
                    {
                        if ( token_validator.Get_token_tx_type()  == TOKEN_ISSUE )
                        {
                            token_tx_mint = new  TokenData;
                            token_tx_mint->chain_postion = i ;
                            token_tx_mint->block_postion = j ;
                            token_tx_mint->vout = k;
                            //token_tx_mint->token_name = token_validator.Get_token_name();
                            //token_tx_mint->token_amount = token_validator.Get_token_amount();
                            vect_mint_token_data_.push_back(token_tx_mint);
                            token_tx_mint = NULL;
                        }
                        else if ( token_validator.Get_token_tx_type() == TOKEN_TRANSACTION )
                        {
                            token_tx_transfer = new  ContractData;
                            token_tx_transfer->chain_postion = i ;
                            token_tx_transfer->block_postion = j ;
                            token_tx_transfer->vout = k;
                            vect_transfer_token_data_.push_back(token_tx_transfer);
                        }
                    }
                }

            }
        }
    }
}



