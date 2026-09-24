#include "Payoff.h"

#include <algorithm>
#include <stdexcept>

namespace quant {

// ===========================================================================
//  CallPayoff  —  WORKED EXAMPLE
// ===========================================================================

CallPayoff::CallPayoff(double strike)
    : strike_(strike)
{
    if (strike <= 0.0)
        throw std::invalid_argument("CallPayoff: strike must be > 0");
}

double CallPayoff::operator()(double spot) const {
    return std::max(spot - strike_, 0.0);
}

std::string CallPayoff::name() const {
    return "Call(K=" + std::to_string(strike_) + ")";
}

// ===========================================================================
//  PutPayoff  —  SECOND WORKED EXAMPLE
// ===========================================================================

PutPayoff::PutPayoff(double strike)
    : strike_(strike)
{
    if (strike <= 0.0)
        throw std::invalid_argument("PutPayoff: strike must be > 0");
}

double PutPayoff::operator()(double spot) const {
    return std::max(strike_ - spot, 0.0);
}

std::string PutPayoff::name() const {
    return "Put(K=" + std::to_string(strike_) + ")";
}

// ===========================================================================
//  TO IMPLEMENT — DigitalCallPayoff
//
//      operator() : return 1.0 if spot > strike_, else 0.0
//      name()     : return "Digital(K=" + std::to_string(strike_) + ")"
//      ctor       : validate strike > 0 like the two above
// ===========================================================================

DigitalCallPayoff::DigitalCallPayoff(double strike)
    : strike_(strike)
{
    if (strike <= 0.0)
        throw std::invalid_argument("DigitalCallPayoff: strike must be > 0");

}

double DigitalCallPayoff::operator()(double spot) const {
    return spot > strike_ ? 1.0 : 0.0;
}

std::string DigitalCallPayoff::name() const {
    return "Digital(K=" + std::to_string(strike_) + ")";
}


CallSpreadPayoff::CallSpreadPayoff(double strikeLow, double strikeHigh)
    : low_(strikeLow), high_(strikeHigh)
{
    if (strikeLow <= 0.0)
        throw std::invalid_argument("CallSpreadPayoff: strikeLow must be > 0");
    if (strikeHigh <= strikeLow)
        throw std::invalid_argument("CallSpreadPayoff: strikeHigh must be > strikeLow");
}

double CallSpreadPayoff::operator()(double spot) const {
    return std::max(spot - low_, 0.0) - std::max(spot - high_, 0.0);
}

std::string CallSpreadPayoff::name() const {
    return "CallSpread(" + std::to_string(low_) + "," + std::to_string(high_) + ")";
}

}
