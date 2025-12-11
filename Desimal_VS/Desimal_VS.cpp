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

#include <fmt/base.h>
#include <fmt/format.h>
#include <argparse/argparse.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

#include <memory>

using Decimal = boost::multiprecision::cpp_dec_float_100;

template <typename T>
std::string ToFixed(const T& value, int prec) {
    std::string out;
    out.resize(prec + 100);

	auto Fmt = "{:.{}f}";
    auto it = fmt::format_to(out.begin(), fmt::runtime(Fmt), value, prec);
    out.erase(it, out.end());
    return out;
}

template <>
std::string ToFixed(const Decimal& value, int prec) {
    return value.str(prec, std::ios_base::fixed);
}

void MainStack(std::string Num, std::string Fmt, auto Prec){
	Decimal Dec(Num);
	float F = std::stof(Num);
	double D = std::stod(Num);
	long double LD = std::stold(Num);
	
	fmt::println("Decimal (boost)   : {}\n{} bytes at {}\n",
		ToFixed(Dec, Prec), sizeof(Dec), fmt::ptr(&Dec)
	);
	
	fmt::println("float             : {}\n{} bytes at {}\n",
		ToFixed(F, Prec), sizeof(F), fmt::ptr(&F)
	);
	
	fmt::println("double            : {}\n{} bytes at {}\n",
		ToFixed(D, Prec), sizeof(D), fmt::ptr(&D)
	);
	
	fmt::println("long double       : {}\n{} bytes at {}",
		ToFixed(LD, Prec), sizeof(LD), fmt::ptr(&LD)
	);
}

void MainHeap(const std::string& Num, const std::string& Fmt, const auto Prec){
	auto Dec = std::make_unique<Decimal>(Decimal(Num));
	auto F   = std::make_unique<float>(std::stof(Num));
	auto D   = std::make_unique<double>(std::stod(Num));
	auto LD  = std::make_unique<long double>(std::stold(Num));
	
	fmt::println("Decimal (boost)   : {}\n{} bytes at {}\n",
		ToFixed(*Dec, Prec), sizeof(*Dec), fmt::ptr(Dec.get())
	);
	
	fmt::println("float             : {}\n{} bytes at {}\n",
		ToFixed(*F, Prec), sizeof(*F), fmt::ptr(F.get())
	);
	
	fmt::println("double            : {}\n{} bytes at {}\n",
		ToFixed(*D, Prec), sizeof(*D), fmt::ptr(D.get())
	);
	
	fmt::println("long double       : {}\n{} bytes at {}",
		ToFixed(*LD, Prec), sizeof(*LD), fmt::ptr(LD.get())
	);
}

int main(const int argc, const char** argv) {
	fmt::println("Compiled using {} in {}\n~~~\n", COMPILER, SYSTEM);

	int Prec;

	argparse::ArgumentParser Args("main");

	// Base + Decimal, example --Base 4 --Dec 1 -> 4.1
	Args.add_argument("--Base", "-b")
		.default_value(std::string("1"))
		.help("Bagian sebelum koma");

	Args.add_argument("--Frac", "-f")
		.default_value(std::string(".1"))
		.help("Bagian setelah koma");

	// Presicion for printing,
	// example 30 will be `fmt::println("{:.30f}", f);`
	Args.add_argument("--Prec", "-p")
		.default_value(17)
		.store_into(Prec)
		.help("Precision output");

	Args.parse_args(argc, argv);

	// String for the number
	std::string Str = Args.get<std::string>("--Base") + "." + Args.get<std::string>("--Frac");
	std::string Fmt = fmt::format("{{:.{}f}}", Prec);

	fmt::println("Input String      : {}", Str);
	fmt::println("Precision Print   : {}", Prec);
	
	fmt::println("\n~~~\n");

	fmt::println("---- Stack ----");	
	MainStack(Str, Fmt, Prec);

	fmt::println("\n---- Heap ----\n");	
	MainHeap(Str, Fmt, Prec);
	
	fmt::println("\n---- End ----");	
	return 0;
}