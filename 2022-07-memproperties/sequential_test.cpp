#include <vector>
#include <string>
#include <numeric>
#include "likwid.h"

static void escape(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

template <typename T>
void sum(T* a, T* b, T* out, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = a[i] + b[i];
    }
    escape(out);
    escape(a);
    escape(b);
}

template <typename T>
void div(T* a, T* b, T* out, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        out[i] = a[i] / b[i];
    }
    escape(out);
    escape(a);
    escape(b);
}

enum test_type_e {
    SUM,
    DIV,
};

std::string get_test_name(test_type_e t, int n, int repeat_count) {
    std::string result;
    
    switch(t) {
        case SUM: result = "SUM_"; break;
        case DIV: result = "DIV_"; break;
    }

    result += std::to_string(n);
    result += "_";
    result += std::to_string(repeat_count);

    return result;
}


void run_test(test_type_e test_type, int n, int repeat_count) {
    std::vector<double> in1(n+64);
    std::vector<double> in2(n+64);
    std::vector<double> out(n+64);

    std::iota(in1.begin(), in1.end(), 1.0);
    std::iota(in2.begin(), in2.end(), 2.0);

    std::string test_name = get_test_name(test_type, n, repeat_count);

    LIKWID_MARKER_START(test_name.c_str());
    switch(test_type) {
        case SUM: {
            for (int i = 0; i < repeat_count; i++) {
                sum<double>(&in1[0], &in2[0], &out[0], n);
            }
            break;
        }
        case DIV: {
            for (int i = 0; i < repeat_count; i++) {
                div<double>(&in1[0], &in2[0], &out[0], n);
            }
            break;
        }
    }
    LIKWID_MARKER_STOP(test_name.c_str());
}

int main(int argc, char** argv) {
    static constexpr int MIN_SIZE = 4 * 1024;
    static constexpr int MAX_SIZE = 256 * 1024 * 1024;

    LIKWID_MARKER_INIT;
    
    for (int size = MIN_SIZE; size <= MAX_SIZE; size *= 2) {
        int repeat_count = MAX_SIZE / size;
        run_test(test_type_e::SUM, size, repeat_count);    
        run_test(test_type_e::DIV, size, repeat_count);
    }

    LIKWID_MARKER_CLOSE;

    return 0;
}