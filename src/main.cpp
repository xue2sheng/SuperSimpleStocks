#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <exception>
#include <random>

#include "version.hpp"
#include "Exceptions.hpp"
#include "Trade.hpp"
#include "Stock.hpp"
#include "GBCE.hpp"

template<typename T>
class RandomNumber {
 public:
  RandomNumber(T low, T high)
   : r(std::bind( std::uniform_real_distribution<>(low,high), 
		  std::default_random_engine( std::chrono::system_clock::now().time_since_epoch().count() )
     )) {}
  T operator()() { return r(); }
 private:
  std::function<T()> r;
};

int main(int argc, char** argv)
{
  try {
        jpmorgan::GlobalBeverageCorporationExchange GBCE;
        GBCE.addStock("TEA",  0.0, 100.0);
        GBCE.addStock("POP",  8.0, 100.0);
	GBCE.addStock("ALE", 23.0, 60.0);
	GBCE.addStock("GIN", 8.0, 100.0, 2.0 / 100.0); // Preferred
	GBCE.addStock("JOE", 13.0, 250.0);

        RandomNumber<double> price_generator(0.0, 200.0);
	RandomNumber<unsigned long> quantity_generator(0, 1000);
	RandomNumber<unsigned short> bool_generator(0, 3);

	// initial prices
      	GBCE.setPrice("TEA",  price_generator() );
        GBCE.setPrice("POP",  price_generator() );
	GBCE.setPrice("ALE",  price_generator() );
	GBCE.setPrice("GIN",  price_generator() ); // Preferred
	GBCE.setPrice("JOE",  price_generator() );

        std::cout << std::endl;

	for(auto& element : GBCE) {

	   // for testing we'd better not wait for 15 minutes
	   // 5 secs can do the trick to choose latest trades
           element.second.setBorder( jpmorgan::_5sec );


	   std::time_t ctime_value = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
	   element.second.setPrice ( price_generator() );

	   std::cout << std::ctime( &ctime_value ) << " GBCE All Share Index = " << GBCE.allShareIndex() << std::endl;
	   
	   for(size_t i=0; i<10; ++i) { 	
	    element.second.addTrade ( quantity_generator(), bool_generator() < 1 );
	   }

	   // wait just a sec and only the last 5 secs trades will be used
	   std::this_thread::sleep_for( jpmorgan::_500msec );
	}

        std::cout << std::endl;	
	GBCE.clearOldTrades();
        std::cout << GBCE << std::endl;	

	return 0;

   } catch ( const std::exception& e ) {

	std::cerr << e.what() << std::endl;
	return -1;

   } catch ( ... ) {

        std::cerr << "Uncught Exception" << std::endl;
        return -2;

   }
	
}


