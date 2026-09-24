#pragma once

#include <string>

// ============================================================================
//  Payoff hierarchy — the terminal cashflow of a European-style contract
//
//  Design notes:
//    * Payoff is ABSTRACT: it declares what every payoff must do, and
//      implements none of it. You cannot instantiate a Payoff.
//    * operator() makes each payoff a FUNCTION OBJECT: you write p(spot),
//      not p.evaluate(spot). This is the idiom used by QuantLib and by
//      Joshi's book, and it lets a payoff be passed anywhere a callable
//      is expected.
//    * The destructor is VIRTUAL. Read the comment below — this is the
//      single most common C++ bug in class hierarchies, and a guaranteed
//      interview question.
// ============================================================================

namespace quant {

class Payoff {
public:

    virtual ~Payoff() = default;
    virtual double operator()(double spot) const = 0;
    
    virtual std::string name() const = 0;
};

// ---------------------------------------------------------------------------
//  Vanilla call:  max(S - K, 0)
// ---------------------------------------------------------------------------

class CallPayoff : public Payoff {
public:
    
    explicit CallPayoff(double strike);

    double operator()(double spot) const override;
    std::string name() const override;

    double strike() const { return strike_; }

private:
    double strike_;
};

// ---------------------------------------------------------------------------
//  Vanilla put:  max(K - S, 0)
// ---------------------------------------------------------------------------

class PutPayoff : public Payoff {
public:
    explicit PutPayoff(double strike);

    double operator()(double spot) const override;
    std::string name() const override;

    double strike() const { return strike_; }

private:
    double strike_;
};

class DigitalCallPayoff : public Payoff {
public:
    explicit DigitalCallPayoff(double strike);

    double operator()(double spot) const override;
    std::string name() const override;

    double strike() const { return strike_; }

private:
    double strike_;
};


class CallSpreadPayoff : public Payoff {
public:
    CallSpreadPayoff(double strikeLow, double strikeHigh);

    double operator()(double spot) const override;
    std::string name() const override;

    double strikeLow()  const { return low_;  }
    double strikeHigh() const { return high_; }

private:
    double low_;
    double high_;
};

}  // namespace quant
