#ifndef CONTRACTINTERPETER_H
#define CONTRACTINTERPETER_H
#include <vector>
#include <stack>
#include <string>

typedef std::vector<unsigned char>  valuetype; //value type
typedef std::vector<valuetype> programType;//program type
class CScript;

class ContractInterpeter
{
public:
    ContractInterpeter();
    ~ContractInterpeter();

public:
    bool TokenScriptVerify(const CScript &token_script,bool run = true);

    int Get_token_error_type();

    int Get_token_tx_type();

    bool Get_token_name(std::string &token_name);

    bool Get_token_amount(uint64_t &amount);

protected:
    void ReleaseProgram();

protected:
    bool PushValueToProgram(programType& program,const valuetype&push_value,unsigned int&size,bool has_set_size);

    bool TokenFlagInterpreter();

    bool TokenWitnessInterpreter();

    bool TokenInputInterpreter();

    bool TokenOutputInterpreter();

private:
    int token_error_type_;
    int token_tx_type_;

    unsigned int flag_size_ = 0;
    programType program_flag_;
    unsigned int witness_size_ = 0;
    programType program_witness_;
    unsigned int input_size_ = 0;
    programType program_input_;
    unsigned int output_size_ = 0;
    programType program_output_;


};

#endif // CONTRACTINTERPETER_H
