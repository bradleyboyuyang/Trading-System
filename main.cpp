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
	// price data path
	const string pricePath = "../data/prices.txt";
	// market data path
	const string marketDataPath = "../data/marketdata.txt";
	// trade data path
	const string tradePath = "../data/trades.txt";
	// inquiry data path
	const string inquiryPath = "../data/inquiries.txt";


	// generate price and orderbook data
    log(LogLevel::INFO, "Generating price and orderbook data...");
    vector<string> bonds = {"9128283H1", "9128283L2", "912828M80", "9128283J7", "9128283F5", "912810TW8", "912810RZ3"};
    genOrderBook(bonds, pricePath, marketDataPath, 42);
	// generate trade data
	genTrades(bonds, tradePath, 42);
	// generate inquiry data
	genInquiries(bonds, inquiryPath, 42);

    // start trading system
    log(LogLevel::INFO, "Trading System starts running...");

    // create services
    log(LogLevel::INFO, "Creating trading services...");
	PricingService<Bond> pricingService;
	MarketDataService<Bond> marketDataService;
	TradeBookingService<Bond> tradeBookingService;
	PositionService<Bond> positionService;
	// RiskService<Bond> riskService;
	// AlgoExecutionService<Bond> algoExecutionService;
	// AlgoStreamingService<Bond> algoStreamingService;
	// GUIService<Bond> guiService;
	// ExecutionService<Bond> executionService;
	// StreamingService<Bond> streamingService;
	InquiryService<Bond> inquiryService;

	log(LogLevel::INFO, "Trading services created");

	// create listeners
	log(LogLevel::INFO, "Linking trading listeners...");
	tradeBookingService.AddListener(positionService.GetPositionListener());



	// set output precision
	cout << fixed << setprecision(6);


    // test Price class
    Price<Bond> price1(Bond("912828M72", CUSIP, "T", 30, date(2028, Oct, 31)), 100.0, 0.01);
    cout << price1 << endl;

	// test convertPrice function
	string pricestr = "100-25+";
	cout << "converted price: " << convertPrice(pricestr) << endl;

	cout << "converted price: " << convertPrice(100.796875) << endl;

	// test bond pricing service
    log(LogLevel::INFO, "Processing price data...");
	pricingService.GetConnector()->Subscribe(pricePath);
	cout << pricingService.GetData("9128283H1") << endl;
	cout << pricingService.GetData("9128283L2") << endl;
	cout << pricingService.GetData("912828M80") << endl;
	cout << pricingService.GetData("9128283J7") << endl;
	cout << pricingService.GetData("9128283F5") << endl;
	cout << pricingService.GetData("912810TW8") << endl;
	cout << pricingService.GetData("912810RZ3") << endl;
	log(LogLevel::INFO, "Price data processed");

	// test bond market data service
	log(LogLevel::INFO, "Processing market data...");
	marketDataService.GetConnector()->Subscribe(marketDataPath);
	log(LogLevel::INFO, "Market data processed");


	// test bond trade booking service
	log(LogLevel::INFO, "Processing trade data...");
	tradeBookingService.GetConnector()->Subscribe(tradePath);
	log(LogLevel::INFO, "Trade data processed");


	// test inquiry service
	log(LogLevel::INFO, "Processing inquiry data...");
	inquiryService.GetConnector()->Subscribe(inquiryPath);
	log(LogLevel::INFO, "Inquiry data processed");

}
