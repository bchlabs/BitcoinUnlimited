#include "base58.h"
#include "contract/rpctoken.h"
#include "init.h"
#include "main.h"
#include "rpc/server.h"
#include "script/script.h"
#include "script/script_error.h"
#include "script/sign.h"
#include "script/standard.h"
#include "sync.h"
#include "dstencode.h"
#include "txmempool.h"
#include "core_io.h"
#include "contract/tokentxcheck.h"
#include "contract/tokeninterpreter.h"
#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#endif

#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>

static void TxInErrorToJSON(const CTxIn &txin, UniValue &vErrorsRet, const std::string &strMessage)
{
    UniValue entry(UniValue::VOBJ);
    entry.push_back(Pair("txid", txin.prevout.hash.ToString()));
    entry.push_back(Pair("vout", (uint64_t)txin.prevout.n));
    entry.push_back(Pair("scriptSig", HexStr(txin.scriptSig.begin(), txin.scriptSig.end())));
    entry.push_back(Pair("sequence", (uint64_t)txin.nSequence));
    entry.push_back(Pair("error", strMessage));
    vErrorsRet.push_back(entry);
}

UniValue tokenmint(const UniValue& params, bool fHelp)
{
	if (fHelp || params.size() != 3)
		throw std::runtime_error(
                "tokenmint \"feetx\" \"txid\" \n"

				"\nAdds a transaction input to the transaction.\n"

				"\nIf no raw transaction is provided, a new transaction is created.\n"

				"\nArguments:\n"
                "1. feetx (rawtx)                \n"
                "2. witness_token(txid)          \n"
                "3. tokenInfo)                   \n"

				"\nResult:\n"
				"\"rawtx\"                 (string) the hex-encoded modified raw transaction\n"

				"\nExamples\n"
                +HelpExampleCli("tokenmint", "\"[{\\\"txid\\\":\\\"myid\\\",\\\"vout\\\":0}]\" \"{\\\"witness_txid\\\":0 } \" \"{\\\"name\\\":\"token_name\" ,\\\"amount\":1000000,\\\"address\\\":0.01}\"")
                +HelpExampleRpc("tokenmint", "\"[{\\\"txid\\\":\\\"myid\\\",\\\"vout\\\":0}]\" \"{\\\"witness_txid\\\":0 } \" \"{\\\"name\\\":\"token_name\" ,\\\"amount\":1000000,\\\"address\\\":0.01}\"")
                );

    UniValue inputs = params[0].get_array();
    UniValue witness_utxo = params[1].get_obj();
    UniValue token_sendTo = params[2].get_obj();

    CMutableTransaction rawTx;

    for (unsigned int idx = 0; idx < inputs.size(); idx++)
    {
        const UniValue& input = inputs[idx];
        const UniValue& o = input.get_obj();
        uint256 txid = ParseHashO(o, "txid");

        const UniValue& vout_v = find_value(o, "vout");
        if (!vout_v.isNum())
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, missing vout key");

        int nOutput = vout_v.get_int();
        if (nOutput < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, vout must be positive");

        uint32_t nSequence = (rawTx.nLockTime ? std::numeric_limits<uint32_t>::max() - 1 : std::numeric_limits<uint32_t>::max());

        // set the sequence number if passed in the parameters object
        const UniValue& sequenceObj = find_value(o, "sequence");
        if (sequenceObj.isNum()) {
            int64_t seqNr64 = sequenceObj.get_int64();
            if (seqNr64 < 0 || seqNr64 > std::numeric_limits<uint32_t>::max())
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, sequence number is out of range");
            else
                nSequence = (uint32_t)seqNr64;
        }

        CTxIn in(COutPoint(txid, nOutput), CScript(), nSequence);
        rawTx.vin.push_back(in);
    }

    int error_token = CheckTokenVin(witness_utxo);
    if ( error_token )
    {
        throw JSONRPCError(error_token, std::string("Token vin error: "));
    }

    CScript script_token_tx;
    script_token_tx << OP_RETURN << TOKEN_ISSUE;

    std::vector<std::string> token_witness_list  = witness_utxo.getKeys();
    for (unsigned int i =0; i<token_witness_list.size(); i++)
    {
        std::string witness_txid = token_witness_list.at(i);
        int vin_pos = witness_utxo[witness_txid].get_int();
        if ( !IsTxidUnspent(witness_txid,(unsigned int)vin_pos))
        {
            throw JSONRPCError(-8, std::string("witness id  has spent"));
        }
        script_token_tx << ToByteVector(witness_txid);
        //int vin_pos = witness_utxo[witness_txid].get_int();
        script_token_tx << vin_pos ;
    }

    std::vector<std::string> addrList = token_sendTo.getKeys();
    BOOST_FOREACH(const std::string& name_, addrList)
    {
        if (name_ == "name")
        {
            script_token_tx << ToByteVector(token_sendTo[name_].getValStr());
        }
        else if (name_ == "amount")
        {
            CAmount amount  = token_sendTo[name_].get_int64();
            script_token_tx << amount;
        }
        else
        {
            CTxDestination destination = DecodeDestination(name_);
            if (!IsValidDestination(destination))
            {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("Invalid Bitcoin address: ") + name_);
            }

            // CScript scriptPubKey = GetScriptForDestination(destination);
            // script_token_tx  += scriptPubKey;
            CKeyID keyID = boost::get<CKeyID>(destination);
            script_token_tx << ToByteVector(keyID);
        }

    }

    CTxOut out(0, script_token_tx);
    rawTx.vout.push_back(out);

// sign tx
#ifdef ENABLE_WALLET
    LOCK2(cs_main, pwalletMain ? &pwalletMain->cs_wallet : NULL);
#else
    LOCK(cs_main);
#endif

    std::vector<CMutableTransaction> txVariants;
    txVariants.push_back(rawTx);

    if (txVariants.empty())
        throw JSONRPCError(RPC_DESERIALIZATION_ERROR, "Missing transaction");

    // mergedTx will end up with all the signatures; it
    // starts as a clone of the rawtx:
    CMutableTransaction mergedTx(txVariants[0]);

    // Fetch previous transactions (inputs):
    CCoinsView viewDummy;
    CCoinsViewCache view(&viewDummy);
    {
        READLOCK(mempool.cs);
        CCoinsViewCache &viewChain = *pcoinsTip;
        CCoinsViewMemPool viewMempool(&viewChain, mempool);
        view.SetBackend(viewMempool); // temporarily switch cache backend to db+mempool view

        BOOST_FOREACH (const CTxIn &txin, mergedTx.vin)
        {
            view.AccessCoin(txin.prevout); // Load entries from viewChain into view; can fail.
        }

        view.SetBackend(viewDummy); // switch back to avoid locking mempool for too long
    }

    bool fGivenKeys = false;
    CBasicKeyStore tempKeystore;

#ifdef ENABLE_WALLET
    if (pwalletMain)
        EnsureWalletIsUnlocked();
#endif

#ifdef ENABLE_WALLET
    const CKeyStore &keystore = ((fGivenKeys || !pwalletMain) ? tempKeystore : *pwalletMain);
#else
    const CKeyStore &keystore = tempKeystore;
#endif

    int nHashType = SIGHASH_ALL;
    bool pickedForkId = false;
    if (!pickedForkId) // If the user didn't specify, use the configured default for the hash type
    {
        if (IsUAHFforkActiveOnNextBlock(chainActive.Tip()->nHeight))
        {
            nHashType |= SIGHASH_FORKID;
            pickedForkId = true;
        }
    }

    bool fHashSingle = ((nHashType & ~(SIGHASH_ANYONECANPAY | SIGHASH_FORKID)) == SIGHASH_SINGLE);

    // Script verification errors
    UniValue vErrors(UniValue::VARR);

    // Use CTransaction for the constant parts of the
    // transaction to avoid rehashing.
    const CTransaction txConst(mergedTx);
    // Sign what we can:
    for (unsigned int i = 0; i < mergedTx.vin.size(); i++)
    {
        CTxIn &txin = mergedTx.vin[i];
        const Coin &coin = view.AccessCoin(txin.prevout);
        if (coin.IsSpent())
        {
            TxInErrorToJSON(txin, vErrors, "Input not found or already spent");
            continue;
        }
        const CScript &prevPubKey = coin.out.scriptPubKey;
        const CAmount &amount = coin.out.nValue;

        // Only sign SIGHASH_SINGLE if there's a corresponding output:
        if (!fHashSingle || (i < mergedTx.vout.size()))
            SignSignature(keystore, prevPubKey, mergedTx, i, amount, nHashType);

        // ... and merge in other signatures:
        if (pickedForkId)
        {
            BOOST_FOREACH (const CMutableTransaction &txv, txVariants)
            {
                txin.scriptSig = CombineSignatures(prevPubKey,
                    TransactionSignatureChecker(&txConst, i, amount, SCRIPT_ENABLE_SIGHASH_FORKID), txin.scriptSig,
                    txv.vin[i].scriptSig);
            }
            ScriptError serror = SCRIPT_ERR_OK;
            if (!VerifyScript(txin.scriptSig, prevPubKey, STANDARD_SCRIPT_VERIFY_FLAGS | SCRIPT_ENABLE_SIGHASH_FORKID,
                    MutableTransactionSignatureChecker(&mergedTx, i, amount, SCRIPT_ENABLE_SIGHASH_FORKID), &serror))
            {
                TxInErrorToJSON(txin, vErrors, ScriptErrorString(serror));
            }
        }
        else
        {
            BOOST_FOREACH (const CMutableTransaction &txv, txVariants)
            {
                txin.scriptSig = CombineSignatures(prevPubKey, TransactionSignatureChecker(&txConst, i, amount, 0),
                    txin.scriptSig, txv.vin[i].scriptSig);
            }
            ScriptError serror = SCRIPT_ERR_OK;
            if (!VerifyScript(txin.scriptSig, prevPubKey, STANDARD_SCRIPT_VERIFY_FLAGS,
                    MutableTransactionSignatureChecker(&mergedTx, i, amount, 0), &serror))
            {
                TxInErrorToJSON(txin, vErrors, ScriptErrorString(serror));
            }
        }
    }

    UniValue result(UniValue::VOBJ);
    if (!vErrors.empty())
    {
        result.push_back(Pair("hex", EncodeHexTx(mergedTx)));
        result.push_back(Pair("errors", vErrors));
    }

    // send tx
    uint256 hashTx = mergedTx.GetHash();

    bool fOverrideFees = false;
    TransactionClass txClass = TransactionClass::DEFAULT;

    CCoinsViewCache &view2 = *pcoinsTip;
    bool fHaveChain = false;
    for (size_t o = 0; !fHaveChain && o < mergedTx.vout.size(); o++)
    {
        const Coin &existingCoin = view2.AccessCoin(COutPoint(hashTx, o));
        fHaveChain = !existingCoin.IsSpent();
    }
    bool fHaveMempool = mempool.exists(hashTx);
    if (!fHaveMempool && !fHaveChain)
    {
        // push to local node and sync with wallets
        CValidationState state;
        bool fMissingInputs;
        if (!AcceptToMemoryPool(mempool, state, mergedTx, false, &fMissingInputs, false, !fOverrideFees, txClass))
        {
            if (state.IsInvalid())
            {
                throw JSONRPCError(
                    RPC_TRANSACTION_REJECTED, strprintf("%i: %s", state.GetRejectCode(), state.GetRejectReason()));
            }
            else
            {
                if (fMissingInputs)
                {
                    throw JSONRPCError(RPC_TRANSACTION_ERROR, "Missing inputs");
                }
                throw JSONRPCError(RPC_TRANSACTION_ERROR, state.GetRejectReason());
            }
        }
#ifdef ENABLE_WALLET
        else
            SyncWithWallets(mergedTx, NULL, -1);
#endif
    }
    else if (fHaveChain)
    {
        throw JSONRPCError(RPC_TRANSACTION_ALREADY_IN_CHAIN, "transaction already in block chain");
    }
    RelayTransaction(mergedTx);

    return hashTx.GetHex();
}


UniValue tokentransfer(const UniValue& params, bool fHelp)
{
    using namespace std;
    if (fHelp || params.size() != 3)
        throw std::runtime_error(
                "tokentransaction \"feetx\" \"txid\" n\n"

                "\nAdds a transaction input to the transaction.\n"

                "\nIf no raw transaction is provided, a new transaction is created.\n"

                "\nArguments:\n"
                "1. feetx (rawtx)                \n"
                "2. witness_token(witness_txid ,token_txid ,sigture)          \n"
                "3. tokenInfo)                   \n"

                "\nResult:\n"
                "\"rawtx\"                 (string) the hex-encoded modified raw transaction\n"

                "\nExamples\n"
                +HelpExampleCli("tokentransfer", "\"[{\\\"txid\\\":\\\"myid\\\",\\\"vout\\\":0}]\" \"{\\\"witness_txid\\\":0 ,\\\"token_txid\\\":0,\\\"sign\\\":1} \" \"{\\\"name\\\":\"token_name\" ,\\\"amount\":1000000,\\\"address\\\":0.01}\"")
                +HelpExampleRpc("tokentransfer", "\"[{\\\"txid\\\":\\\"myid\\\",\\\"vout\\\":0}]\" \"{\\\"witness_txid\\\":0 ,\\\"token_txid\\\":0,\\\"sign\\\":1} \" \"{\\\"name\\\":\"token_name\" ,\\\"amount\":1000000,\\\"address\\\":0.01}\"")
                );

    UniValue inputs = params[0].get_array();
    UniValue witness_utxo = params[1].get_obj();
    UniValue token_sendTo = params[2].get_obj();

    CMutableTransaction rawTx;

    for (unsigned int idx = 0; idx < inputs.size(); idx++)
    {
        const UniValue& input = inputs[idx];
        const UniValue& o = input.get_obj();
        uint256 txid = ParseHashO(o, "txid");

        const UniValue& vout_v = find_value(o, "vout");
        if (!vout_v.isNum())
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, missing vout key");

        int nOutput = vout_v.get_int();
        if (nOutput < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, vout must be positive");

        uint32_t nSequence = (rawTx.nLockTime ? std::numeric_limits<uint32_t>::max() - 1 : std::numeric_limits<uint32_t>::max());

        // set the sequence number if passed in the parameters object
        const UniValue& sequenceObj = find_value(o, "sequence");
        if (sequenceObj.isNum()) {
            int64_t seqNr64 = sequenceObj.get_int64();
            if (seqNr64 < 0 || seqNr64 > std::numeric_limits<uint32_t>::max())
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, sequence number is out of range");
            else
                nSequence = (uint32_t)seqNr64;
        }

        CTxIn in(COutPoint(txid, nOutput), CScript(), nSequence);
        rawTx.vin.push_back(in);
    }

    int error_token = CheckTokenVin(witness_utxo);
    if ( error_token )
    {
        throw JSONRPCError(error_token, std::string("Token vin error: "));
    }

    CScript script_token_tx;
    script_token_tx << OP_RETURN << TOKEN_ISSUE;

    std::vector<std::string> token_witness_list  = witness_utxo.getKeys();
    for (unsigned int i =0; i<token_witness_list.size(); i++)
    {

        string witness_name = token_witness_list.at(i);
        if (witness_name == "sign")
        {
            script_token_tx << ToByteVector(witness_name);
        }
        else
        {
            int vin_pos = witness_utxo[witness_name].get_int();
            if ( !IsTxidUnspent(witness_name,(unsigned int)vin_pos))
            {
                throw JSONRPCError(-8, std::string("witness id  has spent"));
            }
            script_token_tx << ToByteVector(witness_name);


            script_token_tx << vin_pos ;
        }

    }

    std::vector<string> addrList = token_sendTo.getKeys();
    BOOST_FOREACH(const string& name_, addrList)
    {
        if (name_ == "name")
        {
            script_token_tx << ToByteVector(token_sendTo[name_].getValStr());
        }
        else if (name_ == "amount")
        {
            CAmount amount  = token_sendTo[name_].get_int64();
            script_token_tx << amount;
        }
        else
        {
            CTxDestination destination = DecodeDestination(name_);
            if (!IsValidDestination(destination))
            {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("Invalid Bitcoin address: ") + name_);
            }

            // CScript scriptPubKey = GetScriptForDestination(destination);
            // script_token_tx  += scriptPubKey;
            CKeyID keyID = boost::get<CKeyID>(destination);
            script_token_tx << ToByteVector(keyID);
        }

    }

    CTxOut out(0, script_token_tx);
    rawTx.vout.push_back(out);
    return EncodeHexTx(rawTx);
}


UniValue listtokeninfo(const UniValue &params, bool fHelp)
{    
    if (fHelp || params.size() != 0)
        throw std::runtime_error("listtokeninfo \n");

    int height = chainActive.Height();
    CBlockIndex *pblockindex = NULL;

    for (int i = 90; i <= height; ++i) 
    {
        pblockindex = chainActive[i];
        CBlock block;
        if (ReadBlockFromDisk(block, pblockindex, Params().GetConsensus()))
        {          
            for (const auto &tx : block.vtx)
            {
                if (!tx->IsCoinBase())
                {
                    for (const auto &out: tx->vout)
                    {
                        CScript pk = out.scriptPubKey;
                        if (pk[0] == OP_RETURN && pk[1] == OP_10)
                        {
                            // RETURN 10 0x40 
                            // 0x66396231366332396464343933313066623065363063393232663062323331343164386536326432373538373435353561643963363666306264653138646336 
                            // 1 0x03 0x65736b 0x02 0x1027 0x14 0xd5a0da39ece93072133fc42e4858974586ece592
                            CScript pubkey;
                            VerifyTokenScript(pk, pubkey);
                        }
                    }
                }
            }
        }
    }


    UniValue result(UniValue::VOBJ);
    return result;
}


static const CRPCCommand commands[] =
{ //  category                             name                            actor (function)               okSafeMode
  //  ------------------------------------ ------------------------------- ------------------------------ ----------
#ifdef ENABLE_WALLET
    { "constract token", "tokenmint",                                    &tokenmint,               false },
    { "constract token", "tokentransfer",                               &tokentransfer,                    false },
    { "constract token", "listtokeninfo",                               &listtokeninfo,                    false },
#endif
};


void RegisterContractTokenCommands(CRPCTable &tableRPC)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}



