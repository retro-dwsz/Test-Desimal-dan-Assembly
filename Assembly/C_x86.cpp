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
    #warning "Warning: Compiling on non-x86 may no be working"
#elif defined (__arm__) || defined(_M_ARM)
    #define CPU "ARM-32"
    #warning "Warning: Compiling on non-x86 may no be working"
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
    #define CPU "x86-64"
#elif defined(__i386__) || defined(_M_IX86)
    #define CPU "x86"
#elif defined(__riscv)
    #define CPU "RISC-V"
    #warning "Warning: Compiling on non-x86 may no be working"
#elif defined(__powerpc64__)
    #define CPU "POWER-PC-64"
    #warning "Warning: Compiling on non-x86 may no be working"
#endif

#include <fmt/format.h>
#include <argparse/argparse.hpp>

// ASM int
namespace Asm {
    // x86 ADD
    int add(int x, int y) {
        int result;
        asm volatile(
            "movl %1, %%eax;"      // eax = x
            "addl %2, %%eax;"      // eax = eax + y
            : "=a"(result)         // output: result <- eax
            : "r"(x), "r"(y)       // input operands
            : "cc"                 // clobbers flags
            );
        return result;
    }

    // x86 SUB
    int sub(int x, int y) {
        int result;
        asm volatile(
            "movl %1, %%eax;"
            "subl %2, %%eax;"      // eax = x - y
            : "=a"(result)
            : "r"(x), "r"(y)
            : "cc"
            );
        return result;
    }

    // x86 MUL (signed IMUL)
    int mul(int x, int y) {
        int result;
        asm volatile(
            "movl %1, %%eax;"
            "imull %2, %%eax;"     // eax *= y
            : "=a"(result)
            : "r"(x), "r"(y)
            : "cc"
            );
        return result;
    }

    // x86 DIV (signed IDIV)
    int div(int x, int y) {
        int result;
        asm volatile(
            "movl %1, %%eax;"      // eax = x
            "cdq;"                 // sign extend eax -> edx:eax
            "idivl %2;"            // eax = (edx:eax) / y
            : "=a"(result)
            : "r"(x), "r"(y)
            : "edx", "cc"
            );
        return result;
    }

} 

// X86 Legacy ASM float
namespace LAsm {
    float add(float x, float y){
        float result;
        asm volatile(
            "flds %1\n\t"        // st(0) = x
            "flds %2\n\t"        // st(0) = y, st(1) = x
            "faddp %%st(1), %%st(0)\n\t" // st(1) = x+y, pop
            "fstps %0"           // store to result, pop
            : "=m"(result)
            : "m"(x), "m"(y)
        );
        return result;
    }

    float sub(float x, float y){
        float result;
        asm volatile(
            "flds %1\n\t"        // x
            "flds %2\n\t"        // y
            "fsubp %%st(1), %%st(0)\n\t" // st(1) = x - y
            "fstps %0"
            : "=m"(result)
            : "m"(x), "m"(y)
        );
        return result;
    }

    float mul(float x, float y){
        float result;
        asm volatile(
            "flds %1\n\t"        // x
            "flds %2\n\t"        // y
            "fmulp %%st(1), %%st(0)\n\t"
            "fstps %0"
            : "=m"(result)
            : "m"(x), "m"(y)
        );
        return result;
    }

    float div(float x, float y){
        float result;
        asm volatile(
            "flds %1\n\t"        // x
            "flds %2\n\t"        // y
            "fdivp %%st(1), %%st(0)\n\t" // x / y
            "fstps %0"
            : "=m"(result)
            : "m"(x), "m"(y)
        );
        return result;
    }

}


// X86 ASM float
namespace Asm {
    float add(float x, float y){
        float result;
        asm volatile(
            "movss %1, %%xmm0\n\t"
            "addss %2, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            : "=m"(result)
            : "m"(x), "m"(y)
            : "xmm0"
        );
        return result;
    }

    float sub(float x, float y){
        float result;
        asm volatile(
            "movss %1, %%xmm0\n\t"
            "subss %2, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            : "=m"(result)
            : "m"(x), "m"(y)
            : "xmm0"
        );
        return result;
    }

    float mul(float x, float y){
        float result;
        asm volatile(
            "movss %1, %%xmm0\n\t"
            "mulss %2, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            : "=m"(result)
            : "m"(x), "m"(y)
            : "xmm0"
        );
        return result;
    }

    float div(float x, float y){
        float result;
        asm volatile(
            "movss %1, %%xmm0\n\t"
            "divss %2, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            : "=m"(result)
            : "m"(x), "m"(y)
            : "xmm0"
        );
        return result;
    }
}

// X86 HPC ASM float
// Rule:
// vop dest, src1, src2 ; dest = src1 op src2
namespace HAsm {
    float add(float x, float y){
        float result;
        asm volatile(
            "vaddss %1, %2, %0"
            : "=x"(result)
            : "x"(x), "x"(y)
        );
        return result;
    }

    float sub(float x, float y){
        float result;
        asm volatile(
            "vsubss %1, %2, %0"
            : "=x"(result)
            : "x"(x), "x"(y)
        );
        return result;
    }

    float mul(float x, float y){
        float result;
        asm volatile(
            "vmulss %1, %2, %0"
            : "=x"(result)
            : "x"(x), "x"(y)
        );
        return result;
    }

    float div(float x, float y){
        float result;
        asm volatile(
            "vdivss %1, %2, %0"
            : "=x"(result)
            : "x"(x), "x"(y)
        );
        return result;
    }
}

// OLD C — BITWISE IMPLEMENTATION
namespace OldC {
    // BITWISE ADD
    int add(int a, int b) {
        while(b != 0) {
            int carry = a & b;    // bit yang sama (1+1)
            a = a ^ b;            // penjumlahan tanpa carry
            b = carry << 1;       // carry geser kiri
        }
        return a;
    }

    // BITWISE SUB (A - B)
    int sub(int a, int b) {
        while(b != 0) {
            int borrow = (~a) & b;
            a = a ^ b;
            b = borrow << 1;
        }
        return a;
    }

    // BITWISE MUL
    int mul(int a, int b) {
        int result = 0;
        while(b > 0) {
            if(b & 1) result = add(result, a);
            a <<= 1;
            b >>= 1;
        }
        return result;
    }

    // BITWISE DIV (integer
    int div(int a, int b) {
        int result = 0;
        int bit = 1 << 30; // start dari bit tertinggi

        while(bit > 0) {
            if((b * (result + bit)) <= a) {
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

// Modern C
namespace ModF {
    float add(float a, float b) { 
        return a + b;
    };
    
    float sub(float a, float b) { 
        return a - b;
    };
    
    float mul(float a, float b) { 
        return a * b;
    };

    float div(float a, float b) { 
        return a / b;
    };
}

int main(const int argc, const char** argv) {
    fmt::println("Compiled using {} on {} with {} CPU", COMPILER, SYSTEM, CPU);

    argparse::ArgumentParser Args("main");

    Args.add_argument("-xi")
        .default_value("3")
        .scan<'i', int>()
        .help("input int value 1");
        
    Args.add_argument("-yi")
        .default_value("3")
        .scan<'i', int>()
        .help("input int value 2");

    Args.add_argument("-xf")
        .default_value("3.14")
        .scan<'g', float>()
        .help("input float value 1");
        
    Args.add_argument("-yf")
        .default_value("2.71")
        .scan<'g', float>()
        .help("input float value 2");

    Args.parse_args(argc, argv);

    int xi = Args.get<int>("-xi");
    int yi = Args.get<int>("-yi");
    float xf = Args.get<float>("-xf");
    float yf = Args.get<float>("-yf");

    fmt::println("\nInputed: xi = {}, yi = {}\n", xi, yi);
    fmt::println("\nInputed: xf = {}, yf = {}\n", xf, yf);

    fmt::println("{:-^50}", "x86 ASM int");
    fmt::println(" + (Add): {}", Asm::add(xi, yi));
    fmt::println(" - (sub): {}", Asm::sub(xi, yi));
    fmt::println(" * (mul): {}", Asm::mul(xi, yi));
    fmt::println(" / (div): {}\n", Asm::div(xi, yi));

    fmt::println("{:-^50}", "x86 ASM float");
    fmt::println(" + (Add): {}", Asm::add(xf, yf));
    fmt::println(" - (sub): {}", Asm::sub(xf, yf));
    fmt::println(" * (mul): {}", Asm::mul(xf, yf));
    fmt::println(" / (div): {}\n", Asm::div(xf, yf));

    fmt::println("{:-^50}", "Legacy x86 ASM float");
    fmt::println(" + (Add): {}", LAsm::add(xf, yf));
    fmt::println(" - (sub): {}", LAsm::sub(xf, yf));
    fmt::println(" * (mul): {}", LAsm::mul(xf, yf));
    fmt::println(" / (div): {}\n", LAsm::div(xf, yf));

    fmt::println("{:-^50}", "x86 HPC ASM float");
    fmt::println(" + (Add): {}", HAsm::add(xf, yf));
    fmt::println(" - (sub): {}", HAsm::sub(xf, yf));
    fmt::println(" * (mul): {}", HAsm::mul(xf, yf));
    fmt::println(" / (div): {}\n", HAsm::div(xf, yf));
    
    fmt::println("{:-^50}", "Bit-wise C");
    fmt::println(" + (Add): {}", Asm::add(xi, yi));
    fmt::println(" - (sub): {}", Asm::sub(xi, yi));
    fmt::println(" * (mul): {}", Asm::mul(xi, yi));
    fmt::println(" / (div): {}\n", Asm::div(xi, yi));
    
    fmt::println("{:-^50}", "Modern C");
    fmt::println(" + (Add): {}", Mod::add(xi, yi));
    fmt::println(" - (sub): {}", Mod::sub(xi, yi));
    fmt::println(" * (mul): {}", Mod::mul(xi, yi));
    fmt::println(" / (div): {}\n", Mod::div(xi, yi));

    fmt::println("{:-^50}", "Modern C float");
    fmt::println(" + (Add): {}", ModF::add(xf, yf));
    fmt::println(" - (sub): {}", ModF::sub(xf, yf));
    fmt::println(" * (mul): {}", ModF::mul(xf, yf));
    fmt::println(" / (div): {}\n", ModF::div(xf, yf));
    return 0;
}
