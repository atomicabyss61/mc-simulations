#ifndef MC_SIMULATIONS_HPP
#define MC_SIMULATIONS_HPP

#include <iostream>
#include <vector>
#include <queue>
#include <math.h>
#include <concepts>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <random>
#include <thread>

template <typename T>
concept DistributionType = requires(T dist, double x) {
    { dist.sample(x) } -> std::same_as<double>;
    { dist.density(x) } -> std::same_as<double>;
};

template <DistributionType Distribution>
class MC_simulations
{
public:
    MC_simulations(double seed)
    {
        gen = std::mt19937(seed);
        dis = std::uniform_real_distribution<>(0.0, 1.0);
    }

    MC_simulations()
    {
        MC_simulations(0.0);
    }

    std::vector<double> &rejection_sampler(const std::function<double(double)> &target_density, Distribution &dist, int k, int num_samples)
    {
        done_sampling = false;
        if (!sample_queue.empty())
        {
            size_t left = sample_queue.size();
            for (int i = 0; i < left; i++)
            {
                sample_queue.pop();
            }
        }

        if (!result_queue.empty())
        {
            size_t left = result_queue.size();
            for (int i = 0; i < left; i++)
            {
                result_queue.pop();
            }
        }
        std::vector<double> *accepted_samples = new std::vector<double>();
        accepted_samples->reserve(num_samples);

        std::jthread sampler([this, &dist]()
                             { sampler_thread(dist); });

        std::jthread computer([this, &dist, target_density, k]()
                              { compute_thread(dist, target_density, k); });

        std::jthread accepter([this, &accepted_samples, num_samples]()
                              { accept_thread(*accepted_samples, num_samples); });

        return *accepted_samples;
    }

private:
    std::mutex mutex_1, mutex_2;
    std::condition_variable cv1, cv2;
    std::queue<double> sample_queue;
    std::queue<std::pair<double, double>> result_queue;
    std::atomic<bool> done_sampling{false};
    size_t SAMPLES_PER_EXEC = 100;

    std::mt19937 gen;
    std::uniform_real_distribution<> dis;

    void sampler_thread(Distribution &dist)
    {

        while (!done_sampling)
        {
            // sample from known distribution
            std::vector<double> samples;
            samples.reserve(SAMPLES_PER_EXEC);

            for (int i = 0; i < SAMPLES_PER_EXEC; i++)
            {
                samples.push_back(dist.sample(dis(gen)));
            }

            {

                // acquire lock to add all generates samples to queue
                std::unique_lock<std::mutex> lock(mutex_1);
                cv1.wait(lock, [&]
                         { return sample_queue.size() <= SAMPLES_PER_EXEC || done_sampling; });

                for (auto it = samples.begin(); it != samples.end(); it++)
                {
                    sample_queue.push((*it));
                }
                cv1.notify_one();
            }
        }
    }
    void compute_thread(Distribution &dist, const std::function<double(double)> &target_density, int k)
    {
        // acquire condition variable first

        while (!done_sampling)
        {
            // make sure this doesnt reset capacity
            std::vector<double> incoming_samples;
            incoming_samples.reserve(SAMPLES_PER_EXEC);
            {

                std::unique_lock<std::mutex> lock(mutex_1);
                cv1.wait(lock, [&]
                         { return !sample_queue.empty() || done_sampling; });

                if (done_sampling)
                {
                    return;
                }

                for (int i = 0; i < SAMPLES_PER_EXEC; i++)
                {
                    double sample = sample_queue.front();
                    sample_queue.pop();
                    incoming_samples.push_back(sample);
                }
                cv1.notify_one();
            }

            std::vector<std::pair<double, double>> computed_samples;
            computed_samples.reserve(SAMPLES_PER_EXEC);
            // compute the acceptance ratio in place
            for (int i = 0; i < SAMPLES_PER_EXEC; i++)
            {
                double x = incoming_samples[i];
                computed_samples.push_back({x, target_density(x) / (k * dist.density(x))});
            }

            {

                std::lock_guard<std::mutex> lock(mutex_2);

                for (auto it = computed_samples.begin(); it != computed_samples.end(); it++)
                {
                    result_queue.push(std::make_pair((*it).first, (*it).second));
                }
                cv2.notify_one();
            }
        }
    }
    void accept_thread(std::vector<double> &accepted_samples, int num_samples)
    {

        while (!done_sampling)
        {
            std::vector<std::pair<double, double>> incoming_samples;
            incoming_samples.reserve(SAMPLES_PER_EXEC);
            {

                // acquire condition variable
                std::unique_lock<std::mutex> lock(mutex_2);
                cv2.wait(lock, [&]
                         { return !result_queue.empty(); });

                for (int i = 0; i < SAMPLES_PER_EXEC; i++)
                {
                    std::pair<double, double> sample = result_queue.front();
                    result_queue.pop();
                    incoming_samples.push_back(sample);
                }
                cv2.notify_one();
            }

            for (auto it = incoming_samples.begin(); it != incoming_samples.end() && accepted_samples.size() != num_samples; ++it)
            {
                double acceptance_rate = (*it).second, sample = (*it).first, u = dis(gen);
                if (u <= acceptance_rate)
                {
                    accepted_samples.push_back(sample);
                }
            }

            if (accepted_samples.size() == num_samples)
            {
                done_sampling = true;
            }
        }
    }
};

#endif