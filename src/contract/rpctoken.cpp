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
#include "core_io.h"
#include "contract/tokentxcheck.h"
#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#endif
UniValue tokenmint(const UniValue& params, bool fHelp)
{
    using namespace std;
	if (fHelp || params.size() != 3)
		throw std::runtime_error(
                "tokenmint \"feetx\" \"txid\" n\n"

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
//                + HelpExampleCli("tokenmint", "\"01000000000000000000\" \"b006729017df05eda586df9ad3f8ccfee5be340aadf88155b784d1fc0e8342ee\" 0")
//                + HelpExampleRpc("tokenmint", "\"01000000000000000000\", \"b006729017df05eda586df9ad3f8ccfee5be340aadf88155b784d1fc0e8342ee\", 0")
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
        string witness_txid = token_witness_list.at(i);
        int vin_pos = witness_utxo[witness_txid].get_int();
        if ( !IsTxidUnspent(witness_txid,(unsigned int)vin_pos))
        {
            throw JSONRPCError(-8, std::string("witness id  has spent"));
        }
        script_token_tx << ToByteVector(witness_txid);
        //int vin_pos = witness_utxo[witness_txid].get_int();
        script_token_tx << vin_pos ;
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

            CScript scriptPubKey = GetScriptForDestination(destination);
            script_token_tx  += scriptPubKey;
        }

    }

    CTxOut out(0, script_token_tx);
    rawTx.vout.push_back(out);
    return EncodeHexTx(rawTx);
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
//                + HelpExampleCli("tokentransfer", "\"01000000000000000000\" \"b006729017df05eda586df9ad3f8ccfee5be340aadf88155b784d1fc0e8342ee\" 0")
//                + HelpExampleRpc("tokentransfer", "\"01000000000000000000\", \"b006729017df05eda586df9ad3f8ccfee5be340aadf88155b784d1fc0e8342ee\", 0")
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

            CScript scriptPubKey = GetScriptForDestination(destination);
            script_token_tx +=  scriptPubKey;
        }

    }

    CTxOut out(0, script_token_tx);
    rawTx.vout.push_back(out);
    return EncodeHexTx(rawTx);


//      UniValue result(UniValue::VOBJ);
//      return result;
}
UniValue listtokeninfo(const UniValue &params, bool fHelp)
{
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




