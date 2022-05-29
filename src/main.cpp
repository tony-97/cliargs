#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>

#include <cstring>

struct nothing_t {  };

template<class Is>
constexpr Is& operator>>(Is& is, nothing_t& nothing)
{
    return is;
}

template<class Is>
constexpr Is& operator>>(Is& is, std::vector<int>& values)
{
    int v {  };
    while (is >> v) {
        values.push_back(v);
    }
    is.clear();
    is.ignore(std::numeric_limits<std::streamsize>::max());

    return is;
}

template<class T> struct value_t {  };

template<class T> constexpr value_t<T> value_v {  };

template<class Value_t, std::size_t Size>
struct option_t
{
    using type = Value_t;

    const char long_name[Size] {  };
    value_t<Value_t> v_type {  };
};

struct simple_option_t
{
    
};

struct short_option
{
    
};

struct long_option
{
    
};

template<std::size_t Size>
option_t(const char (&)[Size]) -> option_t<nothing_t, Size>;

template<class Value_t, std::size_t Size>
option_t(const char (&)[Size], value_t<Value_t>) -> option_t<Value_t, Size>;

template<class... Ts> struct matcher_t : Ts... { using Ts::operator()...; };
template<class... Ts> matcher_t(Ts...) -> matcher_t<Ts...>;

struct args_parser_t
{
    constexpr explicit args_parser_t() = default;

    template<auto Opt, auto... Opts, class Do_t>
    constexpr void match_arg(const char* arg, Do_t&& fn)
    {
        if (std::string_view(Opt.long_name) == arg) {
            fn(Opt);
        } else {
            match_arg<Opts...>(arg, std::forward<Do_t>(fn));
        }
    }

    template<class Do_t>
    constexpr void match_arg([[maybe_unused]] const char* rg, [[maybe_unused]] Do_t&& fn) {  }

    // TODO: change for use iterators
    template<auto... Opts, class Matcher_t>
    constexpr void parse(int argc, char* argv[], Matcher_t&& match)
    {
        for (int i = 0; i < argc; ++i) {
            match_arg<Opts...>(argv[i], [&](auto&& opt){
                using value_type = typename std::remove_reference_t<decltype(opt)>::type;
                value_type value {  };
                if constexpr (not std::is_same_v<value_type, nothing_t>) {
                    std::stringstream ss {  };
                    while (argv[++i] != nullptr && argv[i][0] != '-') {
                        ss << argv[i] << " ";
                    }
                    ss >> value;
                    match(std::forward<decltype(opt)>(opt), std::move(value));
                } else {
                    match(std::forward<decltype(opt)>(opt));
                }
            });
        }
    }
};

int main(int argc, char* argv[])
{
    constexpr option_t help    { "--help" };
    constexpr option_t version { "--version", value_v<float> };
    constexpr option_t values  { "--values", value_v<std::vector<int>> };

    args_parser_t args {  };
    args.parse<help, version, values>(argc, argv, matcher_t {
            [](decltype(help)) { std::cout << "help option found!\n"; },
            [](decltype(version), auto&& v) {
                std::cout << "version option found!\nValue: " << v << std::endl;
            },
            [](decltype(values), auto&& v) { 
                std::cout << "values option found!\n";
                for (auto&& i : v ) { std::cout << i << " "; }
                std::cout << std::endl;
            }
            });
    return 0;
}
