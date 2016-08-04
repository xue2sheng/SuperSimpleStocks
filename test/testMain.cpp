#define BOOST_TEST_MAIN
#define BOOST_TEST_STATIC_LINK
#define BOOST_TEST_MODULE testMain

#include <chrono>
#include <thread>
#include <cmath>

#include <boost/test/unit_test.hpp>
#include "version.hpp"

// software under test
#include "Trade.hpp"
#include "Stock.hpp"
#include "GBCE.hpp"

// just logging something ( --log_level=message )
BOOST_AUTO_TEST_CASE( testMain000 ) {
    std::time_t ctime_value = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
    BOOST_TEST_MESSAGE( "\nSeveral test cases at " << std::ctime( &ctime_value ) << VERSION_INFO );
    BOOST_CHECK_EQUAL(0, 0);
}

BOOST_AUTO_TEST_CASE( testMain001 ) {
    BOOST_TEST_MESSAGE(  "\nTests on 'Trade' class" );

    jpmorgan::Trade trade;

    BOOST_TEST_MESSAGE(  "   Insert a value" );
    trade.addTrade( 10, true, 123.4 );
    BOOST_CHECK_EQUAL(trade.size(), 1);

    BOOST_TEST_MESSAGE(  "   Clear all values" );
    trade.clear();
    BOOST_CHECK_EQUAL(trade.size(), 0);

    BOOST_TEST_MESSAGE(  "   Insert 10 values, one each 500 msec" );
    trade.setBorder( jpmorgan::_5sec );
    for(size_t i=1; i<11; ++i)
    {
      trade.addTrade( i, i % 2 == 0, 10.0 * i );
      std::this_thread::sleep_for( jpmorgan::_500msec );
    }
    BOOST_CHECK_EQUAL(trade.size(), 10);

    BOOST_TEST_MESSAGE(  "   Check out stock price formula" );
    BOOST_CHECK(trade.stockPrice() >= 71.1111);
    BOOST_CHECK(trade.stockPrice() <= 71.1112);

    BOOST_TEST_MESSAGE(  "   Clear only ald trades" );
    BOOST_CHECK(trade.stockPrice() >= 71.1111);
    trade.clearOldTrades();
    BOOST_CHECK_EQUAL(trade.size(), 9);
}

BOOST_AUTO_TEST_CASE( testMain002 ) {
    BOOST_TEST_MESSAGE(  "\nTests on 'Stock' class" );
    
    BOOST_TEST_MESSAGE(  "   Create Common Stock" );
    jpmorgan::Stock common_stock {"ALE", 23.0, 60.0};
    BOOST_CHECK( common_stock.isCommon() );

    common_stock.setPrice( 10.0 );
    BOOST_TEST_MESSAGE(  "   Check common dividend yield formula" );
    BOOST_CHECK(common_stock.dividendYield() >= 2.29);
    BOOST_CHECK(common_stock.dividendYield() <= 2.31);

    BOOST_TEST_MESSAGE(  "   Create Preferred Stock" );
    jpmorgan::Stock preferred_stock {"GIN", 8.0, 100.0, 2.0 / 100.0};
    BOOST_CHECK( preferred_stock.isPreferred() );

    preferred_stock.setPrice( 10.0 );
    BOOST_TEST_MESSAGE(  "   Check preferred dividend yield formula" );
    BOOST_CHECK(preferred_stock.dividendYield() >= 0.19999);
    BOOST_CHECK(preferred_stock.dividendYield() <= 0.20001);
    
    BOOST_TEST_MESSAGE(  "   Create Stock with positive dividend" );
    jpmorgan::Stock positive_stock {"JOE", 13.0, 250.0 };
    positive_stock.setPrice( 10.0 );
    BOOST_TEST_MESSAGE(  "   Check P/E Ratio formula" );
    BOOST_CHECK( positive_stock.p_e_ratio() >= 0.769230);
    BOOST_CHECK( positive_stock.p_e_ratio() <= 0.769232);

    BOOST_TEST_MESSAGE(  "   Create Stock with zero dividend" );
    jpmorgan::Stock zero_stock {"TEA",  0.0, 100.0};
    zero_stock.setPrice( 10.0 );
    BOOST_TEST_MESSAGE(  "   Check P/E Ratio formula now is infinity ");
    BOOST_CHECK( std::isinf( zero_stock.p_e_ratio() ));
}

BOOST_AUTO_TEST_CASE( testMain003 ) {
    BOOST_TEST_MESSAGE(  "\nTests on 'GBCE' class" );

    BOOST_TEST_MESSAGE(  "   Create 'Global Beverage Corporation Exchange' class " );
    jpmorgan::GlobalBeverageCorporationExchange GBCE;
    GBCE.addStock("TEA",  0.0, 100.0);
    GBCE.addStock("POP",  8.0, 100.0);
    GBCE.addStock("ALE", 23.0, 60.0);
    GBCE.addStock("GIN", 8.0, 100.0, 2.0 / 100.0); // Preferred
    GBCE.addStock("JOE", 13.0, 250.0);

    GBCE.setPrice("TEA",  10.0 );
    GBCE.setPrice("POP",  10.0 );
    GBCE.setPrice("ALE",  10.0 );
    GBCE.setPrice("GIN",  10.0 ); // Preferred
    GBCE.setPrice("JOE",  10.0 );

    BOOST_TEST_MESSAGE(  "   Check GDCE All Share Index formula" );
    BOOST_CHECK( GBCE.allShareIndex() <= 10.0001);
    BOOST_CHECK( GBCE.allShareIndex() >= 9.99999);
} 



