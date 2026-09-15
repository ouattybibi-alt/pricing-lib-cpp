#include <iostream>
#include <cmath>

// Fonction de repartition de la loi normale centree reduite
double normalCDF(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

// Prix d'un call europeen sous Black-Scholes
double blackScholesCall(double S, double K, double r, double sigma, double T) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    return S * normalCDF(d1) - K * std::exp(-r * T) * normalCDF(d2);
}

int main() {
    double S = 100.0;      // spot
    double K = 100.0;      // strike
    double r = 0.05;       // taux sans risque
    double sigma = 0.20;   // volatilite
    double T = 1.0;        // maturite en annees

    double prix = blackScholesCall(S, K, r, sigma, T);

    std::cout << "Prix du call europeen : " << prix << std::endl;
    return 0;
}