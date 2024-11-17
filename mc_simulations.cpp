#include <stdio.h>
#include <vector>
#include <math.h>
#include <concepts>
#include <functional>
#define M_PI 3.14159265358979323846 /* pi */

// class normal_dist
// {
//     std::vector<double> hyperparameters;

//     normal_dist(double mu, double sigma)
//     {
//         hyperparameters.push_back(mu);
//         hyperparameters.push_back(sigma);
//     }

//     double density(double input)
//     {
//         return 1 / sqrt(2 * M_PI * hyperparameters[1]) * exp((-1 / (2 * hyperparameters[1])) * pow(input - hyperparameters[0], 2));
//     }

//     double sample(double input)
//     {
//     }
// };

template <typename T>
concept DistributionType = requires(T dist, double x) {
    { dist.sample() } -> std::same_as<double>;
    { dist.density(x) } -> std::same_as<double>;
};

template <DistributionType Distribution>
class mc_simulator
{
    double rejection_sampler(const std::function<double(double)> &target_density, Distribution &dist)
    {
        return 1.0;
    }
};

int main()
{
    int x = 3;
}