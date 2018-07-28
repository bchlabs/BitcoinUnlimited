
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
#include "contract/contractconfig.h"


TokenStruct VerifyTokenScript(const CScript &token_script)
{

    TokenStruct ret;
    CScript::const_iterator pc = token_script.begin();
    CScript::const_iterator pend = token_script.end();

    int token_type = -1;
    std::vector<unsigned char> witness_txid ;
    int witness_vout =0;

    std::vector<unsigned char> token_name;
    uint64_t  token_amount=0;

    std::vector<unsigned char> token_vin;
//    int token_vin_pos =0;
    CScript sign_token ;
    std::vector<unsigned char> sign_token_vect;

    int pos =0;
    while (pc <pend)
    {
        opcodetype opcode;
        std::vector<unsigned char> vchPushValue;

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
//                CScriptNum scriptNumValue(vchPushValue,true);

//                token_vin_pos  = scriptNumValue.getint();
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


    return ret;

}
