/*! @file overloaded.hpp
    @brief Helper templates for std::visit.
*/

#ifndef BOALANG_OVERLOADED_HPP
#define BOALANG_OVERLOADED_HPP

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

#endif  // BOALANG_OVERLOADED_HPP
