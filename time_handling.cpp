#include <cmath>
#include <cstdint>
#include <ctime>
#include <cstring>
#include <string>
#include "time_handling.h"

constexpr std::time_t EPOCHDIFF_MAC_UNIX = 2082844800;
constexpr double JanFirst1990MACTime = 1580970496.0; //1580947200.0; // better value?
constexpr auto HIGH_DWORD = 4294967296.0;

std::time_t PMtime2time_t(double t)
{
    t -= JanFirst1990MACTime;
    if (t < 0.0) {
        t += HIGH_DWORD; // why is this necessary?
    }
    return std::time_t(std::floor(t)) - EPOCHDIFF_MAC_UNIX;
}

std::string formatPMtimeDate(double t)
{
    auto unixtime = PMtime2time_t(t);
    char buffer[128];
    std::strftime(buffer, 128, "%F", gmtime(&unixtime));
    return std::string(buffer);
}

std::string formatPMtimeUTC(double t)
{
    auto unixtime = PMtime2time_t(t);
    char buffer[128];
    std::strftime(buffer, 128, "%FT%T UTC", gmtime(&unixtime));
    return std::string(buffer);
}
