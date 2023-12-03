/**
 * main.hpp
 * Main function to run the trading system
 *
 * @author Boyu Yang
 */

#include <iostream>
#include <string>
#include <iomanip>

#include "soa.hpp"
#include "products.hpp"
#include "marketdataservice.hpp"
#include "pricingservice.hpp"
#include "riskservice.hpp"
#include "executionservice.hpp"
#include "positionservice.hpp"
#include "inquiryservice.hpp"
#include "historicaldataservice.hpp"
#include "streamingservice.hpp"
#include "algostreamingservice.hpp"
#include "tradebookingservice.hpp"
#include "algoexecutionservice.hpp"
#include "guiservice.hpp"
#include "utils.hpp"

using namespace std;




int main(int, char**){

	// generate price and orderbook data
    log(LogLevel::INFO, "Generating price and orderbook data...");
    std::vector<std::string> bonds = {"9128283H1", "9128283L2", "912828M80", "9128283J7", "9128283F5", "912810RZ3"};
    genOrderBook(bonds, "../data/prices.txt", "../data/marketdata.txt", 42);

    // start trading system
    log(LogLevel::INFO, "Trading System starts running...");

    // create services
    log(LogLevel::INFO, "Creating trading services...");
	// PricingService<Bond> pricingService;
	// TradeBookingService<Bond> tradeBookingService;
	// PositionService<Bond> positionService;
	// RiskService<Bond> riskService;
	// MarketDataService<Bond> marketDataService;
	// AlgoExecutionService<Bond> algoExecutionService;
	// AlgoStreamingService<Bond> algoStreamingService;
	// GUIService<Bond> guiService;
	// ExecutionService<Bond> executionService;
	// StreamingService<Bond> streamingService;
	// InquiryService<Bond> inquiryService;

	// set output precision
	cout << fixed << setprecision(6);


    // test Price class
    Price<Bond> price1(Bond("912828M72", CUSIP, "T", 30, date(2028, Oct, 31)), 100.0, 0.01);
    cout << price1 << endl;

	// test convertPrice function
	string pricestr = "100-25+";
	cout << "converted price: " << convertPrice(pricestr) << endl;

	cout << "converted price: " << convertPrice(100.796875) << endl;


}
