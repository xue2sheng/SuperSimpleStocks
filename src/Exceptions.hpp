#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <exception>

class unexpected_negative_value : public std::exception
{
  virtual const char* what() const noexcept override { return "Unexpected Negative Value"; }
};

class unexpected_zero_denominator : public std::exception
{
  virtual const char* what() const noexcept override { return "Unexpected Zero Denominator"; }
};

class unexpected_empty_string : public std::exception
{
  virtual const char* what() const noexcept override { return "Unexpected Empty String"; }
};

class stock_non_found : public std::exception
{
  virtual const char* what() const noexcept override { return "Stock Non Found"; }
};

#endif // EXCEPTIONS_HPP
