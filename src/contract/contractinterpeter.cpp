#include "contractinterpeter.h"
#include "script/script.h"
#include "contract/contracterror.h"
#include "contract/rpctoken.h"
#include <string>
#include "contract/tokentxcheck.h"


ContractInterpeter::ContractInterpeter()
{

}

ContractInterpeter::~ContractInterpeter()
{

}

bool ContractInterpeter::PushValueToProgram(programType& program,const valuetype&push_value,unsigned int&size,bool has_set_size)
{

    if ( !has_set_size )
    {
         CScriptNum scriptNumValue(push_value,true);
         size = scriptNumValue.getint();
         if ( size <= 0 )
         {
             return false;
         }
    }
    else
    {
        if ( program.size() < size )
        {
            program.push(push_value);
        }
    }

    return true;
}

bool ContractInterpeter::TokenFlagInterpreter(programType &program_flag, unsigned int flag_size)
{
    if ( flag_size  != 1 )
    {
        return false;
    }

    valuetype value_flag = program_flag.top();
    CScriptNum script_num_value(value_flag ,true);
    token_tx_type_ = script_num_value.getint();

    if ( token_tx_type_ !=  TOKEN_ISSUE && token_tx_type_ != TOKEN_TRANSACTION )
    {
        token_tx_type_ = TOKENTXNOFIND;
        return false;
    }

    return true;
}

bool ContractInterpeter::TokenWitnessInterpreter(programType &program_witness,unsigned int witness_size)
{
    if ( ( token_tx_type_ == TOKEN_ISSUE || token_tx_type_ == TOKEN_TRANSACTION ) && witness_size  != 2)
    {
        token_error_type_ = TOKENFORMATERROR;
        return false;
    }

    std::string witness_txid = std::string(program_witness.top().begin(),program_witness.top().end());
    program_witness.pop();
    int witness_vout = CScriptNum(program_witness.top(),true).getint();

    if ( !WitnessTxidValid(witness_txid,witness_vout) )
    {
        token_error_type_ = TOKENWITNESSERROR;
        return false;
    }

    return true;

}

bool ContractInterpeter::TokenInputInterpreter(programType &program_input,unsigned int input_size)
{

    if ( token_tx_type_ == TOKEN_ISSUE || token_tx_type_ == TOKEN_TRANSACTION )
    {
        token_error_type_ = TOKENFORMATERROR;
        return false;
    }

    std::string token_input_txid = std::string(program_input.top().begin(),program_input.top().end());
    program_input.pop();
    int token_input_vout = CScriptNum(program_input.top(),true).getint();

    if ( !TokenInputValid(token_input_txid,token_input_vout) )
    {
        token_error_type_ = TOKENWITNESSERROR;
        return false;
    }

    return true;

}

bool ContractInterpeter::TokenOutputInterpreter(programType &program_output,unsigned int output_size)
{
    if ( token_tx_type_ == TOKEN_ISSUE && token_tx_type_ == TOKEN_TRANSACTION )
    {
        token_error_type_ = TOKENFORMATERROR;
        return false;
    }

    return true;
}

bool ContractInterpeter::TokenScriptVerify(const CScript &token_script)
{
    bool ret = true;

    CScript::const_iterator pc = token_script.begin();
    CScript::const_iterator pend = token_script.end();
    unsigned int flag_size = 0;
    programType program_flag;
    unsigned int witness_size = 0;
    programType program_witness;
    unsigned int input_size = 0;
    programType program_input;
    unsigned int output_size = 0;
    programType program_output;


    opcodetype opcode;
    while ( pc < pend )
    {
        valuetype vchPushValue;
        if ( token_script.GetOp(pc, opcode, vchPushValue) )
        {
            if ( program_flag.size() == 0 )
            {
                if ( opcode == OP_RETURN )
                {
                    continue;
                }
                else
                {
                    token_error_type_ = TOKENFORMATERROR;
                    ret = false;
                    break;
                }
            }
        }
        else
        {
            //  the value  of flag'program  push
            if ( flag_size != program_flag.size() )
            {
                if ( flag_size == 0)
                {
                    ret = PushValueToProgram(program_flag,vchPushValue,flag_size,false);
                }
                else
                {
                    ret = PushValueToProgram(program_flag,vchPushValue,flag_size,true);
                }

                if ( !ret )
                {
                    token_error_type_ = TOKENFLAGSIZEERROR;
                    break;
                }
            }
            //  the value  of witness'program  push
            else if ( witness_size != program_witness.size() )
            {
                if ( witness_size == 0)
                {
                    ret = PushValueToProgram(program_witness,vchPushValue,witness_size,false);
                }
                else
                {
                    ret = PushValueToProgram(program_witness,vchPushValue,witness_size,true);
                }

                if ( !ret )
                {
                    token_error_type_ = TOKENWITNESSSIZEEROR;
                    break;
                }
            }
            //  the value  of input'program  push
            else if ( input_size != program_input.size() )
            {
                if ( input_size == 0)
                {
                    ret = PushValueToProgram(program_input,vchPushValue,input_size,false);
                }
                else
                {
                    ret = PushValueToProgram(program_input,vchPushValue,input_size,true);
                }

                if ( !ret )
                {
                    token_error_type_ = TOKENINPUTSIZEERROR;
                    break;
                }
            }
            //  the value  of output'program  push
            else if ( output_size != program_output.size() )
            {
                if ( output_size == 0)
                {
                    ret = PushValueToProgram(program_output,vchPushValue,output_size,false);
                }
                else
                {
                    ret = PushValueToProgram(program_output,vchPushValue,output_size,true);
                }

                if ( !ret )
                {
                    token_error_type_ = TOKENOUTPUTSIZEERROR;
                    break;
                }
            }
        }
    }

    if ( !TokenFlagInterpreter(program_flag,flag_size) )
    {
        return false;
    }

    if ( !TokenWitnessInterpreter(program_witness,witness_size) )
    {
        return false;
    }

    return ret;

}

int ContractInterpeter::Get_token_error_type()
{
    return token_error_type_;
}
