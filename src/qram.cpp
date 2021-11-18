#include "qram.h"

namespace QRAMOp {
    void SWAP_bb(Qubit& q1, Qubit& q2) {
        swap(q1, q2);
    }

    void router_bbbb(Qubit& addr, Qubit& data, Qubit& x0, Qubit& x1) {
        if (!addr.data()) SWAP_bb(x0, data);
        else              SWAP_bb(x1, data);
    }

    void CNOT_tb(Qutrit ctrl, Qubit& targ) {
        if (ctrl.data() == 0) { return; }
        else { targ.NOT(); }
    }    
}

void Branch::run_acopy(bool a)
{
    state.access_treenode(0)->data.CNOT(a);
}

void Branch::run_swap(size_t node)
{
    swap(state.access_treenode(node)->data, state.access_treenode(node)->addr);
}

void Branch::run_cswap(size_t nodeid)
{
    auto node = state.access_treenode(nodeid);
    auto left = state.left_of(node);
    auto right = state.right_of(node);
    auto& a = node->addr;
    auto& d = node->data;
    auto& l = left->data;
    auto& r = right->data;
    QRAMOp::router_bbbb(a, d, l, r);
}

void Branch::run_bitflip(size_t qubit_id)
{
    size_t node_id = qubit_id / 2;
    node_t* node = state.access_treenode(node_id);
    Qubit &qubit = qubit_id % 2 ? node->data : node->addr;
    qubit.NOT();
}

double Branch::run_phaseflip(size_t qubit_id)
{
    size_t node_id = qubit_id / 2;
    auto node = state.get_treenode(node_id);
    auto qubit = qubit_id % 2 ? node->data : node->addr;
    if (qubit.ZEROONE == 1) {
        return -1;
    }
    else if (qubit.ZEROONE == 0) {
        return +1;
    }
    else {
        throw runtime_error("Unknown state.");
    }
}

void Branch::run_busout()
{
    Qubit& data = state.root()->data;
    swap(data, bus);
}

void QRAM_bb::add_or_append(const Tree& tree, complex<double> amplitude, bool valid)
{
    if (tree_fidelity.find(tree) == tree_fidelity.end()) {
        tree_fidelity[tree] = TreeFidelity();
    }
    TreeFidelity& fid = tree_fidelity[tree];
    // tree_fidelity[tree].amplitudes.push_back(amplitude);
    if (valid) {
        fid.accumulate_amplitude += amplitude;
    }
    fid.total_prob += abs_sqr(amplitude);
}

bool QRAM_bb::compare_result(const Branch& branch)
{
        size_t addr = branch.address;
        Qubit state = branch.bus;
        // _logger.info(format("addr={}, state={}, memory={}\n", addr, state.state(), memory[addr]));
        return state.state() == memory[addr];
}

OperationPack QRAM_bb::_cswap_layer(int layer)
{
    vector<size_t> ids = get_nodes_in_layer(layer);
    OperationPack ops;
    for (size_t id : ids) {
        ops.append(
            Operation(OperationType::ControlSwap, { id })
        );
    }
    ops.set_name("CSWAP " + num2str(layer));
    return ops;
}

OperationPack QRAM_bb::_swap_layer(int layer) {
    OperationPack ops;
    
    vector<size_t> ids = get_nodes_in_layer(layer);
    for (size_t id : ids) {
        ops.append(
            Operation(OperationType::SwapInternal, { id })
        );
    }
    ops.set_name("SWAP " + num2str(layer));
    return ops;
}

OperationPack QRAM_bb::_hadamard_first() {
    OperationPack ops;
    ops.append(
        Operation(OperationType::HadamardData, { })
    );
    ops.set_name("H");
    return ops;
}
OperationPack QRAM_bb::_fetch_data() {

    OperationPack ops;
    ops.append(
        Operation(OperationType::FetchData, { })
    );
    ops.set_name("Fetch");
    return ops;
}

TimeSlices QRAM_bb::route_one_step(int layer) {
    TimeSlices time_slices;
    { // step 1
        OperationPack pack;
        pack.append(
            Operation(OperationType::FirstCopy, { (size_t)layer })
        );
        pack.set_name("FirstCopy " + num2str(layer));
        time_slices.append(pack);
    }

    { // step 2
        for (int i = 0; i < layer; ++i) {
            time_slices.append(_cswap_layer(i));
        }        
    }

    { // step 3
        time_slices.append(_swap_layer(layer));
    }
    return time_slices;
}

TimeSlices QRAM_bb::route() {
    TimeSlices time_slices;
    for (int i = 0; i < nlayers; ++i) {
        time_slices.append(route_one_step(i));
    }
    return time_slices;
}

TimeSlices QRAM_bb::data_fetch() {
    TimeSlices time_slices;
    { // step 1
        time_slices.append(_hadamard_first());
    }

    { // step 2
        for (int i = 0; i < nlayers - 1; ++i) {
            time_slices.append(_cswap_layer(i));
        }
    }

    TimeSlices reverse = time_slices.reverse();

    { // step 3
        time_slices.append(
            _fetch_data()
        );
    }

    { // uncomputing
        time_slices.append(reverse);
    }

    return time_slices;
}

void QRAM_bb::generate_QRAM_operations() {

    profiler m(__FUNCTION__);
    raw_operations.clear();
    raw_operations.append(route());
    raw_operations.append(data_fetch());
    raw_operations.append(route().reverse());
}

void QRAM_bb::init_state() {
    branches.clear();
    branches.reserve(4 * address.size());
    for (auto addr : address) {
        Branch b(nlayers, addr);
        branches.push_back({ b, 1.0 });
    }
}

void QRAM_bb::set_address(set<size_t> &addresses) {
    address = addresses;
    init_state();
}

void QRAM_bb::set_address_sample(size_t nsample) {
    choice_from(address, nlayers, nsample, reng);
}

void QRAM_bb::set_address_portion(double portion) {
    address.clear();
    uniform_real_distribution<double> ud(0, 1);
    for (size_t i = 0;i < pow2(nlayers);++i) {
        if (ud(reng) < portion)
            address.insert(i);
    }
}

void QRAM_bb::set_address_full() {
    address.clear();
    for (size_t i = 0;i < pow2(nlayers);++i) {
        address.insert(i);
    }
}

void QRAM_bb::set_memory(vector<bool> new_memory) {
    if (new_memory.size() != pow2(nlayers)) throw runtime_error("Bad Input.");
    else memory.swap(new_memory);
}

void QRAM_bb::set_memory_random() {
    random_memory(memory, reng);
}

double QRAM_bb::get_global_coef() const {
    
    double global_coef = 0;
    for (const auto& branch : branches) {
        double re = branch.second.real();
        double im = branch.second.imag();
        global_coef += re * re + im * im;
    }

    global_coef = 1 / sqrt(global_coef);
    return global_coef;
}

double QRAM_bb::get_fidelity() {

    profiler("qram::get_fidelity");
    if (memory.size() != pow2(nlayers))
        throw runtime_error("Memory not set.");

    if (address.size() == 0)
        throw runtime_error("Address not set.");
    
    double ret = 0;

    double global_coef = get_global_coef();

    for (auto& branch : branches) {
        complex<double> actual_amp = branch.second * global_coef;
        Tree& tree = branch.first.state;
        // +1 or -1
        bool correct = compare_result(branch.first);
        // _logger << format("{}\n", correct);
        // add_or_append(tree, actual_amp, correct);

        if (tree_fidelity.find(tree) == tree_fidelity.end()) {
            tree_fidelity[tree] = TreeFidelity();
        }
        TreeFidelity& fid = tree_fidelity[tree];
        // tree_fidelity[tree].amplitudes.push_back(amplitude);
        if (correct) {
            fid.accumulate_amplitude += actual_amp;
        }
        fid.total_prob += abs_sqr(actual_amp);
        
    }
    for (auto& fidelity : tree_fidelity) {
        // complex<double> total_amp = accumulate(
        //     fidelity.second.amplitudes.begin(), fidelity.second.amplitudes.end(), complex<double>(0));
        // double f = abs_sqr(total_amp);
        double f = abs_sqr(fidelity.second.accumulate_amplitude);
        
        ret += f * fidelity.second.total_prob;
    }
    
    // consider the input address probability distribution
    ret = ret / address.size();

    return ret;
}

void QRAM_bb::state_view_preparation() {
    state_view.clear();

    double global_coef = get_global_coef();

    for (const auto& branch : branches) {
        const Tree& tree_state = branch.first.state;
        
        auto iter = state_view.find(tree_state);
        if (iter == state_view.end()) {
            state_view.insert({ tree_state, TreeInfoHelper() });
        }
        BranchInfoHelper branchinfo;
        branchinfo.amplitude = branch.second * global_coef;
        branchinfo.address = branch.first.address;
        branchinfo.expect_memory = memory[branchinfo.address];
        branchinfo.memory = branch.first.bus.state();
        state_view[tree_state].branches.push_back(branchinfo);
        state_view[tree_state].prob += abs_sqr(branchinfo.amplitude);
        state_tree_view.insert(tree_state);
    }
}

set<Tree, less<Tree>> QRAM_bb::get_state_tree_view() {
    return state_tree_view;
}

string QRAM_bb::state_view_to_string() {
    string s = format(
        "Number of Tree Classes: {}\n", state_view.size()
    );
    for (const auto& clas: state_view) {
        auto& treeinfo = clas.second;
        double prob = treeinfo.prob;
        double nbranches = treeinfo.branches.size();
        s += format("p={} nbranch={}", prob, nbranches);
        s += '\n';
    }
    return s;
}

void QRAM_bb::run() {
    FunctionProfiler;

    {
        profiler m("run - init_state");
        init_state();
    }
    {
        profiler m("run - tree_fid_clear");
        tree_fidelity.clear();
    }
    {
        profiler m("run - append_noise");
        append_noise();
    }
    // global_coef = 1.0 / sqrt(asz());
    
    if (memory.size() == 0) {
        throw runtime_error("Memory not set.");
    }
    
    bool flag = true;
    for (OperationPack& ops : operations.time_slices) {
        for (Operation& op : ops.operations) {
            switch (op.type) {
            case OperationType::ControlSwap:
                run_cswap(op.targets[0]); break;
            case OperationType::HadamardData:                
                if (flag) { run_busin(); flag = false; break; }
                else { run_busout(); break; }
            case OperationType::SwapInternal:                
                run_swap(op.targets[0]); break;
            case OperationType::FirstCopy:                
                run_acopy(op.targets[0]); break;
            case OperationType::FetchData:                
                run_fetchdata(); break;
            case OperationType::BitFlip:
                run_bitflip(op.targets[0]); break;
            case OperationType::PhaseFlip:
                run_phaseflip(op.targets[0]); break;
            default:
                throw runtime_error("Bad type.");
            }
        }
    }
}

void QRAM_bb::run_acopy(int layer) {
    for (auto& branch : branches) {
        bool a = get_digit_reverse(branch.first.address, layer, nlayers);
        branch.first.run_acopy(a);
    }
}

void QRAM_bb::run_swap(size_t node) {

    for (auto& branch : branches) {
        branch.first.run_swap(node);
    }
}

void QRAM_bb::run_cswap(size_t node) {
    FunctionProfiler;
    for (auto& branch : branches) {
        branch.first.run_cswap(node);
    }
}

void QRAM_bb::run_bitflip(size_t qubit_id) {
    for (auto& branch : branches) {
        branch.first.run_bitflip(qubit_id);
    }    
}

void QRAM_bb::run_phaseflip(size_t qubit_id) {
    for (auto& branch : branches) {
        branch.second *= branch.first.run_phaseflip(qubit_id);
    }
}

void QRAM_bb::run_busin() {

    profiler m("run_busin");
    size_t n_current_branch = branches.size();
    branches.reserve(2 * n_current_branch);
    for (size_t i = 0; i < n_current_branch;++i) {
        branches.push_back(branches[i]);
        if (branches.back().first.data_of(0).data() == 0) {
            branches.back().first.data_of(0).NOT();
        }
        else {
            branches[i].first.data_of(0).NOT();
            branches.back().second *= -1;
        }
    }
    // sort(branches.begin(), branches.end(), [](const BranchType& t1, const BranchType t2)
    //     {
    //         return t1.first < t2.first;
    //     }
    // );
    // global_coef *= 1.0 / sqrt(2);
}

void QRAM_bb::run_fetchdata() {
    int branchid = 0;
    for (auto& branch : branches) {
        branchid++;
        Operation::target_t ids = get_nodes_in_layer(branch.first.state.get_nlayer() - 1);
        vector<const node_t*> nodes = branch.first.state.get_treenodes(ids);
        for (size_t i = 0, pos = 0; i < nodes.size(); ++i, pos += 2) {
            const node_t* node = nodes[i];
            // _logger << format("branch={} node=[{},{}] memory=[{},{}]\n",
            //     branchid,
            //     node->addr.state(), node->data.state(),
            //     memory[pos], memory[pos + 1]);
            if (node->addr.data() == 0) {
                if (memory[pos]) {
                    if (node->data.data() == 1) {
                        branch.second *= -1;
                    }
                }
            }
            else {
                if (memory[pos + 1]) {
                    if (node->data.data() == 1) {
                        branch.second *= -1;
                    }
                }
            }
        }
    }
}

void QRAM_bb::run_busout() {
    // |0> -> |0> + |1>
    // |1> -> |0> - |1>
    run_busin();
    // try_merge_v2();
    try_merge();
    // try_merge();

    for (auto& branch : branches) {
        branch.first.run_busout();
    }
}

void QRAM_bb::try_merge() {

    profiler m("try_merge");
    for (size_t i = 0; i < branches.size(); i++) {
        for (size_t j = i + 1; j < branches.size(); ++j) {
            if (ignorable(branches[i].second) && ignorable(branches[j].second))
                continue;
            if (branches[i].first == branches[j].first) {
            branches[i].second += branches[j].second;
            branches[j].second = 0;
            // _logger << "merged" << "\n";
            }
        }
    }
    auto iter = remove_if(branches.begin(), branches.end(), [](const BranchType& b)
        {
            return ignorable(b.second);
        }
    );
    branches.erase(iter, branches.end());

}

void QRAM_bb::try_merge_v2() {

    profiler m("try_merge_v2");
    auto less = [](const BranchType& t1, const BranchType& t2)
    {
        return t1.first < t2.first;
    };
    auto equal = [](const BranchType& t1, const BranchType & t2)
    {
        return t1.first == t2.first;
    };
    auto fn = [](BranchType& t1, const BranchType& t2)
    {
        t1.second += t2.second;
    };
    auto remove = [](const BranchType& b)
    {
        return ignorable(b.second);
    };

    sort(branches.begin(), branches.end(), less);
    auto iter = unique_and_merge(branches.begin(), branches.end(), equal, fn);
    iter = remove_if(branches.begin(), iter, remove);
    branches.resize(distance(branches.begin(), iter));
}

OperationPack QRAM_bb::_noise_one_step() {
    OperationPack pack;
    pack.set_name("Noise");
    uniform_real_distribution<double> ud;
    for (auto noise : noise_parameters) {
        for (size_t i = 0; i < get_qubit_num(); ++i) {
            double r = ud(reng);
            if (r < noise.second) {
                pack.append(Operation(noise.first, { i }));
            }
        }
    }
    
    return pack;
}

void QRAM_bb::append_noise() {
    operations = raw_operations;
    vector<OperationPack>& time_slices = operations.time_slices;
    for (auto& time_slice : time_slices) {
        OperationPack&& noise = _noise_one_step();
        if (!noise.empty()) {
            time_slice.append(noise);
        }
    }
}

string QRAM_bb::to_string() const {
    profiler("qram::to_string");
    stringstream out;

    out << "memory=[";
    for (const auto& bit : memory) {
        out << (bool(bit) ? '1' : '0');
    }
    out << "]" << endl;
    double global_coef = get_global_coef();
    for (const auto& branch : branches) {
        out << branch.second * global_coef << endl;
        out << branch.first.to_string() << endl;
    }
    return out.str();
}