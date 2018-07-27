#include "contractinterpeter.h"
#include "script/script.h"
#include "contract/contracterror.h"
#include "contract/rpctoken.h"
#include <string>
#include "contract/tokentxcheck.h"
#include "contract/contractconfig.h"


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
            program.push_back(push_value);
        }
    }

    return true;
}

bool ContractInterpeter::TokenFlagInterpreter()
{
    if ( flag_size_  != 1 )
    {
        return false;
    }

    valuetype value_flag = program_flag_.back();
    CScriptNum script_num_value(value_flag ,true);
    token_tx_type_ = script_num_value.getint();

    if ( token_tx_type_ !=  TOKEN_ISSUE && token_tx_type_ != TOKEN_TRANSACTION )
    {
        token_tx_type_ = TOKENTXNOFIND;
        return false;
    }

    return true;
}

bool ContractInterpeter::TokenWitnessInterpreter()
{
    if ( ( token_tx_type_ == TOKEN_ISSUE || token_tx_type_ == TOKEN_TRANSACTION ) )
    {
        token_error_type_ = TOKENFORMATERROR;
        return false;
    }

    std::string witness_txid = std::string(program_witness_.back().begin(),program_witness_.back().end());
    program_witness_.pop_back();
    int witness_vout = CScriptNum(program_witness_.back(),true).getint();

    if ( !WitnessTxidValid(witness_txid,witness_vout) )
    {
        token_error_type_ = TOKENWITNESSERROR;
        return false;
    }

    return true;

}

bool ContractInterpeter::TokenInputInterpreter()
{

    if ( token_tx_type_ == TOKEN_ISSUE || token_tx_type_ == TOKEN_TRANSACTION )
    {
        token_error_type_ = TOKENFORMATERROR;
        return false;
    }

    std::string token_input_txid = std::string(program_input_.back().begin(),program_input_.back().end());
    program_input_.pop_back();
    int token_input_vout = CScriptNum(program_input_.back(),true).getint();

    if ( !TokenInputValid(token_input_txid,token_input_vout) )
    {
        token_error_type_ = TOKENWITNESSERROR;
        return false;
    }

    return true;

}

bool ContractInterpeter::TokenOutputInterpreter()
{
    if ( token_tx_type_ == TOKEN_ISSUE && token_tx_type_ == TOKEN_TRANSACTION )
    {
        token_error_type_ = TOKENFORMATERROR;
        return false;
    }


    return true;
}

bool ContractInterpeter::TokenScriptVerify(const CScript &token_script, bool run)
{
    bool ret = true;

    CScript::const_iterator pc = token_script.begin();
    CScript::const_iterator pend = token_script.end();
//    unsigned int flag_size_ = 0;
//    programType program_flag_;
//    unsigned int witness_size = 0;
//    programType program_witness;
//    unsigned int input_size = 0;
//    programType program_input;
//    unsigned int output_size = 0;
//    programType program_output;
    ReleaseProgram();

    opcodetype opcode;
    while ( pc < pend )
    {
        valuetype vchPushValue;
        if ( token_script.GetOp(pc, opcode, vchPushValue) )
        {
            if ( program_flag_.size() == 0 )
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
            if ( flag_size_ != program_flag_.size() )
            {
                if ( flag_size_ == 0)
                {
                    ret = PushValueToProgram(program_flag_,vchPushValue,flag_size_,false);
                }
                else
                {
                    ret = PushValueToProgram(program_flag_,vchPushValue,flag_size_,true);
                }

                if ( !ret )
                {
                    token_error_type_ = TOKENFLAGSIZEERROR;
                    break;
                }
            }
            //  the value  of witness'program  push
            else if ( witness_size_ != program_witness_.size() )
            {
                if ( witness_size_ == 0)
                {
                    ret = PushValueToProgram(program_witness_,vchPushValue,witness_size_,false);
                }
                else
                {
                    ret = PushValueToProgram(program_witness_,vchPushValue,witness_size_,true);
                }

                if ( !ret )
                {
                    token_error_type_ = TOKENWITNESSSIZEEROR;
                    break;
                }
            }
            //  the value  of input'program  push
            else if ( input_size_ != program_input_.size() )
            {
                if ( input_size_ == 0)
                {
                    ret = PushValueToProgram(program_input_,vchPushValue,input_size_,false);
                }
                else
                {
                    ret = PushValueToProgram(program_input_,vchPushValue,input_size_,true);
                }

                if ( !ret )
                {
                    token_error_type_ = TOKENINPUTSIZEERROR;
                    break;
                }
            }
            //  the value  of output'program  push
            else if ( output_size_ != program_output_.size() )
            {
                if ( output_size_ == 0)
                {
                    ret = PushValueToProgram(program_output_,vchPushValue,output_size_,false);
                }
                else
                {
                    ret = PushValueToProgram(program_output_,vchPushValue,output_size_,true);
                }

                if ( !ret )
                {
                    token_error_type_ = TOKENOUTPUTSIZEERROR;
                    break;
                }
            }
        }
    }

    if ( run )
    {
        if ( !TokenFlagInterpreter() )
        {
            return false;
        }

        if ( !TokenWitnessInterpreter() )
        {
            return false;
        }

    }

    return ret;

}



int ContractInterpeter::Get_token_error_type()
{
    return token_error_type_;
}

int ContractInterpeter::Get_token_tx_type()
{
    return token_tx_type_;
}

bool ContractInterpeter::Get_token_name(std::string&token_name)
{
    unsigned int size = (program_output_.size() + 1 ) / 4 ;
    if ((program_output_.size()+1) % 4)
    {
        return false;
    }
    token_name = "";
    for (unsigned int i = 0; i < size; i++)
    {
        unsigned int pos = 2 + i*4;
        std::string token_vout_name = std::string(program_output_.at(pos).begin(),program_output_.at(pos).end());
        if (token_name.empty() || token_name == "")
        {
            token_name = token_vout_name;
        }
        else if ( token_name  != token_vout_name )
        {
             return false;
        }
    }
    return true;
}

bool ContractInterpeter::Get_token_amount( uint64_t&amount)
{
    unsigned int size = (program_output_.size() + 1) / 4;

    if ((program_output_.size()+1) % 4)
    {
        return false;
    }
    amount = 0;
    for (unsigned int i = 0; i < size; i++)
    {
        unsigned int pos = 3 + i*4;

        uint64_t amount_vout = CScriptNum(program_output_.at(pos),true).getint64();
        amount += amount_vout;
    }

    return true;
}

void ContractInterpeter::ReleaseProgram()
{
    //program_flag_
    flag_size_ = 0;
    program_flag_.clear();

    // program_witness_;
    witness_size_ = 0;
    program_witness_.clear();

    //program_input_;
    input_size_ = 0;
    program_input_.clear();

    //program_output_;
    output_size_ = 0;
    program_output_.clear();
}
