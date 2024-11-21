# Monte Carlo Method Simulations

## Rejection Sampler
```rejection_sampler``` performs multi-threaded sampling from an un-normalised distribution given a known proposal distribution. 
- The function utilises multiple threads to concurrently sample from the proposed distribution, calculate acceptance rate of sample and accept/reject given samples. 
- The proposal distribution uses c++ concepts to enforce type restrictions on the proposal distribution and can be made to be comptaible with Boost.Math and STL random distribution with a simple wrapper shown below.


### Example Usage
#### Custom defined proposal distribution
We can define our own distribution class as shown,
```
// Simple Uniform distribution class
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
```


Then in our main function we can instantiate our custom class and define a disitrbution which is the sine function via a lambda function, note that the support is defined via the range of the inverse-CDF of our proposal distribution. In the example we use the support [0, 3],


```
Unif_Dist unif(0, 3);
auto un_normalised_trig_density = [] (double x) { return std::sin(x); };
MC_simulations<Unif_Dist> simulator;
```

We then run our sampler and pass in the un-normalised distribution, proposal distribution, $k$ and number of iterations. The $k$ value is computed seperately and is equivalent to $\sup_{x\in\mathcal{X}_f} \frac{f(x)}{g(x)}$. However, it is often hard or impossible to calculate the exact value, hence, a reasonable approximation for the supremium over the support is acceptable. Running the function and capturing the result,

```
std::vector<double> res = simulator.rejection_sampler(un_normalised_trig_density, unif, 30, 100000);
```


We can write the contents to a python file to perform analysis of the sampled points as follows,

```
std::ofstream file("data.py");
file << "data = [\n";
for (double x : res) {
    file << '\t' << x << ",\n";
}
file << "]";
```

Now getting the data and plotting a histogram using reasonable parameters,
```
import matplotlib.pyplot as plt
from data import data

plt.hist(data, bins = 50, alpha=0.7)
plt.xlabel('X')
plt.ylabel('Num Samples')
plt.show()
```
![Figure_1](https://github.com/user-attachments/assets/1ca9c12b-0271-4a08-9852-0f45c7e46aef)


#### Math.Boost proposal distribution
