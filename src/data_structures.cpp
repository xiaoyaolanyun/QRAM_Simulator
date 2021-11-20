#include "data_structures.h"

// QuditPool QuditPool::_pool;

// string Qudit::to_string() {
//     stringstream ss;
//     ss << name() << ":" << state();
//     return ss.str();
// }

vector<bool> calc_pos(int pos, int layer) {
    vector<bool> posv;
    posv.reserve(layer);

    for (int i = 0; i < layer; i++) {
        posv.push_back(get_digit(pos, i));
    }
    return posv;
}

vector<size_t> get_nodes_in_layer(int layer) {
    // layer -= 1;
    size_t begin = pow2(layer) - 1;
	size_t end = begin + pow2(layer);
    vector<size_t> ret;
    ret.resize(end - begin);
    for (size_t i = begin; i < end; ++i) {
        ret[i - begin] = i;
    }
    return ret;
}

void test_get_nodes_in_layer() {
	for (int i = 0; i < 3; ++i) {
		cout << vec2str(get_nodes_in_layer(i)) << endl;
	}
}

string pos2str(int pos, int layer) {
    vector<bool> posv = calc_pos(pos, layer);
    string ret;
    for (bool i : posv) {
        char x = i ? '1' : '0';
        ret = x + ret;
    }
    return ret;
}

Tree::Tree(int n_layer_) {
    n_layer = n_layer_;
    size_t size = pow2(n_layer) - 1;
    treenodes.resize(size);
}

Tree::Tree(const Tree& old) {
    treenodes = old.treenodes;
    n_layer = old.n_layer;
}

bool Tree::is_same(const Tree& other) const {
    if (n_layer != other.n_layer)
        throw runtime_error("Bad comparison.");

    bool ret = (treenodes == other.treenodes);
    // if (ret) { printf("Tree same\n"); }
    // else { printf("Tree not same\n"); }
    return ret;
}

bool Tree::less_than(const Tree& other) const {
    if (n_layer != other.n_layer)
        throw runtime_error("Bad comparison.");

    bool ret = (treenodes < other.treenodes);
    // bool ret = vector_less(treenodes, other.treenodes);
    // if (ret) { fmt::print("Tree {} less than {}\n", to_string(), other.to_string()); }
    // else { printf("Tree not less\n"); }
    return ret;
}

const node_t* Tree::get_treenode(size_t idx) const {
    if (idx >= treenodes.size()) { return nullptr; }
    if (idx < 0) { return nullptr; }

    return &treenodes[idx];
}

node_t* Tree::access_treenode(size_t idx) {
    if (idx >= treenodes.size()) { return nullptr; }
    if (idx < 0) { return nullptr; }

    return &treenodes[idx];
}

vector<const node_t*> Tree::get_treenodes(vector<size_t> ids) const {
    vector<const node_t*> ret;
    ret.resize(ids.size());
    for (size_t i = 0; i < ids.size(); ++i) {
        ret[i] = get_treenode(ids[i]);
    }
    return ret;
}

vector<const node_t*> Tree::get_treenodes(int layer) const {
    auto ids = get_nodes_in_layer(layer);
    return get_treenodes(ids);
}

size_t Tree::left_of(size_t idx) const {
    return 2 * idx + 1;
}

node_t* Tree::left_of(node_t* node) {
    size_t idx = idx_of(node);
    return access_treenode(left_of(idx));
}

size_t Tree::right_of(size_t idx) const {
    return 2 * idx + 2;
}

node_t* Tree::right_of(node_t* node) {
    size_t idx = idx_of(node);
    return access_treenode(right_of(idx));
}

size_t Tree::parent_of(size_t idx) const {
    return size_t((idx - 1) / 2);
}

node_t* Tree::parent_of(node_t* node) {
	size_t idx = idx_of(node);
	return access_treenode(parent_of(idx));
}

string Tree::to_string() const {
    stringstream ret;
    ret << 0 << ": " << get_treenode(0)->to_string() << "\n";
    
    for (int j = 1; j < n_layer;++j) {
		for (size_t i = pow2(j) - 1; i < pow2(j + 1) - 1; ++i) {
			ret << i << ": ";
			ret << get_treenode(i)->to_string();
			ret << " ";
		}
        ret << "\n";
    }
    return ret.str();
}

string type2str(OperationType type) {
    static std::map<OperationType, std::string> _TypeNameMap = {
        {OperationType::ControlSwap, "cSwap"},
        {OperationType::HadamardData, "BusIn"},
        {OperationType::SwapInternal, "Swap"},
        {OperationType::FirstCopy, "ACopy"},
        {OperationType::FetchData, "FetchData"},
        {OperationType::SetZero, "SetZero"},
        {OperationType::Damp, "Damp"},
        {OperationType::BitFlip, "BitFlip"},
        {OperationType::PhaseFlip, "PhaseFlip"},
    };
    if (type > OperationType::Begin && type < OperationType::End)
        return _TypeNameMap[type];
    else
        return "Unknown";
}

string Operation::to_string() const {
    string name, oprands, ret;
    name = type2str(type);
    oprands = vec2str(targets);
    ret = name + oprands;
    if (dagger) { ret += "^"; }
    return ret;
}

string OperationPack::to_string() const {
    vector<string> strs;
    for (const Operation& op : operations) {
        strs.push_back(op.to_string());
    }
    return vec2str(strs, "", "", " ");
}

string TimeSlices::to_string() const {
    vector<string> strs;
    int i = 0;
    for (const OperationPack& time_slice : time_slices) {
        strs.push_back("Time " + num2str(i)
            + " (" + time_slice.name + ") > \n  "
            + time_slice.to_string() + "\n");
        ++i;
    }
    return vec2str(strs, "", "", "\n");
}
