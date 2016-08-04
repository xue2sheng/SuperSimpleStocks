#ifndef STOCK_HPP
#define STOCK_HPP

#include <iostream>
#include <map>
#include <chrono>
#include <string>
#include <exception>
#include <cmath>

#include "Exceptions.hpp"
#include "Trade.hpp"

namespace jpmorgan {

// Although having a couple of stock times, common and preferred, seems to ask for a base class and two derived ones
// we'll take advantage of the fact that only one method is impacted by that and it's known how to choose
// the correct dividend yield formula based on fixed dividend value. This way vtable and associated penalties are spared.
 
// if the penalty of having that extra 'fixed dividend' double is greater than vtable and more complex implementation,
// then this design should be changed.
	
// As well composition with "Trade" class is chosen to simplify ctors and associated methods.

/******** PROPER INTERFACE *************/

class Stock
{
public:

  explicit Stock(std::string symbol, double last_dividend = 0.0, double par_value = 0.0, double fixed_dividend = 0.0); // possibly Preferred

  inline Stock();
  inline Stock(const Stock&);
  inline Stock(Stock&&);
  inline Stock& operator=(const Stock&) =delete;
  inline Stock&& operator=(Stock&&) =delete;

  inline std::string getSymbol() const;
  inline bool isCommon() const;
  inline bool isPreferred() const;

  // for testing
  inline void setBorder( std::chrono::milliseconds new_border );

  inline size_t getTradeSize() const;
  inline void setPrice(double price);
  inline double getPrice() const;
  inline void setLastDividend(double price);
  inline double getLastDividend() const;
  inline void setFixedDividend(double price);
  inline double getFixedDividend() const;
  inline void setFixedDividendPercentage(double price);
  inline double getFixedDividendPercentage() const;

  inline void addTrade(unsigned long quantity, bool indicator, double price);
  inline void addTrade(unsigned long quantity, bool indicator); // price private memeber of Stock class
  inline double stockPrice() const;
  inline double stockPriceAndClear(); 
  inline void clearOldTrades();
  inline void clear();

  inline double dividendYield() const; // use price member as ticker price
  inline double dividendYield(double ticker_price) const;
  inline double p_e_ratio() const; // use price member as ticker price 
  inline double p_e_ratio(double ticker_price) const;  

  inline friend std::ostream &operator<<(std::ostream &stream, const jpmorgan::Stock& stock);

  // optimization for GBCE All Share Index
  inline bool hasChanged( double exponent );
  inline double getPricePow() const; 

private:

  // only mutable at init 
  std::string symbol {}; 

  // special functions
  Trade trade {};

  // getters & setters
  double price {}; 
  double last_dividend {};
  double fixed_dividend {0.0};
  double par_value {}; 

  // optimization for GBCE All Share Index
  double previous_exponent {};
  double previous_price {};
  double price_pow {};

};

// for debugging
std::ostream &operator<<(std::ostream &stream, const jpmorgan::Stock& stock)
{
     stream << stock.getSymbol() << ": " << ( stock.isCommon() ? "Common" : "Preferred" ) << ", ";
     stream << "last_dividend = " << stock.last_dividend << ", ";
     stream << "par_value = " << stock.par_value << ", ";
     stream << "fixed_dividend = " << stock.fixed_dividend << ", ";
     stream << "price = " << stock.price << ", ";
     stream << "dividend yield = " << stock.dividendYield() << ", ";
     stream << "P/E ratio = " << stock.p_e_ratio() << ", ";
     stream << "number of trades = " << stock.trade.size() << ", ";
     stream << "stock price = " << stock.stockPrice();
     stream << std::endl;

     return stream;
}

} // namespace jpmorgan

/****** INLINE FUNTIONS DEFINITIONS **********/

jpmorgan::Stock::Stock()
{
  symbol= ""; 
  trade = Trade{};
  price = 0.0; 
  last_dividend = 0.0;
  fixed_dividend = 0.0;
  par_value = 0.0; 
  previous_exponent = 0.0;
  previous_price = 0.0;
  price_pow = 0.0;
}

jpmorgan::Stock::Stock(const Stock& s)
{
  symbol= s.symbol; 
  trade = s.trade;
  price = s.price; 
  last_dividend = s.last_dividend;
  fixed_dividend = s.fixed_dividend;
  par_value = s.par_value; 
  previous_exponent = s.previous_exponent;
  previous_price = s.previous_price;
  price_pow = s.price_pow;
}

jpmorgan::Stock::Stock(Stock&& s)
{
  symbol= s.symbol; 
  trade = s.trade;
  price = s.price; 
  last_dividend = s.last_dividend;
  fixed_dividend = s.fixed_dividend;
  par_value = s.par_value; 
  previous_exponent = s.previous_exponent;
  previous_price = s.previous_price;
  price_pow = s.price_pow;
}

void jpmorgan::Stock::setBorder( std::chrono::milliseconds new_border ) { trade.setBorder( new_border ); }

size_t jpmorgan::Stock::getTradeSize() const { return trade.size(); }

// optimization for GBCE All Share Index
bool jpmorgan::Stock::hasChanged( double exponent )
{
   bool changed = ( (exponent != previous_exponent) || (price != previous_price) );

   if( changed )
   {
      previous_price = price;
      previous_exponent = exponent;
      price_pow = std::pow( price, exponent );
   }

   return changed;
}
double jpmorgan::Stock::getPricePow() const { return price_pow; } 

jpmorgan::Stock::Stock(std::string s, double l_d, double p_v, double f_d) :
 symbol{s}, trade{}, price{}, last_dividend{l_d}, fixed_dividend{f_d}, par_value{p_v}, previous_exponent{}, previous_price{}, price_pow{}
{
  if( symbol.empty() ) { throw unexpected_empty_string(); }
  if( 0.0 > last_dividend || 0.0 > par_value || 0.0 > fixed_dividend ) { throw unexpected_negative_value(); }
}

std::string jpmorgan::Stock::getSymbol() const { return symbol; }
bool jpmorgan::Stock::isCommon() const { return ( 0.0 >= fixed_dividend ); }
bool jpmorgan::Stock::isPreferred() const { return ( 0.0 < fixed_dividend ); }

void jpmorgan::Stock::setPrice(double p) 
{ 
  if( 0.0 > p ) { throw unexpected_negative_value(); } 
  price = p; 
}
double jpmorgan::Stock::getPrice() const { return price; }

void jpmorgan::Stock::setLastDividend(double d) 
{ 
  if( 0.0 > d ) { throw unexpected_negative_value(); } 
  last_dividend = d; 
}
double jpmorgan::Stock::getLastDividend() const { return last_dividend; }

void jpmorgan::Stock::setFixedDividend(double d) 
{ 
  if( 0.0 > d ) { throw unexpected_negative_value(); } 
  fixed_dividend = d; 
}
double jpmorgan::Stock::getFixedDividend() const { return fixed_dividend; }

void jpmorgan::Stock::setFixedDividendPercentage(double d) 
{ 
  if( 0.0 > d ) { throw unexpected_negative_value(); } 
  fixed_dividend = ( d / 100.0 ); 
}
double jpmorgan::Stock::getFixedDividendPercentage() const { return ( fixed_dividend * 100.0 ); }

double jpmorgan::Stock::dividendYield(double ticker_price) const
{
   if( 0.0 > ticker_price ) { throw unexpected_zero_denominator(); }

   if( isCommon() )
   {
      return ( last_dividend / ticker_price );
   }
   else // Preferred
   {
      return ( (fixed_dividend * par_value) / ticker_price );
   }
}
double jpmorgan::Stock::dividendYield() const { return dividendYield(this->price); } // use price member as ticker price

double jpmorgan::Stock::p_e_ratio(double ticker_price) const  
{
   if( 0.0 > last_dividend ) { throw unexpected_zero_denominator(); }

   return ( ticker_price / last_dividend );
}
double jpmorgan::Stock::p_e_ratio() const { return p_e_ratio(this->price); } // use price member as ticker price 

void jpmorgan::Stock::addTrade(unsigned long quantity, bool indicator, double p) // p->price
{
    if( 0.0 > quantity ) { throw unexpected_negative_value(); }
    if( 0.0 > p ) { throw unexpected_negative_value(); }

    trade.addTrade(quantity, indicator, price);
} 

void jpmorgan::Stock::addTrade(unsigned long quantity, bool indicator)
{
    if( 0.0 > quantity ) { throw unexpected_negative_value(); }
    if( 0.0 > this->price ) { throw unexpected_negative_value(); }

    addTrade(quantity, indicator, this->price); // price private memeber of Stock class
} 

double jpmorgan::Stock::stockPrice() const { return trade.stockPrice(); }
double jpmorgan::Stock::stockPriceAndClear() { return trade.stockPriceAndClear(); }
void jpmorgan::Stock::clearOldTrades() { trade.clearOldTrades(); }
void jpmorgan::Stock::clear() { trade.clear(); }

#endif // STOCK_HPP
