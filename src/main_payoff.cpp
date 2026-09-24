#include "Payoff.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace quant;

namespace {

int failures = 0;

void check(const std::string& label, double got, double expected, double tol = 1e-9) {
    const bool ok = std::fabs(got - expected) < tol;
    if (!ok) ++failures;
    std::cout << (ok ? "  [ OK ] " : "  [FAIL] ")
              << std::left << std::setw(34) << label
              << std::right << std::fixed << std::setprecision(4)
              << std::setw(10) << got
              << "   expected " << std::setw(10) << expected << "\n";
}

void checkName(const std::string& label, const std::string& got,
               const std::string& expectedPrefix) {
    const bool ok = got.rfind(expectedPrefix, 0) == 0;   // starts_with
    if (!ok) ++failures;
    std::cout << (ok ? "  [ OK ] " : "  [FAIL] ")
              << std::left << std::setw(34) << label
              << got << (ok ? "" : "   expected to start with " + expectedPrefix)
              << "\n";
}

}  // namespace

int main() {
    std::cout << "\n=== Payoff values ===\n";

    CallPayoff        call(100.0);
    PutPayoff         put(100.0);
    DigitalCallPayoff digital(100.0);
    CallSpreadPayoff  spread(95.0, 105.0);

    std::cout << "\n--- Call(100) ---\n";
    check("S = 90",  call(90.0),   0.0);
    check("S = 100", call(100.0),  0.0);
    check("S = 110", call(110.0), 10.0);

    std::cout << "\n--- Put(100) ---\n";
    check("S = 90",  put(90.0),  10.0);
    check("S = 100", put(100.0),  0.0);
    check("S = 110", put(110.0),  0.0);

    std::cout << "\n--- Digital call(100) ---\n";
    check("S = 90",  digital(90.0),  0.0);
    check("S = 100", digital(100.0), 0.0);   // strict inequality: S > K
    check("S = 110", digital(110.0), 1.0);

    std::cout << "\n--- Call spread(95, 105) ---\n";
    check("S = 90",  spread(90.0),   0.0);
    check("S = 100", spread(100.0),  5.0);
    check("S = 110", spread(110.0), 10.0);

    std::cout << "\n--- Names ---\n";
    checkName("Call",        call.name(),    "Call(K=100");
    checkName("Put",         put.name(),     "Put(K=100");
    checkName("Digital",     digital.name(), "Digital(K=100");
    checkName("Call spread", spread.name(),  "CallSpread(95");

    // =====================================================================
    //  Polymorphism: one loop, four different contracts.
    //  The loop body knows nothing about which payoff it is holding.
    //  Adding a fifth product would require zero change here.
    // =====================================================================
    std::cout << "\n=== Polymorphic evaluation at S = 107 ===\n";

    std::vector<std::unique_ptr<Payoff>> book;
    book.push_back(std::make_unique<CallPayoff>(100.0));
    book.push_back(std::make_unique<PutPayoff>(100.0));
    book.push_back(std::make_unique<DigitalCallPayoff>(100.0));
    book.push_back(std::make_unique<CallSpreadPayoff>(95.0, 105.0));

    const double spot = 107.0;
    double total = 0.0;
    for (const auto& p : book) {
        const double v = (*p)(spot);           // virtual dispatch happens here
        total += v;
        std::cout << "  " << std::left << std::setw(34) << p->name()
                  << std::right << std::fixed << std::setprecision(4)
                  << std::setw(10) << v << "\n";
    }
    std::cout << "  " << std::left << std::setw(34) << "TOTAL"
              << std::right << std::setw(10) << total << "\n";
    check("Book total at S = 107", total, 7.0 + 0.0 + 1.0 + 10.0);

    // =====================================================================
    //  Digital replication by a call spread.
    //
    //  A digital paying 1 above K is the limit of
    //          [ Call(K-h) - Call(K+h) ] / (2h)
    //  as h goes to 0. This is how a desk hedges a digital: you cannot
    //  delta-hedge a discontinuity, so you overhedge with a tight spread.
    // =====================================================================
    std::cout << "\n=== Digital replicated by a call spread ===\n";
    std::cout << "  ratio = [ Call(K-h) - Call(K+h) ] / (2h),  K = 100\n\n";

    const double K = 100.0;
    const double widths[] = {5.0, 1.0, 0.1, 0.01};
    const double spots[]  = {95.0, 100.0, 102.0, 110.0};

    std::cout << "        h ";
    for (double s : spots)
        std::cout << " |  S=" << std::setw(6) << std::setprecision(2) << s;
    std::cout << "\n  ---------";
    for (std::size_t i = 0; i < sizeof(spots) / sizeof(spots[0]); ++i)
        std::cout << "-+---------";
    std::cout << "\n";

    for (double h : widths) {
        CallSpreadPayoff cs(K - h, K + h);
        std::cout << "  " << std::setw(7) << std::setprecision(2) << h << " ";
        for (double s : spots)
            std::cout << " | " << std::setw(8) << std::setprecision(4)
                      << cs(s) / (2.0 * h);
        std::cout << "\n";
    }

    std::cout << "  ---------";
    for (std::size_t i = 0; i < sizeof(spots) / sizeof(spots[0]); ++i)
        std::cout << "-+---------";
    std::cout << "\n  digital ";
    for (double s : spots)
        std::cout << " | " << std::setw(8) << std::setprecision(4) << digital(s);
    std::cout << "\n";

    std::cout << "\n  Note the column S = K: the spread converges to 0.5, not to 0.\n"
                 "  At the discontinuity the replication splits the jump in half.\n";

    std::cout << "\n"
              << (failures == 0 ? "All checks passed.\n"
                                : std::to_string(failures) + " check(s) failed.\n")
              << "\n";
    return failures == 0 ? 0 : 1;
}
