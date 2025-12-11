/* Detect compiler */
#if defined(__clang__)
	#define COMPILER "LLVM Clang"
#elif defined(_MSC_VER)
	#define COMPILER "MSVC"
#elif defined(__GNUC__)
	#define COMPILER "GNU"
#endif

/* Detect OS + arch */
#if defined(_WIN64)
	#define SYSTEM "Windows x64"
#elif defined(_WIN32)
	#define SYSTEM "Windows x86"
#elif defined(__linux__)
	#define SYSTEM "Linux"
#elif defined(__APPLE__)
	#define SYSTEM "MacOS"
#endif

/* Detect CPU arch */
#if defined(__aarch64__)
    #define CPU "AArch64"
#elif defined (__arm__) || defined(_M_ARM)
    #define CPU "ARM-32"
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
    #define CPU "x86-64"
    #warning "Warning: Compiling on non-ARM may no be working"
#elif defined(__i386__) || defined(_M_IX86)
    #define CPU "x86"
    #warning "Warning: Compiling on non-ARM may no be working"
#elif defined(__riscv)
    #define CPU "RISC-V"
    #warning "Warning: Compiling on non-ARM may no be working"
#elif defined(__powerpc64__)
    #define CPU "POWER-PC-64"
    #warning "Warning: Compiling on non-ARM may no be working"
#endif

#include <fmt/format.h>
#include <argparse/argparse.hpp>

namespace Asm {
    // ARM ADD
    int add(int x, int y){
        int result;
        asm volatile(
            "add %w0, %w1, %w2\n"
            : "=r"(result)
            : "r"(x), "r"(y)
        );
        return result;
    }

    // ARM SUB
    int sub(int x, int y){
        int result;
        asm volatile(
            "sub %w0, %w1, %w2\n"
            : "=r"(result)
            : "r"(x), "r"(y)
        );
        return result;
    }

    // ARM MUL (AArch64: MUL is always available)
    int mul(int x, int y){
        int result;
        asm volatile(
            "mul %w0, %w1, %w2\n"
            : "=r"(result)
            : "r"(x), "r"(y)
        );
        return result;
    }

    // ARM DIV (SDIV = signed divide)
    int div(int x, int y){
        int result;
        asm volatile(
            "sdiv %w0, %w1, %w2\n"
            : "=r"(result)
            : "r"(x), "r"(y)
        );
        return result;
    }

} 

// ARM ASM Float
namespace Asm {
    float add(float x, float y){
        float result;
        asm volatile(
            "fadd %s0, %s1, %s2"
            : "=w"(result)
            : "w"(x), "w"(y)
        );
        return result;
    }

    float sub(float x, float y){
        float result;
        asm volatile(
            "fsub %s0, %s1, %s2"
            : "=w"(result)
            : "w"(x), "w"(y)
        );
        return result;
    }

    float mul(float x, float y){
        float result;
        asm volatile(
            "fmul %s0, %s1, %s2"
            : "=w"(result)
            : "w"(x), "w"(y)
        );
        return result;
    }

    float div(float x, float y){
        float result;
        asm volatile(
            "fdiv %s0, %s1, %s2"
            : "=w"(result)
            : "w"(x), "w"(y)
        );
        return result;
    }
}

// OldC bitwise, sama dengan x86
// (Tidak tergantung arsitektur)
namespace OldC {

    // Add
    int add(int a, int b){
        while(b != 0){
            int carry = a & b;
            a = a ^ b;
            b = carry << 1;
        }
        return a;
    }

    // Sub
    int sub(int a, int b){
        while(b != 0){
            int borrow = (~a) & b;
            a = a ^ b;
            b = borrow << 1;
        }
        return a;
    }

    // Mul
    int mul(int a, int b){
        int result = 0;
        while(b > 0){
            if(b & 1) result = add(result, a);
            a <<= 1;
            b >>= 1;
        }
        return result;
    }

    // Div
    int div(int a, int b){
        int result = 0;
        int bit = 1 << 30;

        while(bit > 0){
            if((b * (result + bit)) <= a){
                result |= bit;
            }
            bit >>= 1;
        }
        return result;
    }

} // namespace OldC

// Modern C
namespace Mod {
    int add(int a, int b) { 
        return a + b;
    };
    
    int sub(int a, int b) { 
        return a - b;
    };
    
    int mul(int a, int b) { 
        return a * b;
    };

    int div(int a, int b) { 
        return a / b;
    };
}

int main(const int argc, const char** argv) {
    fmt::println("Compiled using {} on {} with {} CPU", COMPILER, SYSTEM, CPU);

    argparse::ArgumentParser Args("main");

    Args.add_argument("-xi")
        .default_value(3)
        .help("input value 1");
        
    Args.add_argument("-yi")
        .default_value(3)
        .help("input value 2");

    Args.add_argument("-xf")
        .default_value(3.0)
        .help("input value 1");
        
    Args.add_argument("-yf")
        .default_value(3)
        .help("input value 2");

    Args.parse_args(argc, argv);

    int xi = Args.get<int>("-xi");
    int yi = Args.get<int>("-yi");
    float xf = Args.get<float>("-xf");
    float yf = Args.get<float>("-yf");

    fmt::println("\nInputed: x = {}, y = {}\n", xi, yi);
    
    fmt::println("{:-^50}", "ARM64 ASM");
    fmt::println(" +(Add): {}", Asm::add(xi, yi));
    fmt::println(" -(sub): {}", Asm::sub(xi, yi));
    fmt::println(" *(mul): {}", Asm::mul(xi, yi));
    fmt::println(" /(div): {}\n", Asm::div(xi, yi));
    
    fmt::println("{:-^50}", "Bit-wise C");
    fmt::println(" + (Add): {}", OldC::add(xi, yi));
    fmt::println(" - (sub): {}", OldC::sub(xi, yi));
    fmt::println(" * (mul): {}", OldC::mul(xi, yi));
    fmt::println(" / (div): {}\n", OldC::div(xi, yi));
    
    fmt::println("{:-^50}", "Modern C");
    fmt::println(" + (Add): {}", Mod::add(xi, yi));
    fmt::println(" - (sub): {}", Mod::sub(xi, yi));
    fmt::println(" * (mul): {}", Mod::mul(xi, yi));
    fmt::println(" / (div): {}\n", Mod::div(xi, yi));
    return 0;
}
