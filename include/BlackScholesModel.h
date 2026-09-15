#pragma once

// ============================================================================
//  BlackScholesModel — analytic pricing of European vanilla options
//
//  Design notes (this is the point of the exercise):
//    * The model owns the MARKET parameters (S0, r, q, sigma).
//      The CONTRACT parameters (K, T) are passed per call.
//      Rationale: one model, many options priced against it.
//    * All pricing methods are const — they never mutate the model.
//    * The constructor validates its inputs and throws on nonsense.
//    * d1 / d2 are private helpers, so the formulas live in exactly one place.
// ============================================================================

namespace quant {

// ---------------------------------------------------------------------------
//  Standard normal distribution helpers
// ---------------------------------------------------------------------------

/// Cumulative distribution function of the standard normal law.
double normalCDF(double x);

/// Probability density function of the standard normal law.
double normalPDF(double x);

// ---------------------------------------------------------------------------
//  Option type
// ---------------------------------------------------------------------------

// enum class (not a plain enum, not a char): the compiler refuses to mix it up
// with an int, so OptionType::Call can never be silently confused with 1.
enum class OptionType { Call, Put };

// ---------------------------------------------------------------------------
//  The model
// ---------------------------------------------------------------------------

class BlackScholesModel {
public:
    /// @param S0     spot price of the underlying, must be > 0
    /// @param r      continuously compounded risk-free rate
    /// @param q      continuous dividend yield (0 if none)
    /// @param sigma  volatility, must be > 0
    /// @throws std::invalid_argument if S0 <= 0 or sigma <= 0
    BlackScholesModel(double S0, double r, double q, double sigma);

    // --- Accessors (const-correct, no setters: the model is immutable) ---
    double spot()       const { return S0_;    }
    double rate()       const { return r_;     }
    double dividend()   const { return q_;     }
    double volatility() const { return sigma_; }

    // --- Pricing ---
    /// Price of a European vanilla option.
    /// @param K     strike, must be > 0
    /// @param T     time to maturity in years, must be > 0
    /// @param type  Call or Put
    double price(double K, double T, OptionType type) const;

    // --- Greeks ---
    // Delta and Rho depend on the option type; Gamma and Vega do not
    // (a call and a put with the same strike share them — think about why).
    double delta(double K, double T, OptionType type) const;
    double gamma(double K, double T) const;
    double vega (double K, double T) const;
    double theta(double K, double T, OptionType type) const;
    double rho  (double K, double T, OptionType type) const;

private:
    double S0_;
    double r_;
    double q_;
    double sigma_;

    // Trailing underscore on members: a common convention that makes it
    // obvious at a glance whether a name is a member or a local.

    double d1(double K, double T) const;
    double d2(double K, double T) const;

    /// Shared validation for the contract parameters.
    static void checkContract(double K, double T);
};

}  // namespace quant
