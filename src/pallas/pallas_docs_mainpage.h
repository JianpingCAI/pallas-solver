/*! \mainpage Pallas Index Page
 *
 * \section intro_sec Introduction
 *
 * This project is a suite of global optimization algorithms for C++ inspired by SciPy's global optimization package.
 * Currently supported functions are basinhopping, brute, differential evolution, and simulated annealing.
 *
 * * pallas::Basinhopping
 *
 * * pallas::Brute
 *
 * * pallas::DifferentialEvolution
 *
 * * pallas::SimulatedAnnealing
 *
 * \subsection dependency_sec Dependencies
 *
 * * C++11 compatible compiler
 * * <a href="https://github.com/google/glog">glog</a>
 * * <a href="http://ceres-solver.org/">Ceres</a>
 * * <a href="https://cmake.org">CMake</a>
 *
 * \subsection build_install_sec Building and Installation
 *
 * To use this library first install glog and CMake. Pallas is based off of the Google Ceres project which has extensive
 * use of glog for logging and debugging features, and this functionality is carried over into Pallas. Follow the
 * instructions to install Ceres at <a href="http://ceres-solver.org/building.html">ceres-solver.org/building.html</a>.
 * Once CMake, Ceres, and glog are built and installed use the following steps to build Pallas:
 *     - Navigate to the pallas root directory.
 *     - On the same level as the `README.md`, create a folder named `build`.
 *     - In the terminal, navigate to the newly created `build` folder.
 *     - Execute the following command: `cmake .. -DCERES_DIR:PATH=/path/to/CeresConfig.cmake`, where
 *       `/path/to/CeresConfig.cmake` denotes the folder where the file `CeresConfig.cmake` is located
 *       Currently on Linux, Ceres by default will place this file at `/usr/local/share/Ceres`, though
 *       this may change in the future and may be different on your machine.
 *     - From within the same build directory execute `make` in the terminal.
 * This should build Pallas. The folder `build/lib` will hold the library
 *
 * \subsection Usage
 *
 * An example for the Rosenbrock function is shown below:
 *
 *@code
#include "glog/logging.h"

// Each solver is defined in its own header file.
// include the solver you wish you use:
#include "pallas/basinhopping.h"

// define a problem you wish to solve by inheriting
// from the pallas::GradientCostFunction interface
// and implementing the Evaluate and NumParameters methods.
class Rosenbrock : public pallas::GradientCostFunction {
public:
    virtual ~Rosenbrock() {}

    virtual bool Evaluate(const double* parameters,
                          double* cost,
                          double* gradient) const {
        const double x = parameters[0];
        const double y = parameters[1];

        cost[0] = (1.0 - x) * (1.0 - x) + 100.0 * (y - x * x) * (y - x * x);
        if (gradient != NULL) {
            gradient[0] = -2.0 * (1.0 - x) - 200.0 * (y - x * x) * 2.0 * x;
            gradient[1] = 200.0 * (y - x * x);
        }
        return true;
    }

    virtual int NumParameters() const { return 2; }
};

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);

    // define the starting point for the optimization
    double parameters[2] = {-1.2, 0.0};

    // set up global optimizer options only initialization
    // is need to accept the default options
    pallas::Basinhopping::Options options;

    // initialize a summary object to hold the
    // optimization details
    pallas::Basinhopping::Summary summary;

    // create a problem from your cost function
    pallas::GradientProblem problem(new Rosenbrock());

    // solve the problem and store the optimal position
    // in parameters and the optimization details in
    // the summary
    pallas::Solve(options, problem, parameters, &summary);

    std::cout << summary.FullReport() << std::endl;
    std::cout << "Global minimum found at:" << std::endl;
    std::cout << "\tx: " << parameters[0] << "\ty: " << parameters[1] << std::endl;

    return 0;
}
 *@endcode
 *
 *\subsection getting_started Getting started
 * Pallas global optimization algorithms take as inputs an `Options` struct class specific
 * to each optimizer, `GradientProblem` which encapsulates the objective function to optimize,
 * a `const double*` pointing to the initial starting point for optimization
 * (except for Brute which takes a range of parameters), and a summary in which details of the
 * optimization are stored. The `Options` struct is a subclass specific to each optimizer
 * exposing the options that can be changed in order to customize the optimization procedure.
 * If Basinhopping is being used as the global optimizer, creating an instance of the default
 * options is as simple as:
 *@code
 * pallas::Basinhopping::Options options;
 * @endcode
 *
 * The default options can be changed by accessing member variables:
 * @code
 * options.maxiterations = 1000;
 * @endcode
 *
 * If the global optimizer employs a local minimizer, the options for the local minimizer
 * are accessed through the `options.local_minimizer_options` minimizer variable.
 * `options.local_minimizer_options` is itself a struct containing the parameters
 * to augment the functionality of the local minimization step(s). The local minimization
 * options are from the `ceres::GradientProblemSolver` renamed to `pallas::GradientLocalMinimizer`
 * to avoid confusion between the global and local solvers. If `DifferentialEvolution` is
 * being used as the global optimizer, the `options` struct requires that upper and lower
 * bounds be set for the current problem. Note, however, that if the final output is polished
 * (`options.polish = true`) the local optimization will not respect the bounds of the global
 * optimization due to the restriction of the Ceres local optimization algorithms to purely
 * unbounded problems. Both the `SimulatedAnnealing` and `Basinhopping` algorithms use
 * the `StepFunction` class to generate randomized candidate solutions. A pallas::scoped_ptr
 * to a `DefaultStepFunction` is created by default. This is not going to give optimal results
 * for your problem. If either of these algorithms are being used a class should inherit from
 * `StepFunction` and implement the `Step` method which takes as inputs a pointer to the
 * current solution and the number of parameters, then modifies the current solution in
 * place. If a `StepFunction` is used by the global optimizer, then the options struct
 * has a helper method `set_step_function` that swaps the pointer to the default step
 * function with the user defined functor. The following shows how to create a step
 * functor and replace it as the step function pointer in the options struct:
 *
 * @code
 * // inherit from StepFunction and implement Step method
 * class CustomStepFunction : public pallas::StepFunction {
 * public:
 *     CustomStepFunction(double step_size)
 *         : random_number_(new pallas::internal::RandomNumberGenerator<double>(-step_size, step_size)) {
 *     };
 *
 *     void Step(double* x, unsigned int num_parameters) {
 *         // implementation to modify x in place
 *     };
 *
 *      private:
 *          scoped_ptr<pallas::internal::RandomNumberGenerator<double>> random_number_;
 *      };
 *
 * // create the options for the solver
 * pallas::Basinhopping::Options options;
 *
 * // instantiate scoped pointer to StepFunction
 * pallas::scoped_ptr< CustomStepFunction > step(new CustomStepFunction(1.0));
 *
 * // use convenience method to replace default step function
 * options.set_step_function(step);
 * @endcode
 *
 *
 * Subclassing `pallas::GradientCostFunction`
 * and implementing the `Evaluate` and `NumParameters` methods defines your objective function.
 * Create a `GradientProblem` using:
 *
 * @code
 * pallas::GradientProblem problem(new YourObjectiveFuntion());
 * @endcode
 *
 * The gradient problem is what is then passed to the solver. The `parameters` for the global
 * optimization represents an initial guess required for the `Basinhopping` and `SimulatedAnnealing`
 * algorithms. It should be a `double*` and contain the same number of values as the `NumParameters`
 * method returns. Each global optimizer contains a `Summary` class used to store the results of
 * the global optimization. The summary is created in the same manner as the options struct, i.e.:
 *
 * @code
 * pallas::Basinhopping::Summary summary;
 * @endcode
 *
 * This is then passed as the final parameter to the solver. There are 2 methods optimize a
 * cost function. An instance of the solver can be created then optimized using the `global_optimizer.Solve`
 * method. There is also a `pallas::Solve` function added for convenience. It is overloaded
 * to create a global optimizer instance and run the optimization based on the parameters
 * passed to the function. To summarize, the two method of optimization are given by:
 *
 * @code
 * // create an instance of a global optimizer
 * pallas::Basinhopping bh;
 * bh.Solve(options, problem, parameters, &summary);
 *
 * // bypass the creation of the optimizer
 * pallas::Solve(options, problem, parameters, &summary)
 *
 * @endcode
 *
 *\subsection Credits
 * This libary uses the local minimization algorithms from Google's Ceres solver.
 * Implementations of the global optimization algorithms are based on Scipy's
 * optimize package. Because of the similarities between the Pallas algorithms
 * and scipy.optimize, much of the documentation was taken from their source.
 *
 */