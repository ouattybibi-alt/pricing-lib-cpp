#include "BlackScholesModel.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

using namespace quant;

namespace {

int failures = 0;

void check(const std::string& label, double got, double expected, double tol = 1e-4) {
    const bool ok = std::fabs(got - expected) < tol;
    if (!ok) ++failures;
    std::cout << (ok ? "  [ OK ] " : "  [FAIL] ")
              << std::left << std::setw(22) << label
              << std::right << std::fixed << std::setprecision(6)
              << std::setw(12) << got
              << "   expected " << std::setw(12) << expected
              << "\n";
}

}  // namespace

int main() {
    // ---- Reference case -----------------------------------------------
    //  S0 = 100, K = 100, r = 5%, q = 0, sigma = 20%, T = 1 year
    //  At-the-money forward-ish, the textbook benchmark.
    const double S0 = 100.0, K = 100.0, r = 0.05, q = 0.0, sigma = 0.20, T = 1.0;

    BlackScholesModel model(S0, r, q, sigma);

    std::cout << "\n=== Black-Scholes reference case ===\n"
              << "  S0 = " << S0 << "  K = " << K << "  r = " << r
              << "  q = " << q << "  sigma = " << sigma << "  T = " << T << "\n\n";

    std::cout << "--- Prices ---\n";
    const double call = model.price(K, T, OptionType::Call);
    const double put  = model.price(K, T, OptionType::Put);
    check("Call", call, 10.450584);
    check("Put",  put,   5.573526);

    // Put-call parity:  C - P = S e^{-qT} - K e^{-rT}
    // This is a model-free identity: if it breaks, the code is wrong,
    // not the model. Always the first sanity check to write.
    std::cout << "\n--- Put-call parity ---\n";
    const double parity = S0 * std::exp(-q * T) - K * std::exp(-r * T);
    check("C - P", call - put, parity);

    std::cout << "\n--- Greeks ---\n";
    check("Delta call", model.delta(K, T, OptionType::Call),  0.636831);
    check("Delta put",  model.delta(K, T, OptionType::Put),  -0.363169);
    check("Gamma",      model.gamma(K, T),                    0.018762);
    check("Vega",       model.vega (K, T),                   37.524035, 1e-3);
    check("Theta call", model.theta(K, T, OptionType::Call), -6.414028, 1e-3);
    check("Theta put",  model.theta(K, T, OptionType::Put),  -1.657880, 1e-3);
    check("Rho call",   model.rho  (K, T, OptionType::Call), 53.232482, 1e-3);
    check("Rho put",    model.rho  (K, T, OptionType::Put), -41.890461, 1e-3);

    // ---- Validation: the constructor must reject nonsense ---------------
    std::cout << "\n--- Input validation ---\n";
    try {
        BlackScholesModel bad(-100.0, r, q, sigma);
        std::cout << "  [FAIL] negative spot was accepted\n";
        ++failures;
    } catch (const std::invalid_argument&) {
        std::cout << "  [ OK ] negative spot rejected\n";
    }
    try {
        model.price(K, -1.0, OptionType::Call);
        std::cout << "  [FAIL] negative maturity was accepted\n";
        ++failures;
    } catch (const std::invalid_argument&) {
        std::cout << "  [ OK ] negative maturity rejected\n";
    }

    std::cout << "\n"
              << (failures == 0 ? "All checks passed.\n"
                                : std::to_string(failures) + " check(s) failed.\n")
              << "\n";
    return failures == 0 ? 0 : 1;
}
