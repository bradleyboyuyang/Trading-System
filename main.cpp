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
	// 1. Define data path and generate data
	const string pricePath = "../data/prices.txt";
	const string marketDataPath = "../data/marketdata.txt";
	const string tradePath = "../data/trades.txt";
	const string inquiryPath = "../data/inquiries.txt";

    log(LogLevel::INFO, "Generating price and orderbook data...");
    vector<string> bonds = {"9128283H1", "9128283L2", "912828M80", "9128283J7", "9128283F5", "912810TW8", "912810RZ3"};
    genOrderBook(bonds, pricePath, marketDataPath, 42, 1000);
    log(LogLevel::INFO, "Generating trade data...");
	genTrades(bonds, tradePath, 42);
    log(LogLevel::INFO, "Generating inquiry data...");
	genInquiries(bonds, inquiryPath, 42);
    log(LogLevel::INFO, "Generating data finished.");

    // 2. Start trading service
    log(LogLevel::INFO, "Starting trading system...");
    // 2.1 create services
    log(LogLevel::INFO, "Creating service components...");
	PricingService<Bond> pricingService;
	AlgoStreamingService<Bond> algoStreamingService;
	StreamingService<Bond> streamingService;
	MarketDataService<Bond> marketDataService;
	AlgoExecutionService<Bond> algoExecutionService;
	ExecutionService<Bond> executionService;
	TradeBookingService<Bond> tradeBookingService;
	PositionService<Bond> positionService;
	RiskService<Bond> riskService;
	GUIService<Bond> guiService;
	InquiryService<Bond> inquiryService;

	HistoricalDataService<Position<Bond>> historicalPositionService(POSITION);
	HistoricalDataService<PV01<Bond>> historicalRiskService(RISK);
	HistoricalDataService<ExecutionOrder<Bond>> historicalExecutionService(EXECUTION);
	HistoricalDataService<PriceStream<Bond>> historicalStreamingService(STREAMING);
	HistoricalDataService<Inquiry<Bond>> historicalInquiryService(INQUIRY);

	log(LogLevel::INFO, "Trading services created.");

	// 2.2 create listeners
	log(LogLevel::INFO, "Linking service listeners...");
	pricingService.AddListener(algoStreamingService.GetAlgoStreamingListener());
	pricingService.AddListener(guiService.GetGUIServiceListener());
	algoStreamingService.AddListener(streamingService.GetStreamingServiceListener());
	marketDataService.AddListener(algoExecutionService.GetAlgoExecutionServiceListener());
	algoExecutionService.AddListener(executionService.GetExecutionServiceListener());
	executionService.AddListener(tradeBookingService.GetTradeBookingServiceListener());
	tradeBookingService.AddListener(positionService.GetPositionListener());
	positionService.AddListener(riskService.GetRiskServiceListener());

	positionService.AddListener(historicalPositionService.GetHistoricalDataServiceListener());
	executionService.AddListener(historicalExecutionService.GetHistoricalDataServiceListener());
	streamingService.AddListener(historicalStreamingService.GetHistoricalDataServiceListener());
	riskService.AddListener(historicalRiskService.GetHistoricalDataServiceListener());
	inquiryService.AddListener(historicalInquiryService.GetHistoricalDataServiceListener());

	log(LogLevel::INFO, "Service listeners linked.");



	// tradeBookingService.AddListener(positionService.GetPositionListener());



	// // set output precision
	// cout << fixed << setprecision(6);


    // // test Price class
    // Price<Bond> price1(Bond("912828M72", CUSIP, "T", 30, date(2028, Oct, 31)), 100.0, 0.01);
    // cout << price1 << endl;

	// // test convertPrice function
	// string pricestr = "100-25+";
	// cout << "converted price: " << convertPrice(pricestr) << endl;

	// cout << "converted price: " << convertPrice(100.796875) << endl;

	// // test bond pricing service
    // log(LogLevel::INFO, "Processing price data...");
	// pricingService.GetConnector()->Subscribe(pricePath);
	// cout << pricingService.GetData("9128283H1") << endl;
	// cout << pricingService.GetData("9128283L2") << endl;
	// cout << pricingService.GetData("912828M80") << endl;
	// cout << pricingService.GetData("9128283J7") << endl;
	// cout << pricingService.GetData("9128283F5") << endl;
	// cout << pricingService.GetData("912810TW8") << endl;
	// cout << pricingService.GetData("912810RZ3") << endl;
	// log(LogLevel::INFO, "Price data processed");

	// // test bond market data service
	// log(LogLevel::INFO, "Processing market data...");
	// marketDataService.GetConnector()->Subscribe(marketDataPath);
	// log(LogLevel::INFO, "Market data processed");


	// // test bond trade booking service
	// log(LogLevel::INFO, "Processing trade data...");
	// tradeBookingService.GetConnector()->Subscribe(tradePath);
	// log(LogLevel::INFO, "Trade data processed");

	// // test inquiry service
	// log(LogLevel::INFO, "Processing inquiry data...");
	// inquiryService.GetConnector()->Subscribe(inquiryPath);
	// log(LogLevel::INFO, "Inquiry data processed");

}
