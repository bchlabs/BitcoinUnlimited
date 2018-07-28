#include "tokentxcheck.h"
#include "amount.h"
#include "chain.h"
#include "core_io.h"
#include "dstencode.h"
#include "init.h"
#include "main.h"
#include "net.h"
#include "rpc/server.h"
#include "script/sign.h"
#include "timedata.h"
#include "util.h"
#include "utilmoneystr.h"
#include "txmempool.h"

#include "contract/contractconfig.h"
#include "contract/contracterror.h"

#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#include "wallet/walletdb.h"
#endif

int CheckTokenVin(const UniValue &params)
{

    using namespace std;
    int ret = TOKENOK;
    LOCK2(cs_main, pwalletMain->cs_wallet);

    vector<std::string> token_vin_list  = params.getKeys();
    vector<COutput> vecOutputs;
    assert(pwalletMain != NULL);
    pwalletMain->AvailableCoins(vecOutputs, false, NULL, true);
    int count_utxo =0;
    for (unsigned int i =0; i<token_vin_list.size(); i++)
    {
        string token_vin_txid = token_vin_list[i];
        uint256 hash;
        hash.SetHex(token_vin_txid);
       // isminefilter filter = ISMINE_SPENDABLE;
        if (!pwalletMain->mapWallet.count(hash))
        {
            return TOKENINPUTSIZEERROR;
        }
        for (unsigned int j =0; j<vecOutputs.size(); j++)
        {
             COutput  out = vecOutputs.at(j);
             if (out.tx->GetHash() == hash)
             {
                 count_utxo ++;
                 break;
             }
        }
    }

    if (!count_utxo)
    {
        ret = TOKENINPUTSIZEERROR;
    }

    return ret;
}


// check (txid,vout) is valid, unspent in blockchain or mempool 
bool IsTxidUnspent(const std::string &txid, const uint32_t vout)
{
    if (txid.size() != 64 || !IsHex(txid))
        return false; // invalid txid

    int height = chainActive.Height();
    CBlockIndex *pblockindex = NULL;
    bool txidAndVoutExist = false;

    for (int i = 1; i <= height; ++i) 
    {
        pblockindex = chainActive[i];
        CBlock block;
        if (ReadBlockFromDisk(block, pblockindex, Params().GetConsensus()))
        {          
            for (const auto &tx: block.vtx)
            {
                if (!tx->IsCoinBase())
                {
                    for (const auto &in: tx->vin)
                    {
                        std::string in_hash = in.prevout.hash.ToString();
                        uint32_t in_n = in.prevout.n;
                        if (txid == in_hash && vout == in_n)
                            return false; // (txid.vout) has spend in blockchain
                    }
                }

                if (!txidAndVoutExist && tx->GetHash().ToString() == txid)
                {
                    if (tx->vout.size() > vout)
                        txidAndVoutExist = true;

                    break;
                }
            }
        }
    }

    if (!txidAndVoutExist)
        return false; // (txid,vout) is not in blockchain

    for (const auto &tx: mempool.mapTx) 
    {
        std::cout << tx.GetTx().GetHash().ToString() << std::endl;
        for (const auto &in: tx.GetTx().vin)
        {
            std::string in_hash = in.prevout.hash.ToString();
            uint32_t in_n = in.prevout.n;
            if (txid == in_hash && vout == in_n)
                return false; // (txid,vout) has spend in mempool
        }
    }

    return true;
}


bool TokenInputValid(const std::string &token_input_txid, const uint32_t token_input_vout)
{
    return true;
}

std::string signTokenTxid(const std::string &strAddress,const std::string &strMessage)
{

    CTxDestination dest = DecodeDestination(strAddress);
    if (!IsValidDestination(dest))
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    const CKeyID *keyID = boost::get<CKeyID>(&dest);
    if (!keyID)
        throw JSONRPCError(RPC_TYPE_ERROR, "Address does not refer to key");

    CKey key;
    if (!pwalletMain->GetKey(*keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    std::vector<unsigned char> vchSig = signmessage(strMessage, key);
    if (vchSig.empty())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    return EncodeBase64(&vchSig[0], vchSig.size());
}

bool verifyWitness(const std::string &strSign,const std::string &strAddress,const std::string &strMessage)
{

    LOCK(cs_main);

    CTxDestination destination = DecodeDestination(strAddress);
    if (!IsValidDestination(destination))
    {
        return false;
    }

    const CKeyID *keyID = boost::get<CKeyID>(&destination);
    if (!keyID)
    {
        return false;
    }

    bool fInvalid = false;
    std::vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

    if (fInvalid)
        return false;

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CPubKey pubkey;
    if (!pubkey.RecoverCompact(ss.GetHash(), vchSig))
        return false;

    return (pubkey.GetID() == *keyID);
}


bool WitnessValid(const std::vector<std::string> &witness_data, const std::vector<std::string> &witness_address, const std::string &org_message)
{
    if (witness_data.size() != witness_address.size())
        return false;

    unsigned int size = witness_data.size();
    for (unsigned int i = 0; i < size; i++)
    {
        if (!verifyWitness(witness_data.at(i),witness_address.at(i),org_message))
        {
            return false;
        }
    }

    return true;
}
