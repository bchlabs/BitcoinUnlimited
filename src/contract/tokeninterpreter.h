#ifndef CONTRACT_TOKENINTERPRETER_H
#define CONTRACT_TOKENINIERPRETER_H

#include <univalue.h>
class CScript;

bool VerifyTokenScript(const CScript &token_script);

#endif
