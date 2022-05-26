#include <iostream>
#include <sstream>
#include <string_view>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstring>

struct nothing_t {  };

template<class Is>
constexpr Is& operator>>(Is& is, nothing_t& nothing)
{
    return is;
}

template<class T> struct value_t {  };

template<class Value_t, std::size_t Size>
struct option_t
{
    using type = Value_t;

    const char long_name[Size] {  };
    const char short_naem {  };
    value_t<Value_t> v_type {  };
};

template<class Value_t, std::size_t Size>
option_t(const char (&)[Size], const char, value_t<Value_t>) -> option_t<Value_t, Size>;

template<std::size_t Size>
option_t(const char (&)[Size], const char) -> option_t<nothing_t, Size>;

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
    constexpr void match_arg(const char* arg, Do_t&& fn) {  }

    template<auto... Opts, class Matcher_t>
    constexpr void parse(int arcgc, char* argv[], Matcher_t&& match)
    {
        for (int i = 0; i < arcgc; ++i) {
            if (argv[i][0] == '-' && std::strlen(argv[i]) >= 2 && argv[i][1] != '-') {
                
                continue;
            }
            match_arg<Opts...>(argv[i], [&](auto&& opt){
                using value_type = typename std::remove_reference_t<decltype(opt)>::type;
                value_type value {  };
                if constexpr (not std::is_same_v<value_type, nothing_t>) {
                    std::stringstream ss { argv[++i] };
                    ss >> value;
                    match(std::forward<decltype(opt)>(opt), value);
                } else {
                    match(std::forward<decltype(opt)>(opt));
                }
            });
        }
    }
};

int main(int argc, char* argv[])
{
    constexpr option_t help    { "--help", 'h' };
    constexpr option_t version { "--version", 'v', value_t<float>{} };
    constexpr option_t enable  { "--enable", 'e' };
    args_parser_t args {  };
    args.parse<help, version, enable>(argc, argv, matcher_t {
            [](decltype(help)){ std::cout << "help option found!\n"; },
            [](decltype(version), auto&& v){ std::cout << "version option found!\nValue: " << v << std::endl; },
            [](decltype(enable)){ std::cout << "enable option found!\n"; }
            });
    return 0;
}
