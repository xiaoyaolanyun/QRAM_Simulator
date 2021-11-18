#pragma once
#include <memory>
#include <tuple>
#include <optional>
#include "data_structures.h"
#include "logger.h"
#include "util.h"
#include <algorithm>
#include <random>

struct Branch {
    size_t address;
    Tree state;
    Qubit bus;
    // bool phase = true; // false for -1; true for +1
    // double amplitude = 1.0;

    // bool valid = true;

    Branch() = default;
    Branch(int nlayer, size_t addr) : state(nlayer), address(addr) {}
    Branch(const Branch& old_branch) {
        state = old_branch.state;
        address = old_branch.address;
        bus = old_branch.bus;
        // phase = old_branch.phase;
        // amplitude = old_branch.amplitude;
    }

    inline Tree& get_state() { return state; }
    
    inline bool operator<(const Branch& other) const {
        return tie(address, state, bus) < tie(other.address, other.state, other.bus);
    }

    inline bool operator==(const Branch& other) const {
        return tie(address, state, bus) == tie(other.address, other.state, other.bus);
    }

    inline auto& data_of(size_t idx)
    {
        return state.access_treenode(idx)->data;
    }

    inline auto& addr_of(size_t idx)
    {
        return state.access_treenode(idx)->addr;
    }

    inline string to_string() const
    {
        return format("|{}> {}\n{}", address, bus.to_string(), state.to_string());
    }
    inline string to_string_simple() const
    {
        return format("|{}> {}\n", address, bus.to_string());
    }
    void run_acopy(bool a);
    void run_swap(size_t node);
    void run_cswap(size_t node);
    void run_bitflip(size_t qubit_id);
    double run_phaseflip(size_t qubit_id);
    void run_fetchdata(vector<bool>& memory);
    void run_busout();
};


class QRAM_bb {

    // record operations
    // append noise operators
    // init state
    // run

    constexpr static int default_nlayer = 3;

    int nlayers = default_nlayer;
    set<size_t> address;
    vector<bool> memory;
    TimeSlices operations;
    TimeSlices raw_operations;
    vector<pair<OperationType, double>> noise_parameters;
    vector<pair<Branch, complex<double>>> branches;

    struct TreeFidelity {
        // vector<complex<double>> amplitudes;
        complex<double> accumulate_amplitude = 0;
        double total_prob = 0;
    };
    
    map<Tree, TreeFidelity> tree_fidelity;

    void add_or_append(const Tree& tree, complex<double> amplitude, bool valid);
    bool compare_result(const Branch& branch);

public:
    // double global_coef = 1.0;
	unsigned int seed = 10101;
    default_random_engine reng;
    using noise_type = typename decltype(noise_parameters)::value_type;
    using BranchType = typename decltype(branches)::value_type;
    
    QRAM_bb(int ndigit) : nlayers(ndigit) {
        reng.seed(seed);
        memory.resize(pow2(nlayers));
        generate_QRAM_operations();
    }

    void set_address(set<size_t> &addresses);
    void set_address_sample(size_t n_sample);
    void set_address_full();
    void set_address_portion(double portion);
    void set_memory(vector<bool> new_memory);
    void set_memory_random();

    inline size_t get_qubit_num() const { return 2 * (pow2(nlayers) - 1); }
    inline void set_seed(unsigned int _seed) { seed = _seed; reng.seed(_seed); }
    inline size_t get_seed() { return seed; }
    inline set<size_t>& get_address() { return address; }
    inline void clear_address() { address.clear(); }    
    inline vector<bool>& get_memory() { return memory; }
    inline size_t asz() { return address.size(); }
    inline auto get_branches() { return branches; }
    inline TimeSlices get_operations() const { return operations; }
    inline void clear_noise() { noise_parameters.clear(); }
    inline void add_noise_model(OperationType t, double v) {
        noise_parameters.push_back({ t,v });
    }
    inline void add_noise_models(vector<noise_type> noise) {
        noise_parameters.insert(noise_parameters.end(), noise.begin(), noise.end());
    }

    // operations recorder
    // function with _    : Pack generator
    // function without _ : Time slice generator
    
    OperationPack _cswap_layer(int layer);
    OperationPack _swap_layer(int layer);
    OperationPack _hadamard_first();
    OperationPack _fetch_data();

    TimeSlices route_one_step(int step);
    TimeSlices route();
    TimeSlices data_fetch();
    void generate_QRAM_operations();

private:    
    struct BranchInfoHelper {
        complex<double> amplitude = 0;
        size_t address;
        bool memory;
        bool expect_memory;
        inline bool correct() const { return memory == expect_memory; }
    };
    struct TreeInfoHelper {
        double prob = 0.0;
        vector<BranchInfoHelper> branches;
        TreeInfoHelper() = default;
    };
    map<Tree, TreeInfoHelper, less<Tree>> state_view;
    set<Tree, less<Tree>> state_tree_view;
public:
    void state_view_preparation();
    set<Tree, less<Tree>> get_state_tree_view();
    string state_view_to_string();
    
    double get_global_coef() const;
    double get_fidelity();

    OperationPack _noise_one_step();
    void append_noise();
    
    void run();

    void init_state(); // create copies of the branches
    void run_acopy(int layer);
    void run_swap(size_t node);
    void run_cswap(size_t node);
    void run_bitflip(size_t qubitid);
    void run_phaseflip(size_t qubitid);
    void run_busin(); // create copies of the branches_1
    void run_fetchdata();
    void run_busout(); // remove copies of the branches_1 and set branches_0 / bus
    void try_merge();
    void try_merge_v2();
    
    string to_string() const;

private:
    void CNOT_bb(bool ctrl, Qubit& targ) {
        if (!ctrl) { return; }
        else { targ.NOT(); }
    }

    void CNOT_bb(Qubit ctrl, Qubit& targ) {
        if (!ctrl.data()) { return; }
        else { targ.NOT(); }
    }

};