#include <cerrno>
#include <cstdlib>
#include <iostream>

[[noreturn]] void exit_err_strerror(int);

template<typename T, typename ...Ts>
[[noreturn]] void
exit_err(T&& arg, Ts&&... args)
{
    int errcode{errno};
    std::cerr << "FAIL: " << arg;
    (std::cerr << ... << args);
    exit_err_strerror(errcode);
}

template<typename T, typename ...Ts>
[[noreturn]] void
exit_errx(T&& arg, Ts&&... args)
{
    std::cerr << "FAIL: " << arg;
    (std::cerr << ... << args);
    std::cerr << '\n';
    std::exit(EXIT_FAILURE);
}
