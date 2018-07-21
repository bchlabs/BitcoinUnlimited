
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
using namespace std;

static bool verifytokentxid(const std::string &strAddress,const std::string &strSign,const std::string &strMessage)
{

    //LOCK(cs_main);

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
    vector<unsigned char> vchSig = DecodeBase64(strSign.c_str(), &fInvalid);

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

TokenStruct VerifyTokenScript(const CScript &token_script)
{

    TokenStruct ret;
    CScript::const_iterator pc = token_script.begin();
    CScript::const_iterator pend = token_script.end();
   // CScript::const_iterator pbegincodehash = token_script.begin();

    int token_type = -1;
    vector<unsigned char> witness_txid ;
    int witness_vout =0;

    vector<unsigned char> token_name;
    uint64_t  token_amount=0;

    vector<unsigned char> token_vin;
    int token_vin_pos =0;
    CScript sign_token ;
    vector<unsigned char> sign_token_vect;

    int pos =0;
    while (pc <pend)
    {
        opcodetype opcode;
        vector<unsigned char> vchPushValue;

        if (pos == 0 )
        {
            token_script.GetOp(pc, opcode, vchPushValue);

            if ( opcode == OP_RETURN)
            {
                opcode = OP_INVALIDOPCODE;
                continue;
            }

            if (opcode == OP_10 )
            {
                token_type = TOKEN_ISSUE;
            }
            else if (opcode == OP_11)
            {
                token_type = TOKEN_TRANSACTION;
            }
           // CScriptNum scriptNumValue(vchPushValue,true);
           // token_type  = scriptNumValue.getint();
            pos ++ ;

        }
        else if (pos == 1)
        {
            token_script.GetOp(pc, opcode, witness_txid);
            ret.txid =  std::string(witness_txid.begin(),witness_txid.end());
            pos ++;
        }
        else if (pos == 2)
        {
            token_script.GetOp(pc, opcode, vchPushValue);
            CScriptNum scriptNumValue(vchPushValue,true);
            witness_vout  = scriptNumValue.getint();
            ret.vout = witness_vout;
            pos ++ ;
        }
        else if (pos >2  && token_type == TOKEN_ISSUE)
        {
            //  token_script >> witness_txid >> witness_vout >> token_name >> token_amount >> pubkey_vect;
            if(pos == 3)
            {
                token_script.GetOp(pc, opcode, token_name);
                ret.name = std::string(token_name.begin(),token_name.end());
            }
            else if(pos == 4)
            {
                token_script.GetOp(pc, opcode, vchPushValue);
                CScriptNum scriptNumValue(vchPushValue,true);
                token_amount  = scriptNumValue.getint64();
                ret.amount = token_amount;

            }
            else
            {
                 token_script.GetOp(pc, opcode, vchPushValue);

                 ret.address=  std::string(vchPushValue.begin(),vchPushValue.end());
            }
            pos ++;

        }
        else if (pos !=0 && token_type == TOKEN_TRANSACTION)
        {
           //  token_script >> witness_txid >> witness_vout >>token_vin >> token_vin_pos >> sign_token_vect >> token_name >> token_amount >> pubkey_vect;
            if (pos ==3)
            {
                token_script.GetOp(pc, opcode, token_vin);
            }
            else if (pos == 4)
            {
                token_script.GetOp(pc, opcode, vchPushValue);
                CScriptNum scriptNumValue(vchPushValue,true);

                token_vin_pos  = scriptNumValue.getint();
            }
            else if (pos == 5)
            {
                token_script.GetOp(pc, opcode, sign_token_vect);
                sign_token << sign_token_vect;
            }
            else if(pos == 6)
            {
                token_script.GetOp(pc, opcode, token_name);
                ret.name = std::string (token_name.begin(),token_name.end());
            }
            else if(pos == 7)
            {
                token_script.GetOp(pc, opcode, vchPushValue);
                CScriptNum scriptNumValue(vchPushValue,true);
                token_amount  = scriptNumValue.getint64();
                ret.amount = token_amount;
            }
            else
            {
                 token_script.GetOp(pc, opcode, vchPushValue);

                 ret.address = std::string(vchPushValue.begin(),vchPushValue.end());
            }
            pos ++;
        }
        else
        {
            break;
        }
    }

    if ( token_type == TOKEN_TRANSACTION )
    {
        std::string sign_vin = std::string(sign_token_vect.begin(),sign_token_vect.end());
        std::string message_vin  = std::string(token_vin.begin() ,token_vin.end());
        std::cout << "sign_vin: " << sign_vin<< std::endl;
         std::cout << "message_vin: " << message_vin<< std::endl;
        ret.sign_ok = verifytokentxid(ret.address,sign_vin,message_vin);
    }

    return ret;

}
