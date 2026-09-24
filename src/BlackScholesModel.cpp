#include "BlackScholesModel.h"

#include <cmath>
#include <stdexcept>

namespace quant {

double normalCDF(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

double normalPDF(double x) {
    static const double invSqrt2Pi = 0.3989422804014327;
    return invSqrt2Pi * std::exp(-0.5 * x * x);
}

BlackScholesModel::BlackScholesModel(double S0, double r, double q, double sigma)
    : S0_(S0), r_(r), q_(q), sigma_(sigma)
{
    if (S0 <= 0.0)
        throw std::invalid_argument("BlackScholesModel: spot must be > 0");
    if (sigma <= 0.0)
        throw std::invalid_argument("BlackScholesModel: volatility must be > 0");
}

void BlackScholesModel::checkContract(double K, double T) {
    if (K <= 0.0)
        throw std::invalid_argument("BlackScholesModel: strike must be > 0");
    if (T <= 0.0)
        throw std::invalid_argument("BlackScholesModel: maturity must be > 0");
}


double BlackScholesModel::d1(double K, double T) const {
    return (std::log(S0_ / K) + (r_ - q_ + 0.5 * sigma_ * sigma_) * T)
           / (sigma_ * std::sqrt(T));
}

double BlackScholesModel::d2(double K, double T) const {
    return d1(K, T) - sigma_ * std::sqrt(T);
}


double BlackScholesModel::price(double K, double T, OptionType type) const {
    checkContract(K, T);

    const double D1 = d1(K, T);
    const double D2 = d2(K, T);
    const double dfR = std::exp(-r_ * T);
    const double dfQ = std::exp(-q_ * T);

    if (type == OptionType::Call) {
        //  C = S*e^{-qT} N(d1)  -  K*e^{-rT} N(d2)
        return S0_ * dfQ * normalCDF(D1) - K * dfR * normalCDF(D2);
    } else {
        //  P = K*e^{-rT} N(-d2)  -  S*e^{-qT} N(-d1)
        return K * dfR * normalCDF(-D2) - S0_ * dfQ * normalCDF(-D1);
    }
}

double BlackScholesModel::delta(double K, double T, OptionType type) const {
    checkContract(K, T);

    const double D1  = d1(K, T);
    const double dfQ = std::exp(-q_ * T);

    if (type == OptionType::Call)
        return dfQ * normalCDF(D1);
    else
        return dfQ * (normalCDF(D1) - 1.0);
}

// ===========================================================================
//  Greeks
//
//  Convention: every Greek returns the RAW partial derivative, with no
//  scaling (no /100 on vega, no /365 on theta, no /10000 on rho).
//  Scaling belongs to the presentation layer: desks disagree on it, and
//  finite-difference validation produces the raw derivative anyway.
// ===========================================================================

//          e^{-qT} n(d1)
//  Gamma = ---------------          n(.) = normalPDF
//          S sigma sqrt(T)
//
//  No OptionType: differentiating put-call parity twice in S shows that a
//  call and a put on the same strike share the same gamma.
double BlackScholesModel::gamma(double K, double T) const {
    checkContract(K, T);
    const double D1  = d1(K, T);
    const double dfQ = std::exp(-q_ * T);
    return dfQ * normalPDF(D1) / (S0_ * sigma_ * std::sqrt(T));
}

//  Vega = S e^{-qT} n(d1) sqrt(T)
//
//  Per unit of volatility (sigma += 1.00). Divide by 100 for a 1-vol-point
//  move. Shared by call and put, for the same reason as gamma.
double BlackScholesModel::vega(double K, double T) const {
    checkContract(K, T);
    const double D1  = d1(K, T);
    const double dfQ = std::exp(-q_ * T);
    return (S0_ * dfQ * normalPDF(D1) * std::sqrt(T));
}

//                 S e^{-qT} n(d1) sigma
//  Theta_call = - ---------------------  - r K e^{-rT} N(d2)  + q S e^{-qT} N(d1)
//                      2 sqrt(T)
//
//                 S e^{-qT} n(d1) sigma
//  Theta_put  = - ---------------------  + r K e^{-rT} N(-d2) - q S e^{-qT} N(-d1)
//                      2 sqrt(T)
//
//  Per YEAR. Divide by 365 (or 252) for a per-day figure.
//  The first term is common to call and put and always negative: time decay
//  of optionality. The remaining terms are the carry.
double BlackScholesModel::theta(double K, double T, OptionType type) const {
    checkContract(K, T);
    const double D1  = d1(K, T);
    const double D2  = d2(K, T);
    const double dfR = std::exp(-r_ * T);
    const double dfQ = std::exp(-q_ * T);
    const double common = -(S0_ * dfQ * normalPDF(D1) * sigma_) / (2.0 * std::sqrt(T));
    if(type == OptionType::Call){
        return common - r_ * K * dfR * normalCDF(D2) + q_ * S0_ * dfQ * normalCDF(D1);
    } else {
        return common + r_ * K * dfR * normalCDF(-D2) - q_ * S0_ * dfQ * normalCDF(-D1);
    }
}

//  Rho_call =   K T e^{-rT} N(d2)
//  Rho_put  = - K T e^{-rT} N(-d2)
//
//  Per unit of rate (r += 100%). Divide by 10000 for a 1bp move.
double BlackScholesModel::rho(double K, double T, OptionType type) const {
    checkContract(K, T);
    const double D2  = d2(K, T);
    const double dfR = std::exp(-r_ * T);
    if(type == OptionType::Call){
        return K * T * dfR * normalCDF(D2);
    } else {
        return -K * T * dfR * normalCDF(-D2);
    }
}

}  // namespace quant
