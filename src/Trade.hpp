#ifndef TRADE_HPP
#define TRADE_HPP

#include <iostream>
#include <chrono>
#include <string>
#include <map>

namespace jpmorgan {

/**** PROPER INTERFACE *****/

// supposed just milliseconds precision
static constexpr std::chrono::milliseconds _15min( 15 * 60 * 1000 ); 	
static constexpr std::chrono::milliseconds _5sec( 5 * 1000 ); 	
static constexpr std::chrono::milliseconds _500msec( 5 * 100 ); 	

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

class Trade : public std::multimap<timestamp, trade_data>
{
public:
  inline void addTrade(unsigned long quantity, bool indicator, double price);
  inline double stockPrice() const;
  inline void clearOldTrades();
  inline double stockPriceAndClear();

  inline friend std::ostream &operator<<(std::ostream &stream, const Trade& trade);

  inline void setBorder( std::chrono::milliseconds new_border );
private:
  std::chrono::milliseconds border { _15min }; 
};

/********* INLINE FUNCTION DEFINITIONS ***********/

// for debugging
std::ostream &operator<<(std::ostream &stream, const Trade& trade)
{
    for(const auto& element : trade) {
          std::time_t ctime_value = std::chrono::system_clock::to_time_t( element.first );
          stream << std::ctime( &ctime_value );
          stream << " " << element.second << std::endl;
    }

   return stream;
}

} // namespace jpmorgan

// basically for testing faster
void jpmorgan::Trade::setBorder( std::chrono::milliseconds new_border )
{
   border = new_border;
}

void jpmorgan::Trade::addTrade(unsigned long quantity, bool indicator, double price)
{
   emplace_hint( 
              /* hint for the position */  end(), trade_pair{ 
	      /* timestamp */              std::chrono::system_clock::now(), 
	      /* trade data */             trade_data{ quantity, indicator, price }
	    } );
}

// take into account values in the past 15 minutes 
double jpmorgan::Trade::stockPrice() const
{
   double s_trade_price_x_quantity {0.0};
   double s_quantity {0.0};

   // empty supposed means zero result
   if( this->empty() ) { return 0.0; }

   std::chrono::time_point<std::chrono::system_clock> right_now = std::chrono::system_clock::now();

   for(const auto& element : *this)
   {
       if( (right_now - element.first) >= border ) { continue; } 
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
void jpmorgan::Trade::clearOldTrades()
{
   if( this->empty() ) { return; }

   std::chrono::time_point<std::chrono::system_clock> right_now = std::chrono::system_clock::now();

   for(auto& element : *this)
   {
       if( (right_now - element.first) >= border ) { erase(element.first); }
   }
}

// clear used trades in order to save memory
double jpmorgan::Trade::stockPriceAndClear()
{
   double result = stockPrice();
   clearOldTrades();
   return result;
}

#endif // TRADE_HPP
