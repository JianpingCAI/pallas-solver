/*!
* \file brute.h
*
* \author Ryan Latture
* \date 5-18-15
*
* This file contains a C++ implementation of a brute force minimzation algorithm.
* This code relies on the Google Ceres local minimization functions, and
* is inspired by the SciPy implementaion found in scipy.optimize.
*/

// Pallas Solver
// Copyright 2015. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Author: ryan.latture@gmail.com (Ryan Latture)

#ifndef PALLAS_BRUTE_H
#define PALLAS_BRUTE_H

#include "pallas/types.h"

namespace pallas {

    /**
     * @brief Minimizes an objective function by brute force, trying all possible combinations of specified parameter ranges and outputing the best solution found.
     * @details <B>Example</B>
     * @code
     #include "glog/logging.h"
 
      // Each solver is defined in its own header file.
      // include the solver you wish you use:
      #include "pallas/brute.h"
 
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
          pallas::Brute::Options options;
 
          // initialize a summary object to hold the
          // optimization details
          pallas::Brute::Summary summary;
 
          // create a problem from your cost function
          pallas::GradientProblem problem(new Rosenbrock());
 
          // set up the subset of parameter space to test
          std::vector<pallas::Brute::ParameterRange> ranges = {pallas::Brute::ParameterRange(-3.0, 3.0, 7),
                                                               pallas::Brute::ParameterRange(-3.0, 3.0, 7)};
 
          // solve the problem and store the optimal position
          // in parameters and the optimization details in
          // the summary
          pallas::Solve(options, problem, ranges, parameters, &summary);
 
          std::cout << summary.FullReport() << std::endl;
          std::cout << "Global minimum found at:" << std::endl;
          std::cout << "\tx: " << parameters[0] << "\ty: " << parameters[1] << std::endl;
 
         return 0;
     }
     * @endcode
     */
    class Brute {
    public:
        /**
         * Configurable options for modifying the default behaviour of the brute algorithm.
         */
        struct Options {
            /**
             * @brief Default constructor
             * @details This tries to set up reasonable defaults for optimization. It is highly
             * recommended that the user read and overwrite the defaults based on the cost function.
             */
            Options() {
                local_minimizer_options = GradientLocalMinimizer::Options();
                polish_output = false;
                is_silent = true;
            };

            /**
             * Contains any changes to the default options for the local minimization algorthm. See the documentation for ceres::GradientProblemSolver::Options for relevant options.
             */
            GradientLocalMinimizer::Options local_minimizer_options;
            
            /**
             * Whether the global minimum found through brute force should be subjected to a local minimzation "polishing" step before returning the result.
             */
            bool polish_output;

            /**
             * Whether to log failure information relating the to global optimization algorithm using glog.
             */
            bool is_silent;

        };

        /**
         * @brief Contains a summary of the optimization.
         * @details This struct contains the result of the optimization and has convenience methods for printing reports of a completed optimization.
         */
        struct Summary {
           /**
            * @brief Default constuctor
            */
            Summary();

            std::string BriefReport() const;/**<A brief one line description of the state of the solver after termination.*/

            std::string FullReport() const;/**<A full multi-line description of the state of the solver after termination.*/

            TerminationType termination_type;/**<Reason optimization was terminated*/

            std::string message;/**<Message describing why the solver terminated.*/

            double final_cost;/**<Cost of the problem (value of the objective function) after the optimization.*/

            GradientLocalMinimizer::Summary local_minimization_summary;/**<Summary from the local minimization iteration if the result from pallas::Brute was polished.*/

            unsigned int num_parameters;/**<Number of parameters in the problem.*/

            unsigned int num_iterations;/**<Number of basinhopping iterations*/

            double total_time_in_seconds;/**<total time elapsed in global minimizer*/

            double local_minimization_time_in_seconds;/**<time spent in local minimizer*/

            double permuation_build_time_in_seconds;/**<time spent calculating the possible permutations of the input parameter ranges*/

            double cost_evaluation_time_in_seconds;/**<time spent evaluating cost function (outside local minimization)*/

            bool was_polished;/**<specifies whether the output was polished*/
        };

        /**
         * @brief Range of values to test for the `ith` degree of freedom.
         * @details For a given degree of freedom in an objective function, this struct specifies 
         * the number of points to test along the given degree of freedom to test between 
         * `start` and `stop`. The search is inclusive, i.e. `[start, stop]`.
         * 
         * @param start double. The first value to test along the degree of freedom.
         * @param stop double. The last value to test along the degree of freedom.
         * @param size int. The discete number of samples to test on the range `[start, stop]`
         */
        struct ParameterRange {

            /**
             * @brief Default constuctor
             * @details All values are initialized to 0.
             */
            ParameterRange() : start(0), stop(0), size(0) {};

            /**
             * @brief Constuctor
             * @details Specifies the interval over which the ith degree of freedom should be tested.
             * 
             * @param start double. The first value to test along the degree of freedom.
             * @param stop double. The last value to test along the degree of freedom.
             * @param size int. The discete number of samples to test on the range `[start, stop]`.
             */
            ParameterRange(double start, double stop, int size)
                    : start(start), stop(stop), size(size) {};

            double start;/**<The first value to test along the degree of freedom.*/
            double stop;/**<The last value to test along the degree of freedom.*/
            int size;/**<The discete number of samples to test on the range `[start, stop]`.*/
        };

        /**
         * @brief Default constuctor
         */
        Brute() {};

        /**
         * @brief Minimizes the specified gradient problem.
         * @details The specified options are used to setup a brute instance which
         * is then used to minimize the GradientProblem. The optimal solution is stored
         * in `parameters` and a summary of the global optimization can be found in `summary`.
         * 
         * @param options pallas::Brute::Options. Options used to configure the optimization.
         * @param problem pallas::GradientProblem. The problem to optimize.
         * @param parameters double*. The starting point for further optimization.
         * @param summary Brute::Summary*. Summary instance to store the optimization details.
         */
        void Solve(const Brute::Options options,
                   const GradientProblem& problem,
                   const std::vector<Brute::ParameterRange> &parameter_ranges,
                   double* parameters,
                   Brute::Summary *global_summary);

    private:
        /**
         * @brief Expands the parameter ranges into linearly spaced vectors between the start and stop points for the ith degree of freedom.
         * 
         * @param parameter_ranges std::vector<Brute::ParameterRange>. The parameter ranges input into pallas::Brute::Solve are forwarded to this function.
         * @return An std::vector<Vector> containing the linearly spaced vectors bewteen the start and end points.
         */
        std::vector<Vector> expand_parameter_ranges_(const std::vector<Brute::ParameterRange> &parameter_ranges);

        /**
         * @brief Constructs all possible permutations of the input parameter ranges.
         * 
         * @param expanded_ranges std::vector<Vector>. Contains linearly spaced vectors of all sample points for the input parameter ranges. Used internallly on the result of Brute::expand_parameter_ranges_.
         * @return All possible permutations of the input parameter ranges. Because all permutations are precalculated this could be memory intensive if a large number of parameters and sample points are provided.
         */
        std::vector<Vector> build_permuations_(const std::vector<Vector> &expanded_ranges);
    };

    /**
     * @brief Helper function which avoids going through the interface of the pallas::Brute class.
     * @details The specified options are used to setup a brute instance which
     * is then used to minimize the GradientProblem. The optimal solution is stored
     * in `parameters` and a summary of the global optimization can be found in `summary`.
     *
     * @param options pallas::Brute::Options. Options used to configure the optimization.
     * @param problem pallas::GradientProblem. The problem to optimize.
     * @param parameters double*. The starting point for further optimization.
     * @param summary Brute::Summary*. Summary instance to store the optimization details.
     */
    void Solve(const Brute::Options options,
               const GradientProblem& problem,
               const std::vector<Brute::ParameterRange> &parameter_ranges,
               double* parameters,
               Brute::Summary *global_summary);
} // namespace pallas

#endif // PALLAS_BRUTE_H