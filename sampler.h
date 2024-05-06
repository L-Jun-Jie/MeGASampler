#ifndef SAMPLER_H_
#define SAMPLER_H_

#include <jsoncpp/json/json.h>
#include <z3++.h>

#include <algorithm>  // for std::find
#include <fstream>    //for results_file
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "sampler_config.h"

Z3_ast parse_bv(char const *n, Z3_sort s, Z3_context ctx);
std::string bv_string(Z3_ast ast, Z3_context ctx);

class Sampler {
    // Z3 objects
   public:
    z3::context &c;  // Must come first, memory-managed externally.
    z3::expr original_formula;
    bool debug = false;

   private:
    z3::params params;

   protected:
    z3::model model;
    z3::optimize opt;
    z3::solver solver;

    // Samples
    std::ofstream results_file;
    std::unordered_set<std::string> samples;

   protected:
    std::string input_filename;
    std::string output_dir;
    std::string json_filename;

    // Settings
    const MeGA::SamplerConfig config;

    bool random_soft_bit = false;  // TODO enable change from cmd line or remove
    bool should_exit = false;

    std::map<std::string, struct timespec> timer_start_times;  // the start time of a timer
    std::map<std::string, bool> is_timer_on;                   // marks whether a timer is on or off
    std::map<std::string, double> accumulated_times;           // accumulation of a timer time
    std::map<std::string, double> max_times;                   // setting the maximum time limit for a timer

    // Formula statistics
    int num_arrays = 0, num_bv = 0, num_bools = 0, num_bits = 0, num_uf = 0,
        num_ints = 0, num_reals = 0;
    std::vector<z3::func_decl> variables;          // function declarations in formulas
    std::vector<std::string> variable_names;       // used for model Record the names of the variables in the model (excluding uninterpreted functions)
    std::unordered_set<std::string> var_names = {  // variable names in formulas
        "bv", "Int", "true",
        "false"};                    // initialize with constant names so that
                                     // constants are not mistaken for variables
    int max_depth = 0;               // AST max depth
    bool has_arrays = false;         // whether the formula contains an array
    std::unordered_set<Z3_ast> sup;  // bat: nodes (=leaves?) independent Support Set?

    // Other statistics
    Json::Value json_output;  // json object collecting statistics to be printed
                              // with flag --json
    std::string sat_result = "unknown";
    std::string result = "unknown";  // success/failure
    std::string failure_cause;       // explanation for failure if result==failure
    int epochs = 0;
    int max_smt_calls = 0;            // maximum SMT calls
    int smt_calls = 0;                // current number of SMT calls
    unsigned long epoch_samples = 0;  // number of samples during the epoch
    unsigned long total_samples = 0;  // how many samples we stumbled upon
                                      // (repetitions are counted multiple times)
    unsigned long valid_samples = 0;  // how many samples were valid (repetitions
                                      // are counted multiple times)
    unsigned long unique_valid_samples =
        0;  // how many different valid samples were found (should always equal
            // the size of the samples set and the number of lines in the results
            // file)

    // Methods
    double duration(struct timespec *a, struct timespec *b);     // duration
    double elapsed_time_from(struct timespec start);             // the time elapsed since start
    double get_time_left(const std::string &category);           // the time remaining on a timer
    void parse_formula(const std::string &input);                // parsing Input Files
    void compute_and_print_formula_stats();                      // calculate and output information to the formula
    void _compute_formula_stats_aux(z3::expr e, int depth = 0);  // calculation formula information
    void assert_soft(z3::expr const &e);                         // assert a soft constraint
    /*
     * Assigns a random value to all variables and
     * adds equivalence constraints as soft constraints to opt.
     */
    void choose_random_assignment();
    /*
     * Adds negation of previous model as soft constraints to opt.
     */
    virtual void add_blocking_soft_constraints();
    /*
     * Tries to solve optimized formula (using opt) - if solve_opt is enabled.
     * If too long, resorts to regular formula (using solver).
     * Check result (sat/unsat/unknown) is returned.
     * If sat - model is put in model variable.
     */
    z3::check_result solve(const std::string &timer_category,
                           bool solve_opt = true);
    /*
     * Prints statistic information about the sampling procedure:
     * number of samples and epochs and time spent on each phase.
     */
    void print_stats();
    /*
     * Writes statistics to a newly created json file in json_dir.
     */
    void write_json();

   public:
    /*
     * Initializes limits and parameters.
     * Seeds random number generator.
     * Parses input file to get formula.
     * Computes formula statistics.
     * Creates output file (stored in results_file).
     */
    Sampler(z3::context *_c, const std::string &input,
            const std::string &output_dir, const MeGA::SamplerConfig &config);

    /*
     * Initializes solvers (MAX-SMT and SMT) with formula.
     */
    void initialize_solvers();
    /*
     * Checks if original_formula is satisfiable.
     * If not, or result is known, calls finish and exits.
     * If so, stores a model of it in model.
     */
    void check_if_satisfiable();
    /*
     * Generates and returns a model to begin a new epoch.
     */
    virtual z3::model start_epoch();
    /*
     * Sampling epoch: generates multiple valid samples from the given model.
     * Whenever a sample is produced we check if it was produced before (i.e.,
     * belongs to the samples set). If not, it is added to the samples set and
     * output to the output file.
     */
    virtual void do_epoch(const z3::model &model);
    /*
     * Returns the time that has passed since the sampling process began (since
     * this was created).
     */
    double get_elapsed_time();
    /*
     * Returns the time that has passed since the last epoch has began (since
     * start_epoch was last called).
     */
    double get_epoch_elapsed_time();
    /*
     * Prints stats and closes results file.
     */
    virtual void finish();
    /*
     * Document final result, clean up using finish(), then exit with exit code.
     */
    void safe_exit(int exitcode);

    // returns true iff the sample is unique (i.e., not seen before)
    bool save_and_output_sample_if_unique(const std::string &sample);
    std::string model_to_string(const z3::model &model);

    /*
     * Starts measuring time under the given category.
     */
    void set_timer_on(const std::string &category);

    /*
     * Stops measuring time under the given category (should always be preceded by
     * a matching call to set_timer_on(category)). The time difference from when
     * the timer went on to now is added to the accumulated time for that category
     * (in map accumulated_times).
     */
    void accumulate_time(const std::string &category);

    /*
     * Checks if global timeout is reached.
     * If so, calls finish.
     */
    bool is_time_limit_reached();
    /*
     * Checks if specific timeout is reached, returns true/false.
     */
    bool is_time_limit_reached(const std::string &category);

    /*
     * Set max time for timer (helps with timeout)
     */
    void set_timer_max(const std::string &category, double limit);

    /*
     * Make the sampler exit externally next time we check for time.
     */
    void set_exit() volatile;

    /*
     * Set the model to some value.
     */
    void set_model(const z3::model &new_model);

    /*
     * Set the number of epochs to some value.
     */
    void set_epochs(int _epochs);

    virtual ~Sampler(){};
};

#endif /* SAMPLER_H_ */
