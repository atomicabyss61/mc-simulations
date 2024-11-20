#include <stdio.h>
#include <vector>
#include <queue>
#include <math.h>
#include <concepts>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <random>
#include <iostream>

#define M_PI 3.14159265358979323846 /* pi */

class Unif_Dist
{
public:
    double sample()
    {
        return 1.0;
    }

    double density(double x)
    {
        return x;
    }
};

template <typename T>
concept DistributionType = requires(T dist, double x) {
    { dist.sample() } -> std::same_as<double>;
    { dist.density(x) } -> std::same_as<double>; // might not need because we could sample from unif(0, 1) and pass it into dist.sample(x)
};

template <DistributionType Distribution>
class MC_simulator
{
public:
    double rejection_sampler(const std::function<double(double)> &target_density, Distribution &dist, int k, int num_samples)
    {
        /*
        1. produce sample from dist
        2. calculate value of c = target_density(x) / (k * dist.density(x))
        3. accept if U drawn from Unif(0, 1) <= c
        */
        sampler_thread(dist);
        return 1.0;
    }

private:
    std::mutex mutex_1, mutex_2;
    std::condition_variable cv1, cv2;
    std::queue<double> sample_queue;
    std::queue<std::pair<double, double>> result_queue;
    std::atomic<bool> done_sampling{false};
    size_t SAMPLES_PER_EXEC = 100;

    void sampler_thread(Distribution &dist)
    {
        std::vector<double> samples;
        samples.reserve(SAMPLES_PER_EXEC);

        // sample from known distribution
        std::generate(samples.begin(), samples.end(), [&]()
                      { return dist.sample(); });

        {
            // acquire lock to add all generates samples to queue
            std::lock_guard<std::mutex> lock(mutex_1);
            for (const auto it = samples.begin(); it != samples.end(); it++)
            {
                sample_queue.push(*it);
            }
            cv1.notify_one();
        }
    }

    void compute_thread(Distribution &dist, const std::function<double(double)> &target_density, int k)
    {
        std::vector<double> incoming_samples;
        incoming_samples.reserve(SAMPLES_PER_EXEC);

        {
            std::lock_guard<std::mutex> lock(mutex_1);
            for (int i = 0; i < SAMPLES_PER_EXEC; i++)
            {
                double sample = sample_queue.front();
                sample_queue.pop();
                incoming_samples.push_back(sample);
            }
            cv1.notify_one();
        }

        std::vector<std::pair<double, double>> computed_samples;
        // compute the acceptance ratio in place
        std::transform(incoming_samples.begin(), incoming_samples.end(), computed_samples.begin(), [](double x)
                       { return {x, target_density(x) / (k * dist.density(x))}; });

        {
            std::lock_guard<std::mutex> lock(mutex_2);
            for (const auto it = computed_samples.begin(); it != computed_samples.end(); it++)
            {
                result_queue.push(*it);
            }
        }
    }

    void accept_thread(std::vector<double> &accepted_samples)
    {
    }
};

int main()
{
    Unif_Dist unif;
    const auto un_normalised_trig_density = [](double x)
    { return std::sin(x); };

    // MC_simulator<Unif_Dist> simulator;

    simulator.rejection_sampler(un_normalised_trig_density, unif, 4, 1000);
}