# TESTS

Just preliminary *unit tests* based on **Boost libraries** and connected with **CMake** testing facilities. 

Pay close attention to the fact the *VERSION* of the software was generated automatically basically from *GIT* hash code. That *VERSION_INFO* can be embedded into binaries (executables & libraries) and extracted with the command *strings*. 

      strings /usr/local/SuperSimpleStocks | grep VERSION_INFO
      VERSION_INFO: git@github.com:xue2sheng/SuperSimpleStocks.git master 067c08b [Thu Aug  4 22:29:00 IST 2016] user@MacBookPro.local

As well that info might be included into the documentation and let us link binaries, documentation and repositories with a simple automatically-generated *hash code*.

## Classes tested.

Very humble attempt to cover just some of the possible user cases associados. As well a minimum effort has been allocated to check out limit cases ( zero denominator, obsolete trade records, ....).

```
Running 4 test cases...

Several test cases at Thu Aug  4 22:29:07 2016
VERSION_INFO: git@github.com:xue2sheng/SuperSimpleStocks.git master 067c08b [Thu Aug  4 22:29:00 IST 2016] user@MacBookPro.local

Tests on 'Trade' class
   Insert a value
   Clear all values
   Insert 10 values, one each 500 msec
   Check out stock price formula
   Clear only ald trades

Tests on 'Stock' class
   Create Common Stock
   Check common dividend yield formula
   Create Preferred Stock
   Check preferred dividend yield formula
   Create Stock with positive dividend
   Check P/E Ratio formula
   Create Stock with zero dividend
   Check P/E Ratio formula now is infinity 

Tests on 'GBCE' class
   Create 'Global Beverage Corporation Exchange' class 
   Check GDCE All Share Index formula

*** No errors detected
```
