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
#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#endif
UniValue tokenissue(const UniValue& params, bool fHelp)
{
    using namespace std;
	if (fHelp || params.size() != 3)
		throw std::runtime_error(
				"token_issue \"rawtx\" \"txid\" n\n"

				"\nAdds a transaction input to the transaction.\n"

				"\nIf no raw transaction is provided, a new transaction is created.\n"

				"\nArguments:\n"
				"1. rawtx                (string, required) the raw transaction to extend (can be null)\n"
				"2. txid                 (string, required) the hash of the input transaction\n"
				"3. n                    (number, required) the index of the transaction output used as input\n"

				"\nResult:\n"
				"\"rawtx\"                 (string) the hex-encoded modified raw transaction\n"

				"\nExamples\n"
                + HelpExampleCli("token_issue", "\"01000000000000000000\" \"b006729017df05eda586df9ad3f8ccfee5be340aadf88155b784d1fc0e8342ee\" 0")
                + HelpExampleRpc("token_issue", "\"01000000000000000000\", \"b006729017df05eda586df9ad3f8ccfee5be340aadf88155b784d1fc0e8342ee\", 0")
				);

    UniValue inputs = params[0].get_array();
    UniValue sindTo_vin = params[1].get_obj();
    UniValue sendTo = params[2].get_obj();

    CMutableTransaction rawTx;

    if (params.size() > 2 && !params[2].isNull()) {
        int64_t nLockTime = params[2].get_int64();
        if (nLockTime < 0 || nLockTime > std::numeric_limits<uint32_t>::max())
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, locktime out of range");
        rawTx.nLockTime = nLockTime;
    }

    for (unsigned int idx = 0; idx < inputs.size(); idx++) {
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


    CScript scriptData;
    scriptData << OP_RETURN << TOKEN_ISSUE;

    std::vector<std::string> token_vin_list  = sendTo.getKeys();
    for (unsigned int i =0;i<token_vin_list.size();i++)
    {
        string token_vin_txid = token_vin_list[i];
        scriptData << ParseHex(token_vin_txid);
        int vin_pos = sindTo_vin[token_vin_txid].get_int();
        scriptData << vin_pos ;
    }

    std::vector<string> addrList = sendTo.getKeys();
    BOOST_FOREACH(const string& name_, addrList)
    {
        if (name_ == "name")
        {
            std::vector<unsigned char> data = ParseHexV(sendTo[name_].getValStr(),"name");
            scriptData << data;
        }
        else if (name_ == "amout")
        {
            CAmount amount  = sendTo[name_].get_int64();
            scriptData << amount;
        }
        else
        {
            CTxDestination destination = DecodeDestination(name_);
            if (!IsValidDestination(destination))
            {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("Invalid Bitcoin address: ") + name_);
            }

            CScript scriptPubKey = GetScriptForDestination(destination);
            scriptData +=  scriptPubKey;
        }

    }

    CTxOut out(0, scriptData);
    rawTx.vout.push_back(out);
    return EncodeHexTx(rawTx);
}

UniValue tokentransaction(const UniValue& params, bool fHelp)
{
    using namespace std;
    if (fHelp || params.size() != 3)
        throw std::runtime_error(
                "token_transaction \"rawtx\" \"txid\" n\n"

                "\nAdds a transaction input to the transaction.\n"

                "\nIf no raw transaction is provided, a new transaction is created.\n"

                "\nArguments:\n"
                "1. rawtx                (string, required) the raw transaction to extend (can be null)\n"
                "2. txid                 (string, required) the hash of the input transaction\n"
                "3. n                    (number, required) the index of the transaction output used as input\n"

                "\nResult:\n"
                "\"rawtx\"                 (string) the hex-encoded modified raw transaction\n"

                "\nExamples\n"
                + HelpExampleCli("token_transaction", "\"01000000000000000000\" \"b006729017df05eda586df9ad3f8ccfee5be340aadf88155b784d1fc0e8342ee\" 0")
                + HelpExampleRpc("token_transaction", "\"01000000000000000000\", \"b006729017df05eda586df9ad3f8ccfee5be340aadf88155b784d1fc0e8342ee\", 0")
                );

    UniValue inputs = params[0].get_array();
    UniValue sindTo_vin = params[1].get_obj();
    UniValue sendTo = params[2].get_obj();

    CMutableTransaction rawTx;

    if (params.size() > 2 && !params[2].isNull()) {
        int64_t nLockTime = params[2].get_int64();
        if (nLockTime < 0 || nLockTime > std::numeric_limits<uint32_t>::max())
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, locktime out of range");
        rawTx.nLockTime = nLockTime;
    }

    for (unsigned int idx = 0; idx < inputs.size(); idx++) {
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


    CScript scriptData;
    scriptData << OP_RETURN << TOKEN_TRANSACTION;

    std::vector<std::string> token_vin_list  = sendTo.getKeys();
    for (unsigned int i =0;i<token_vin_list.size();i++)
    {
        string token_vin_txid = token_vin_list[i];
        scriptData << ParseHex(token_vin_txid);
        int vin_pos = sindTo_vin[token_vin_txid].get_int();
        scriptData << vin_pos ;
    }

    std::vector<string> addrList = sendTo.getKeys();
    for (unsigned int i =0;i < addrList.size();i++ )
    {
        string name_ = addrList.at(i);
        if (name_ == "name")
        {
            std::vector<unsigned char> data = ParseHexV(sendTo[name_].getValStr(),"name");
            scriptData << data;
        }
        else if (name_ == "amout")
        {
            CAmount amount  = sendTo[name_].get_int64();
            scriptData << amount;
        }
        else
        {
            CTxDestination destination = DecodeDestination(name_);
            if (!IsValidDestination(destination))
            {
                throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, std::string("Invalid Bitcoin address: ") + name_);
            }

            CScript scriptPubKey = GetScriptForDestination(destination);
            scriptData +=  scriptPubKey;
        }

    }

    CTxOut out(0, scriptData);
    rawTx.vout.push_back(out);
    return EncodeHexTx(rawTx);


//      UniValue result(UniValue::VOBJ);
//      return result;
}

static const CRPCCommand commands[] =
{ //  category                             name                            actor (function)               okSafeMode
  //  ------------------------------------ ------------------------------- ------------------------------ ----------
#ifdef ENABLE_WALLET
    { "constract token", "tokenissue",                                    &tokenissue,               false },
    { "constract token", "tokentransaction",                               &tokentransaction,                    false },
#endif
};


void RegisterContractTokenCommands(CRPCTable &tableRPC)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}



