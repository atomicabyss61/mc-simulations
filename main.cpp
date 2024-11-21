#include "mc_simulations.hpp"
#include <fstream>

class Unif_Dist
{
public:
    double lower = 0, upper = 1;
    Unif_Dist(double _lower, double _upper) : lower(_lower), upper(_upper)
    {
    }

    double sample(double x)
    {
        return (x * (upper - lower)) + lower;
    }

    double density(double x)
    {
        return 1 / (upper - lower);
    }
};

int main()
{
    Unif_Dist unif(0, 4);
    auto un_normalised_trig_density = [](double x)
    { return std::sin(x); };

    MC_simulations<Unif_Dist> simulator;
    std::vector<double> res = simulator.rejection_sampler(un_normalised_trig_density, unif, 30, 100000);

    std::cout << "Completed function: " << res.size() << '\n';
    std::ofstream file("data.py");
    file << "data = [\n";
    for (double x : res)
    {
        file << '\t' << x << ",\n";
    }
    file << "]";
}