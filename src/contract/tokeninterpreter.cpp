#include "contract/tokeninterpreter.h"
#include "script/script.h"
#include "contract/tokentxcheck.h"
#include "contract/rpctoken.h"
using namespace std;
TokenStruct& VerifyTokenScript(const CScript &token_script)
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
                pos ++;
            }
            else if (pos == 4)
            {
                token_script.GetOp(pc, opcode, vchPushValue);
                CScriptNum scriptNumValue(vchPushValue,true);

                token_vin_pos  = scriptNumValue.getint();
            }
            else if (pos == 5)
            {
                token_script.GetOp(pc, opcode, vchPushValue);
                sign_token << vchPushValue;
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
