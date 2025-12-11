#include <fmt/format.h>
#include <argparse/argparse.hpp>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <cmath>

using str = std::string;
constexpr char Charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
constexpr int BASE = 36;

/* Detect OS + arch */
#if defined(_WIN64)
	#define SYSTEM "Windows x64"
    #include <windows.h>

    uint8_t GetCPUC(){
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors;
    }

#elif defined(_WIN32)
	#define SYSTEM "Windows x86"
    #include <windows.h>

    uint8_t GetCPUC(){
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors;
    }

#elif defined(__linux__)
	#define SYSTEM "Linux"
    #include <unistd.h>

    uint8_t GetCPUC(){
        long numCPU = sysconf(_SC_NPROCESSORS_ONLN);
        if (numCPU < 1) {
            // Handle error or use a default of 1
            numCPU = 1;
        }
        return numCPU;
    }

#elif defined(__APPLE__)
	#define SYSTEM "MacOS"
    #include <sys/sysctl.h>
    #include <mach/mach.h>

    int GetCPUC() {
        int count = 0;
        size_t count_len = sizeof(count);

        if (sysctlbyname("hw.logicalcpu", &count, &count_len, NULL, 0) == 0) {
            if (count < 1) {
                count = 1;
            }
        } else {
            std::cerr << "Error getting CPU count via sysctlbyname. Defaulting to 1." << std::endl;
            count = 1;
        }

        return count;
    }
#endif

#if defined(__aarch64__)
    #define CPU "AArch64"
#elif defined (__arm__) || defined(_M_ARM)
    #define CPU "ARM-32"
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
    #define CPU "x86-64"
#elif defined(__i386__) || defined(_M_IX86)
    #define CPU "x86"
#elif defined(__riscv)
    #define CPU "RISC-V"
#elif defined(__powerpc64__)
    #define CPU "POWER-PC-64"
#endif

str ToBase36(uint64_t x, int width) {
    str out(width, '0');
    for (int i = width - 1; i >= 0; i--) {
        out[i] = Charset[x % BASE];
        x /= BASE;
    }
    return out;
}

// Convert integer → zero padded s
str PadNum(uint64_t x, int width) {
    auto s = fmt::format("{}", x);
    if ((int)s.size() < width) {
        return fmt::format("{:0>{}}", x, width);
    }
    return s;
}

uint64_t MaxSearch(int digits) {
    return static_cast<uint64_t>(std::pow(BASE, digits));
}

// Single-thread brute force
void Single(const str& target) {
    const int digits = target.size();
    uint64_t maxN = MaxSearch(digits);

    for (uint64_t i = 0; i < maxN; i++) {
        if (ToBase36(i, digits) == target)
            return;
    }
}

// Multi-thread brute force dengan std::thread
void Multi(const str& target, int Threads) {
    const int digits = target.size();
    uint64_t maxN = MaxSearch(digits);

    std::atomic<bool> Found = false;
    uint64_t chunk = maxN / Threads;

    auto worker = [&](int id) {
        uint64_t begin = id * chunk;
        uint64_t end   = (id == Threads - 1) ? maxN : (id + 1) * chunk;

        for (uint64_t i = begin; i < end && !Found.load(); i++) {
            if (ToBase36(i, digits) == target) {
                Found.store(true);
                return;
            }
        }
    };

    std::vector<std::thread> pool;
    pool.reserve(Threads);

    for (int t = 0; t < Threads; t++)
        pool.emplace_back(worker, t);

    for (auto& th : pool)
        th.join();
}

// Multi-thread brute force dengan std::jthread
void MultiJ(const str& target, int Threads) {
    const int digits = target.size();
    uint64_t maxN = MaxSearch(digits);

    std::stop_source stop;
    auto token = stop.get_token();

    uint64_t chunk = maxN / Threads;

    auto worker = [&](std::stop_token st, int id) {
        uint64_t begin = id * chunk;
        uint64_t end   = (id == Threads - 1) ? maxN : (id + 1) * chunk;

        for (uint64_t i = begin; i < end && !st.stop_requested(); i++) {
            if (ToBase36(i, digits) == target) {
                stop.request_stop();
                return;
            }
        }
    };

    std::vector<std::jthread> pool;
    pool.reserve(Threads);

    for (int t = 0; t < Threads; t++)
        pool.emplace_back(worker, t);
}


// Main
int main(int argc, char** argv) {
    
    argparse::ArgumentParser Args("Brute");
    unsigned int cpu_count = GetCPUC();

    fmt::println("Running on {} using {} CPU ({} threads)\n", SYSTEM, CPU, cpu_count);

    Args.add_argument("-n", "--Num")
        .default_value(str("AB12"))
        .help("Target (0-9, A-Z)");

    Args.add_argument("-m", "--Mode")
        .default_value(str("S"))
        .help("S = Single | M<N> = Multi-thread | MJ<N> = Multi jthread");

    Args.parse_args(argc, argv);

    str Num  = Args.get<str>("--Num");
    str Mode = Args.get<str>("--Mode");

    int Threads = 1;
    bool useJ = false;

    if (Mode == "S") Threads = 1;
    else if (Mode.starts_with("MJ")) {
        useJ = true;
        Threads = std::stoi(Mode.substr(2));
    }
    else if (Mode.starts_with("M")) {
        Threads = std::stoi(Mode.substr(1));
    }

    if(Threads > cpu_count){
        fmt::println("Warning: Using {} more threads than available threads ({})\n", Threads-cpu_count, cpu_count);
    } else if(Threads == cpu_count){
        fmt::println("Warning: Using all available threads\n");
    }

    fmt::println("Target: {}", Num);
    fmt::println("Threads: {}", Threads);

    auto start = std::chrono::high_resolution_clock::now();

    if (Threads == 1)
        Single(Num);
    else if (useJ)
        MultiJ(Num, Threads);
    else
        Multi(Num, Threads);

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    fmt::println("Done in {} ms", ms.count());
}

