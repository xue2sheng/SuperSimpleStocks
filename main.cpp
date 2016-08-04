#include <iostream>
#include <map>
#include <chrono>
#include <string>
#include <exception>
#include <cmath>

#include <thread>

namespace jpmorgan {

// supposed just milliseconds precision
static constexpr std::chrono::milliseconds _15min( 15 * 60 * 1000 ); 	

struct trade_data {
	unsigned long quantity {0}; // quantity of shares
	bool indicator {false}; // buy->false or sell->true
        double price {0.0};
  
	inline friend std::ostream &operator<<(std::ostream &stream, const trade_data& data);
};

// for debugging
std::ostream &operator<<(std::ostream &stream, const trade_data& data)
{
	stream << "quantity = " << data.quantity << ", ";
	stream << "indicator = " << ( data.indicator ? "sell" : "buy" ) << ", ";
	stream << "price = " << data.price;

	return stream;
}

using timestamp = std::chrono::time_point<std::chrono::system_clock>;
using trade_pair = std::pair<timestamp, trade_data>;

struct Trade : public std::multimap<timestamp, trade_data>
{
  inline Trade::iterator addTrade(unsigned long quantity, bool indicator, double price);
  inline double stockPrice() const;
  inline void clearOldTrades();
  inline double stockPriceAndClear(); 

  inline friend std::ostream &operator<<(std::ostream &stream, const Trade& trade);
};

Trade::iterator Trade::addTrade(unsigned long quantity, bool indicator, double price)
{
    return emplace_hint( 
              /* hint for the position */  end(), std::move( trade_pair{ 
	      /* timestamp */              std::chrono::system_clock::now(), std::move(
	      /* trade data */             trade_data{ quantity, indicator, price }
	   ) }) );
}

// take into account values in the past 15 minutes 
double Trade::stockPrice() const
{
   double s_trade_price_x_quantity {0.0};
   double s_quantity {0.0};

   // empty supposed means zero result
   if( this->empty() ) { return 0.0; }

   std::chrono::time_point<std::chrono::system_clock> right_now = std::chrono::system_clock::now();

   for(auto element : *this)
   {
       if( (right_now - element.first) >= _15min ) { continue; } 
       s_trade_price_x_quantity += ( element.second.price * element.second.quantity );
       s_quantity += element.second.quantity;
   }

   // denominator zero supposed means zero result
   if( 0.0 >= s_quantity ) { return 0.0; }

   // numerator 
   if( 0.0 >= s_trade_price_x_quantity ) { return 0.0; }
   
   return ( s_trade_price_x_quantity / s_quantity );
}

// clear old trades in order to save memory
void Trade::clearOldTrades()
{
   if( this->empty() ) { return; }

   std::chrono::time_point<std::chrono::system_clock> right_now = std::chrono::system_clock::now();

   for(auto element : *this)
   {
       if( (right_now - element.first) >= _15min ) { erase(element.first); }
   }
}

// clear used trades in order to save memory
double Trade::stockPriceAndClear()
{
   double result = stockPrice();
   clearOldTrades();
   return result;
}

// for debugging
std::ostream &operator<<(std::ostream &stream, const Trade& trade)
{
	for(auto element : trade) {
		std::time_t ctime_value = std::chrono::system_clock::to_time_t( element.first );
		stream << std::ctime( &ctime_value );
		stream << " " << element.second << std::endl;
	}

	return stream;
}

} // namespace jpmorgan

namespace jpmorgan {

// Although having a couple of stock times, common and preferred, seems to ask for a base class and two derived ones
// we'll take advantage of the fact that only one method is impacted by that and it's known how to choose
// the correct dividend yield formula based on fixed dividend value. This way vtable and associated penalties are spared.
 
// if the penalty of having that extra 'fixed dividend' double is greater than vtable and more complex implementation,
// then this design should be changed.
	
// As well composition with "Trade" class is chosen to simplify ctors and associated methods.

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

  inline friend std::ostream &operator<<(std::ostream &stream, const Stock& stock);

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

Stock::Stock()
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


Stock::Stock(const Stock& s)
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

Stock::Stock(Stock&& s)
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

// optimization for GBCE All Share Index
bool Stock::hasChanged( double exponent )
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
double Stock::getPricePow() const { return price_pow; } 

Stock::Stock(std::string s, double l_d, double p_v, double f_d) :
 symbol(s), last_dividend(l_d), par_value(p_v), fixed_dividend(f_d)
{
  if( symbol.empty() ) { throw unexpected_empty_string(); }
  if( 0.0 > last_dividend || 0.0 > par_value || 0.0 > fixed_dividend ) { throw unexpected_negative_value(); }
}

std::string Stock::getSymbol() const { return symbol; }
bool Stock::isCommon() const { return ( 0.0 >= fixed_dividend ); }
bool Stock::isPreferred() const { return ( 0.0 < fixed_dividend ); }

void Stock::setPrice(double p) 
{ 
  if( 0.0 > p ) { throw unexpected_negative_value(); } 
  price = p; 
}
double Stock::getPrice() const { return price; }

void Stock::setLastDividend(double d) 
{ 
  if( 0.0 > d ) { throw unexpected_negative_value(); } 
  last_dividend = d; 
}
double Stock::getLastDividend() const { return last_dividend; }

void Stock::setFixedDividend(double d) 
{ 
  if( 0.0 > d ) { throw unexpected_negative_value(); } 
  fixed_dividend = d; 
}
double Stock::getFixedDividend() const { return fixed_dividend; }

void Stock::setFixedDividendPercentage(double d) 
{ 
  if( 0.0 > d ) { throw unexpected_negative_value(); } 
  fixed_dividend = ( d / 100.0 ); 
}
double Stock::getFixedDividendPercentage() const { return ( fixed_dividend * 100.0 ); }

double Stock::dividendYield(double ticker_price) const
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
double Stock::dividendYield() const { return dividendYield(this->price); } // use price member as ticker price

double Stock::p_e_ratio(double ticker_price) const  
{
   if( 0.0 > last_dividend ) { throw unexpected_zero_denominator(); }

   return ( ticker_price / last_dividend );
}
double Stock::p_e_ratio() const { return p_e_ratio(this->price); } // use price member as ticker price 

// for debugging
std::ostream &operator<<(std::ostream &stream, const Stock& stock)
{
   stream << stock.getSymbol() << ": " << ( stock.isCommon() ? "Common" : "Preferred" ) << ", ";
   stream << "last_dividend = " << stock.last_dividend << ", ";
   stream << "par_value = " << stock.par_value << ", ";
   stream << "fixed_dividend = " << stock.fixed_dividend << ", ";
   stream << "price = " << stock.price << ", ";
   stream << "dividend yield = " << stock.dividendYield() << ", ";
   stream << "P/E ratio = " << stock.p_e_ratio() << ", ";
   stream << "stock price = " << stock.stockPrice();
   stream << std::endl;

   return stream;
}

void Stock::addTrade(unsigned long quantity, bool indicator, double p) // p->price
{
    if( 0.0 > quantity ) { throw unexpected_negative_value(); }
    if( 0.0 > p ) { throw unexpected_negative_value(); }

    trade.addTrade(quantity, indicator, price);
} 

void Stock::addTrade(unsigned long quantity, bool indicator)
{
    if( 0.0 > quantity ) { throw unexpected_negative_value(); }
    if( 0.0 > this->price ) { throw unexpected_negative_value(); }

    addTrade(quantity, indicator, this->price); // price private memeber of Stock class
} 

double Stock::stockPrice() const { return trade.stockPrice(); }
double Stock::stockPriceAndClear() { return trade.stockPriceAndClear(); }
void Stock::clearOldTrades() { trade.clearOldTrades(); }
void Stock::clear() { trade.clear(); }

} // namespace jpmorgan

namespace jpmorgan {

class stock_non_found : public std::exception
{
  virtual const char* what() const noexcept override { return "Stock Non Found"; }
};

struct GlobalBeverageCorporationExchange : public std::map<std::string, jpmorgan::Stock>
{
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

   inline friend std::ostream &operator<<(std::ostream &stream, const GlobalBeverageCorporationExchange& stock);
}; 

void GlobalBeverageCorporationExchange::clearOldTrades()
{
   for(auto element : *this) { element.second.clearOldTrades(); } 
}

void GlobalBeverageCorporationExchange::addStock(std::string symbol, double last_dividend, double par_value, double fixed_dividend)
{
   emplace(symbol, std::move( jpmorgan::Stock {symbol, last_dividend, par_value, fixed_dividend } ));
}
 
// at least can throw when the symbol is not found or when price is negative
void GlobalBeverageCorporationExchange::setPrice(std::string symbol, double price)
{
  try {
     at(symbol).setPrice(price);
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}
double GlobalBeverageCorporationExchange::getPrice(std::string symbol) const
{
  try {
     return at(symbol).getPrice();
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}
   
void GlobalBeverageCorporationExchange::addTrade(std::string symbol, unsigned long quantity, bool indicator)
{
  try {
     at(symbol).addTrade(quantity, indicator);
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}

double GlobalBeverageCorporationExchange::stockPriceAndClear(std::string symbol) 
{
  try {
     return at(symbol).stockPriceAndClear();
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}

double GlobalBeverageCorporationExchange::stockPrice(std::string symbol) 
{
  try {
     return at(symbol).stockPrice();
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}

double GlobalBeverageCorporationExchange::dividendYield(std::string symbol) const
{
  try {
     return at(symbol).dividendYield();
  } catch (const std::out_of_range& oor) {
     std::cerr << "Non found stock " << symbol << std::endl;
     throw stock_non_found();
  }
}
double GlobalBeverageCorporationExchange::p_e_ratio(std::string symbol) const
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
double GlobalBeverageCorporationExchange::allShareIndex()
{
   static thread_local double last_calculated_value {};
   
   double exponent = ( 1.0 / size() ); 

   bool calculate = false;
   for(auto element : *this)
   {
     // the order of this pedicate is important due to compiler optimization
     calculate = ( element.second.hasChanged( exponent ) || calculate );
   }

   if( calculate ) 
   {
      double product {1.0};
      for(auto element: *this)
      {
         product = ( element.second.getPricePow() * product );
      }
      last_calculated_value = product;
   }

   return last_calculated_value; 
   
}

// for debugging
std::ostream &operator<<(std::ostream &stream, const GlobalBeverageCorporationExchange& gbce)
{
  for(auto element : gbce) { stream << element.second; }
  return stream;
}

} // namespace jpmorgan

int main(int argc, char** argv)
{
  try {
        jpmorgan::GlobalBeverageCorporationExchange GBCE;
        GBCE.addStock("TEA", 0.0, 100.0);
        GBCE.addStock("POP", 8.0, 100.0);
	GBCE.addStock("ALE", 23.0, 60.0);
	GBCE.addStock("GIN", 8.0, 100.0, 2.0 / 100.0); // Preferred
	GBCE.addStock("JOE", 13.0, 250.0);

	GBCE.setPrice("TEA", 25.5);
	GBCE.setPrice("POP", 33.3);
	GBCE.setPrice("ALE", 12.3);
	GBCE.setPrice("GIN", 67.2); // Preferred
	GBCE.setPrice("JOE", 123.0);

	GBCE.addTrade("TEA", 10.0, true);
	GBCE.addTrade("POP", 39.2, false);
	GBCE.addTrade("ALE", 234.3, false);
	GBCE.addTrade("GIN", 11.2, true); // Preferred
	GBCE.addTrade("JOE", 123.1, false);

        std::cout << std::endl;	
        std::cout << GBCE << std::endl;	

        //static constexpr std::chrono::milliseconds _15min( 5 * 1000 ); 	
	std::this_thread::sleep_for( std::chrono::seconds{6} );
        GBCE.clearOldTrades();
	std::cout << std::endl;
        std::cout << GBCE;	

	return 0;

   } catch ( const std::exception& e ) {

	std::cerr << e.what() << std::endl;
	return -1;

   } catch ( ... ) {

        std::cerr << "Uncught Exception" << std::endl;
        return -2;

   }
	
}


