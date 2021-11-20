#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <complex>
#include <map>
#include <algorithm>
#include "logger.h"
using namespace std;

inline constexpr size_t pow2(unsigned int n) { return (1ull) << (n); }
inline constexpr bool get_digit(size_t n, int digit) { return (n >> digit) % 2; }
inline constexpr bool get_digit_reverse(size_t n, int digit, int maxdigit) { return (n >> (maxdigit-digit-1)) % 2; }
inline constexpr size_t log2(size_t n) {
    size_t ret = 0;
    while (n > 1) {
        ret++;
        n /= 2;
    }
    return ret;
}

vector<bool> calc_pos(int pos, int layer);
vector<size_t> get_nodes_in_layer(int layer);
void test_get_nodes_in_layer();
string pos2str(int pos, int layer);

struct Qubit {
    int ZEROONE; // ZERO = 0, ONE = 1, OTHER = 3
    // complex<double> ZERO;
    // complex<double> ONE;
    Qubit() {
        // ZERO = 1;
        // ONE = 0;
        ZEROONE = 0;
    }
    Qubit(const Qubit& old) {
        ZEROONE = old.ZEROONE;
    }
    inline bool state() const { return ZEROONE == 1; }
    inline bool zero() const { return ZEROONE == 0; }
    inline bool one() const { return ZEROONE == 1; }
    inline int data() const { return ZEROONE; }
    inline string to_string() const {
        if (ZEROONE == 0) return "|0>";
        if (ZEROONE == 1) return "|1>";
        return "|?>";
        // stringstream sst;
        // sst << "(" << ZERO << ")" << "|0>"
        //     << "(" << ONE << ")" << "|1>";
        // return sst.str();
    }
    inline void NOT() {
        // swap(ZERO, ONE);
        // if (ZEROONE == 0) { ZEROONE = 1; return; }
        // if (ZEROONE == 1) { ZEROONE = 0; return; }
        // if (ZEROONE == 3) { return; }
        ZEROONE = (ZEROONE + 1) % 2;
    }
    inline void CNOT(bool c) {
        if (c) { NOT(); }
    }
    inline bool is_same(const Qubit& other) const {
        return ZEROONE == other.ZEROONE;
    }
    inline bool less_than(const Qubit& other) const {
        return ZEROONE < other.ZEROONE;
    }
    inline bool operator==(const Qubit& other) const {
        return is_same(other);
    }
    inline bool operator<(const Qubit& other) const {
        return less_than(other);
    }
};

// struct Qubit {
//     bool zeroone; // false: 0, true: 1
//     Qubit() {
//         zeroone = false;
//     }
// };

struct Qutrit {
    
    int WLR; // W = 0, L = 1, R = 2, OTHER = 3
    //complex<double> W;
    //complex<double> L;
    //complex<double> R;
    Qutrit() {
        //W = 1;
        //L = 0;
        //R = 0;
        WLR = 0;
    }
    inline const int data() const { return WLR; }
    
    inline string to_string() {
        if (WLR == 0) return "|W>";
        if (WLR == 1) return "|L>";
        if (WLR == 2) return "|R>";
        
        return "|?>";
        // stringstream sst;
        // sst << "(" << W << ")" << "|W>"
        //     << "(" << L << ")" << "|L>"
        //     << "(" << R << ")" << "|R>";
        // return sst.str();
    }
    // inline bool is_same(Qutrit& other) {
    //     return WLR == other.WLR;
    // }
};

// template<typename AddrTy = Qubit, typename DataTy = Qutrit>
struct Nodebb {
    Qubit addr;
    Qubit data;
    //int layer;
    //int pos;

    /* copy constructor. Class Node is always copyable. */
    Nodebb() { }
    Nodebb(const Nodebb& old) {
        addr = old.addr;
        data = old.data;
    }

    inline string to_string() const {
        string ret;
        ret += "A: "; ret += addr.to_string();
        ret += " ";
        ret += "D: "; ret += data.to_string();
        return ret;
    }
    
    inline bool operator==(const Nodebb& other) const {
        return tie(addr, data) == tie(other.addr, other.data);
    }

    inline bool operator<(const Nodebb& other) const {
        return tie(addr, data) < tie(other.addr, other.data);
    }
};

// using NodeQutrit = Node<Qubit, Qutrit>;
// using node_t = NodeQutrit;
// using node_t = Node<Qubit, Qubit>;

using node_t = Nodebb;

struct Tree {    
private:
    vector<node_t> treenodes;
    
    int n_layer;

public:    
    // In: number of layers
    // To construct a tree. The n_layer is the tree depth except the root.
    Tree() : n_layer(-1) {}
    Tree(int n_layer);

    // copy constructor
    Tree(const Tree&);

    inline int get_nlayer() const { return n_layer; }
    
    bool is_same(const Tree& other) const;
    inline bool operator==(const Tree& other) const { return is_same(other); }
    bool less_than(const Tree& other) const;
    inline bool operator<(const Tree& other) const { return less_than(other); }

    // Return: the root of the tree
    inline node_t* root() { return &treenodes[0]; }
    inline size_t idx_of(node_t* node) const { return node - &treenodes[0]; }
    
    const node_t* get_treenode(size_t idx) const;
    node_t* access_treenode(size_t idx);
    vector<const node_t*> get_treenodes(vector<size_t> idxs) const;
    vector<const node_t*> get_treenodes(int layer) const;
    
	size_t left_of(size_t idx) const;
    node_t* left_of(node_t* node);
	size_t right_of(size_t idx) const;
    node_t* right_of(node_t* node);
    size_t parent_of(size_t idx) const;
    node_t* parent_of(node_t* node);

    string to_string() const;
};

enum class OperationType {
    Begin,
    
    ControlSwap,
    HadamardData,
    SwapInternal,
    FirstCopy,
    FetchData,

    // Noise Op
    SetZero,
    Damp,
    BitFlip,
    PhaseFlip,
    
    End
};

string type2str(OperationType type);

struct Operation {
    using target_t = vector<size_t>;
    
    OperationType type;
    target_t targets;
    bool dagger = false;    

    Operation(OperationType type_, target_t targets_)
        :type(type_), targets(targets_)
    { }

    Operation(const Operation& oldop) = default;
    // {
    //     type = oldop.type;
    //     targets.assign(oldop.targets.begin(), oldop.targets.end());
    // }

    inline target_t get_targets() const { return targets; }
    inline Operation reverse() {
        Operation m(*this);
        m.dagger = !m.dagger;
        return m;
    }
    string to_string() const;
};

struct OperationPack {
    list<Operation> operations;
    string name;
    
    inline OperationPack reverse() {
        OperationPack ret;
        for (auto iter = operations.rbegin(); iter != operations.rend(); ++iter) {
            ret.operations.push_back(iter->reverse());
        }
        ret.name = name + "^";
        return ret;
    }
    inline bool empty() { return operations.size() == 0; }
    inline void set_name(string s) { name = s; }
    inline void append(Operation op) { operations.push_back(op); }
    inline void append(OperationPack ops) {
        for (auto &op : ops.operations) {
            append(op);
        }
        name += "->";
        name += ops.name;
    }
    string to_string() const;
};

struct TimeSlices {
    vector<OperationPack> time_slices;

    inline void clear() {
        time_slices.clear();
    }
    inline void append(OperationPack op) {
        time_slices.push_back(op);
    }
    inline void append(TimeSlices ts) {
        for (auto& tslice : ts.time_slices) {
            append(tslice);
        }
    }
    inline TimeSlices reverse() {
        TimeSlices ret;
        for (auto iter = time_slices.rbegin(); iter != time_slices.rend(); ++iter) {
            ret.time_slices.push_back(iter->reverse());
        }
        return ret;
    }

    string to_string() const;
};
