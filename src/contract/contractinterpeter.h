#ifndef CONTRACTINTERPETER_H
#define CONTRACTINTERPETER_H
#include <vector>
#include <stack>

typedef std::vector<unsigned char>  valuetype; //value type
typedef std::stack<valuetype> programType;//program type
class CScript;

class ContractInterpeter
{
public:
    ContractInterpeter();
    ~ContractInterpeter();

public:
    bool TokenScriptVerify(const CScript &token_script);
    int Get_token_error_type();

protected:
    bool PushValueToProgram(programType& program,const valuetype&push_value,unsigned int&size,bool has_set_size);

    bool TokenFlagInterpreter(programType& program_flag, unsigned int flag_size);

    bool TokenWitnessInterpreter(programType& program_witness, unsigned int witness_size);

    bool TokenInputInterpreter(programType& program_input, unsigned int input_size);

    bool TokenOutputInterpreter(programType& program_output,unsigned int output_size);

private:
    int token_error_type_;
    int token_tx_type_;

};

#endif // CONTRACTINTERPETER_H
