#include "contractio.h"
#include <sstream>

contractio::contractio()
{

}

std::string contractio::uint64ToString(uint64_t amount)
{
    std::stringstream stream;
    stream << amount;
    return stream.str();
}

std::string contractio::intToString(int value)
{
    std::stringstream stream;
    stream << value;
    return stream.str();
}
