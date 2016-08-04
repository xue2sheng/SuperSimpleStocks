#ifndef GBCE_HPP
#define GBCE_HPP

#include <iostream>
#include <map>
#include <chrono>
#include <string>
#include <exception>
#include <cmath>

#include "Exceptions.hpp"
#include "Trade.hpp"
#include "Stock.hpp"

/**** PROPER INTERFACE ******/

namespace jpmorgan {

class GlobalBeverageCorporationExchange : public std::map<std::string, Stock>
{
public:
   inline void addStock(std::string symbol, double last_dividend, double par_value, double fixed_dividend = 0.0); 
   inline void setPrice(std::string symbol, double price);
   inline double getPrice(std::string symbol) const;
 
   inline void addTrade(std::string symbol, unsigned long quantity, bool indicator);
   inline double stockPriceAndClear(std::string symbol); 
   inline double stockPrice(std::string symbol); 

   inline double dividendYield(std::string symbol) const; 
   inline double p_e_ratio(std::string symbol) const;  

   inline void clearOldTrades(); // all stocks

   // no thread-safe at all
   inline double allShareIndex(); // no const because it uses static variables to speed up

   inline friend std::ostream &operator<<(std::ostream &stream, const jpmorgan::GlobalBeverageCorporationExchange& stock);
}; 

// for debugging
std::ostream &operator<<(std::ostream &stream, const jpmorgan::GlobalBeverageCorporationExchange& gbce)
{
  for(auto element : gbce) { stream << element.second; }
  return stream;
}
 
} // namespace jpmorgan

/****** INLINE FUNCTION DEFINITIONS *********/

void jpmorgan::GlobalBeverageCorporationExchange::clearOldTrades()
{
   for(auto element : *this) { element.second.clearOldTrades(); } 
}

void jpmorgan::GlobalBeverageCorporationExchange::addStock(std::string symbol, double last_dividend, double par_value, double fixed_dividend)
{
   emplace(symbol, jpmorgan::Stock { symbol, last_dividend, par_value, fixed_dividend } );
}
 
// at least can throw when the symbol is not found or when price is negative
void jpmorgan::GlobalBeverageCorporationExchange::setPrice(std::string symbol, double price)
{
  try {
     at(symbol).setPrice(price);
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}
double jpmorgan::GlobalBeverageCorporationExchange::getPrice(std::string symbol) const
{
  try {
     return at(symbol).getPrice();
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}
   
void jpmorgan::GlobalBeverageCorporationExchange::addTrade(std::string symbol, unsigned long quantity, bool indicator)
{
  try {
     at(symbol).addTrade(quantity, indicator);
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}

double jpmorgan::GlobalBeverageCorporationExchange::stockPriceAndClear(std::string symbol) 
{
  try {
     return at(symbol).stockPriceAndClear();
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}

double jpmorgan::GlobalBeverageCorporationExchange::stockPrice(std::string symbol) 
{
  try {
     return at(symbol).stockPrice();
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}

double jpmorgan::GlobalBeverageCorporationExchange::dividendYield(std::string symbol) const
{
  try {
     return at(symbol).dividendYield();
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}
double jpmorgan::GlobalBeverageCorporationExchange::p_e_ratio(std::string symbol) const
{
  try {
     return at(symbol).p_e_ratio();
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}

// supposed thant allShareIndex will be invoked more often than individual changes at stock prices
// in order to spare pow(..., 1/n) calculation and reduce overflow issues the policy to be followed will be
// trying to 'cache' values already calculated if possible
//
// no completely thread-safe
double jpmorgan::GlobalBeverageCorporationExchange::allShareIndex()
{
   // AppleClang doesn't support Thread Local Storage 
   static /*thread_local*/ double last_calculated_value {};
   
   double exponent = ( 1.0 / size() ); 

   bool calculate = false;
   for(auto& element : *this)
   {
     // the order of this pedicate is important due to compiler optimization
     calculate = ( element.second.hasChanged( exponent ) || calculate );
   }

   if( calculate ) 
   {
      double product {1.0};
      for(const auto& element: *this)
      {
         product = ( element.second.getPricePow() * product );
      }
      last_calculated_value = product;
   }

   return last_calculated_value; 
}

#endif // GBCE_HPP
