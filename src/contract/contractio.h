#ifndef CONTRACTIO_H
#define CONTRACTIO_H
#include <string>

//typedef unsigned long long    uint64_t;
class contractio
{
public:
    contractio();

    static std::string  uint64ToString(uint64_t amount);
    static std::string  intToString(int value);
};

#endif // CONTRACTIO_H
