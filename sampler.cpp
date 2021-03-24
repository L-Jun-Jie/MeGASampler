/*
 * sampler.cpp
 *
 *  Created on: 21 Mar 2021
 *      Author: batchen
 */
#include "sampler.h"

Sampler::Sampler(std::string input, int max_samples, double max_time, int max_epoch_samples, double max_epoch_time, int strategy) : original_formula(c), max_samples(max_samples), max_time(max_time), max_epoch_samples(max_epoch_samples), max_epoch_time(max_epoch_time), params(c), opt(c), solver(c),model(c){
	z3::set_param("rewriter.expand_select_store", "true");
    clock_gettime(CLOCK_REALTIME, &start_time);

    srand(start_time.tv_sec);

    params.set("timeout", 50000u);
    opt.set(params);
    solver.set(params);

	parse_formula(input);
    opt.add(original_formula); //adds formula as hard constraint to optimization solver (no weight specified for it)
    solver.add(original_formula); //adds formula as constraint to normal solver

    compute_and_print_formula_stats();

    results_file.open(input + ".samples");
}

double Sampler::duration(struct timespec * a, struct timespec * b) {
    return (b->tv_sec - a->tv_sec) + 1.0e-9 * (b->tv_nsec - a->tv_nsec);
}

double Sampler::get_elapsed_time(){
	return elapsed_time_from(start_time);
}

double Sampler::get_epoch_elapsed_time(){
	return elapsed_time_from(epoch_start_time);
}

double Sampler::elapsed_time_from(struct timespec start){
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &end);
    return duration(&start, &end);
}

void Sampler::parse_formula(std::string input){
	std::cout<<"Parsing input file: "<<input<<std::endl;
	z3::expr formula = c.parse_file(input.c_str()); //bat: reads smt2 file
	Z3_ast ast = formula;
	if (ast == NULL) {
		std::cout << "Could not read input formula.\n";
		exit(1);
	}
	original_formula = formula;
}

void Sampler::check_if_satisfiable(){
	z3::check_result result = solve(); // will try to solve the formula and put model in model variable
	if (result == z3::unsat) {
		std::cout << "Formula is unsat\n";
		finish();
		exit(0);
	} else if (result == z3::unknown) {
		std::cout << "Solver returned unknown\n";
		finish();
		exit(0);
	} else {
		std::cout<<"Formula is satisfiable\n";
	}
}

z3::check_result Sampler::solve(){
	z3::check_result result = z3::unknown;
	try {
		result = opt.check(); //bat: first, solve a MAX-SMT instance
	} catch (z3::exception except) {
		std::cout << "Exception: " << except << "\n";
		//TODO exception "canceled" can be thrown when Timeout is reached
		exit(1);
	}
	if (result == z3::sat) {
		model = opt.get_model();
	} else if (result == z3::unknown) {
		std::cout << "MAX-SMT timed out"<< "\n";
		try {
			result = solver.check(); //bat: if too long, solve a regular SMT instance (without any soft constraints)
		} catch (z3::exception except) {
			std::cout << "Exception: " << except << "\n";
			exit(1);
		}
		std::cout << "SMT result: " << result << "\n";
		if (result == z3::sat) {
			model = solver.get_model();
		}
	}
	return result;
}

bool Sampler::is_time_limit_reached(){
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    double elapsed = duration(&start_time, &now);
    if (elapsed >= max_time) {
        std::cout << "Stopping: timeout\n";
        finish();
    }
}

void Sampler::finish() {
    print_stats();
    results_file.close();
    exit(0);
}

void Sampler::print_stats(){
	std::cout<<"printing stats"<<std::endl;
	//TODO print all stats
}

z3::model Sampler::start_epoch(){
	std::cout<<"starting epoch"<<std::endl;
    opt.push(); // because formula is constant, but other hard/soft constraints change between epochs
    choose_random_assignment();
    z3::check_result result = solve(); //bat: find closest solution to random assignment (or some solution)
    opt.pop();
    epochs += 1;
	return model;
}

void Sampler::choose_random_assignment(){
    for (z3::func_decl & v : variables) { //bat: Choose a random assignment: for variable-> if bv or bool, randomly choose a value to it.
		if (v.arity() > 0 || v.range().is_array())
			continue;
		switch (v.range().sort_kind()) {
		case Z3_BV_SORT: // random assignment to bv
		{
			if (random_soft_bit) {
				for (int i = 0; i < v.range().bv_size(); ++i) {
					if (rand() % 2)
						assert_soft(v().extract(i, i) == c.bv_val(0, 1));
					else
						assert_soft(v().extract(i, i) != c.bv_val(0, 1));
				}
			} else {
				std::string n;
				char num[10];
				int i = v.range().bv_size();
				if (i % 4) {
					snprintf(num, 10, "%x", rand() & ((1<<(i%4)) - 1));
					n += num;
					i -= (i % 4);
				}
				while (i) {
					snprintf(num, 10, "%x", rand() & 15);
					n += num;
					i -= 4;
				}
				Z3_ast ast = parse_bv(n.c_str(), v.range(), c);
				z3::expr exp(c, ast);
				assert_soft(v() == exp);
			}
			break; // from switch, bv case
		}
		case Z3_BOOL_SORT: // random assignment to bool var
			if (rand() % 2)
				assert_soft(v());
			else
				assert_soft(!v());
			break; // from switch, bool case
		default:
			//TODO add int and real
			std::cout << "Invalid sort\n";
			exit(1);
		}
    } //end for: random assignment chosen
}

void Sampler::do_epoch(const z3::model & model){
	std::cout<<"doing epoch"<<std::endl;
	//TODO do epoch
}

void Sampler::compute_and_print_formula_stats(){
	// TODO save formula theory
	_compute_formula_stats_aux(original_formula);
//	std::cout << "Nodes " << sup.size() << '\n';
//	std::cout << "Internal nodes " << sub.size() << '\n';
	std::cout << "-------------FORMULA STATISTICS-------------" << '\n';
	std::cout << "Arrays " << num_arrays << '\n';
	std::cout << "Bit-vectors " << num_bv << '\n';
	std::cout << "Bools " << num_bools << '\n';
	std::cout << "Bits " << num_bits << '\n';
	std::cout << "Uninterpreted functions " << num_uf << '\n';
	std::cout << "Ints " << num_ints << '\n';
	std::cout << "Reals " << num_reals << '\n';
	std::cout << "Formula tree depth " << max_depth << '\n';
	std::cout << "--------------------------------------------" << '\n';

}

void Sampler::_compute_formula_stats_aux(z3::expr e, int depth){

    if (sup.find(e) != sup.end())
        return;
    assert(e.is_app());
    z3::func_decl fd = e.decl();
    if (e.is_const()) {
        std::string name = fd.name().str();
        if (var_names.find(name) == var_names.end()) {
            var_names.insert(name);
            // std::cout << "declaration: " << fd << '\n';
            variables.push_back(fd);
            if (fd.range().is_array()) {
               ++num_arrays;
            } else if (fd.is_const()) {
                switch (fd.range().sort_kind()) {
                case Z3_BV_SORT:
                    ++num_bv;
                    num_bits += fd.range().bv_size();
                    break;
                case Z3_BOOL_SORT:
                    ++num_bools;
                    ++num_bits;
                    break;
                case Z3_INT_SORT:
                    ++num_ints;
                    break;
                case Z3_REAL_SORT:
                    ++num_reals;
                    break;
                default:
                    std::cout << "Invalid sort\n";
                    exit(1);
                }
            }
        }
    } else if (fd.decl_kind() == Z3_OP_UNINTERPRETED) {
        std::string name = fd.name().str();
        if (var_names.find(name) == var_names.end()) {
            var_names.insert(name);
            // std::cout << "declaration: " << fd << '\n';
            variables.push_back(fd);
            ++num_uf;
        }
    }
//    if (e.is_bool() || e.is_bv()) {
//        sub.insert(e);
//    }
    sup.insert(e);
    if (depth > max_depth){
        max_depth = depth;
    }
    for (int i = 0; i < e.num_args(); ++i){
    	_compute_formula_stats_aux(e.arg(i), depth + 1);
    }
}

void Sampler::assert_soft(z3::expr const & e) {
    opt.add(e, 1);
}