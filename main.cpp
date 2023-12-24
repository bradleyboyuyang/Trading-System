/**
 * main.hpp
 * Main function to run the trading system
 *
 * @author Boyu Yang
 */

#include <iostream>
#include <string>
#include <iomanip>
#include <filesystem>
#include <thread>

#include "headers/soa.hpp"
#include "headers/products.hpp"
#include "headers/marketdataservice.hpp"
#include "headers/pricingservice.hpp"
#include "headers/riskservice.hpp"
#include "headers/executionservice.hpp"
#include "headers/positionservice.hpp"
#include "headers/inquiryservice.hpp"
#include "headers/historicaldataservice.hpp"
#include "headers/streamingservice.hpp"
#include "headers/algostreamingservice.hpp"
#include "headers/tradebookingservice.hpp"
#include "headers/algoexecutionservice.hpp"
#include "headers/guiservice.hpp"
#include "headers/utils.hpp"

using namespace std;

int main(int, char**){

	// 1. define data path and generate data
	// 1.1 create folders that store data and results
	string dataPath = "../data";
	if (filesystem::exists(dataPath)) {
		filesystem::remove_all(dataPath);
	}
	filesystem::create_directory(dataPath);

	string resPath = "../res";
	if (filesystem::exists(resPath)) {
		filesystem::remove_all(resPath);
	}
	filesystem::create_directory(resPath);

	// 1.2 define data path
	const string pricePath = "../data/prices.txt";
	const string marketDataPath = "../data/marketdata.txt";
	const string tradePath = "../data/trades.txt";
	const string inquiryPath = "../data/inquiries.txt";
	// 1.3 generate data
    log(LogLevel::INFO, "Generating price and orderbook data...");
	// tickers
    vector<string> bonds = {"9128283H1", "9128283L2", "912828M80", "9128283J7", "9128283F5", "912810TW8", "912810RZ3"};
	// generate price and orderbook data (specify random seed and number of data points)
    genOrderBook(bonds, pricePath, marketDataPath, 39373, 5000);
    log(LogLevel::INFO, "Generating trade data...");
	genTrades(bonds, tradePath, 39373);
    log(LogLevel::INFO, "Generating inquiry data...");
	genInquiries(bonds, inquiryPath, 39373);
    log(LogLevel::INFO, "Generating data finished.");

    // 2. start trading service
    log(LogLevel::INFO, "Starting trading system...");
    // 2.1 create four servers with host adn different ports
    log(LogLevel::INFO, "Initializing service components...");
	PricingService<Bond> pricingService("localhost", "3000");
	MarketDataService<Bond> marketDataService("localhost", "3001");
	TradeBookingService<Bond> tradeBookingService("localhost", "3002");
	InquiryService<Bond> inquiryService("localhost", "3003");

	AlgoStreamingService<Bond> algoStreamingService;
	StreamingService<Bond> streamingService;
	AlgoExecutionService<Bond> algoExecutionService;
	ExecutionService<Bond> executionService;
	PositionService<Bond> positionService;
	RiskService<Bond> riskService;
	GUIService<Bond> guiService;

	HistoricalDataService<Position<Bond>> historicalPositionService(POSITION);
	HistoricalDataService<PV01<Bond>> historicalRiskService(RISK);
	HistoricalDataService<ExecutionOrder<Bond>> historicalExecutionService(EXECUTION);
	HistoricalDataService<PriceStream<Bond>> historicalStreamingService(STREAMING);
	HistoricalDataService<Inquiry<Bond>> historicalInquiryService(INQUIRY);
	log(LogLevel::INFO, "Trading service initialized.");

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

	// 3. start trading system data flows
	cout << fixed << setprecision(6);

	// 3.1 price data -> pricing service -> algo streaming service -> streaming service -> historical data service
	// another data flow: pricing service -> GUI service -> GUI data output
	pricingService.GetConnector()->Subscribe(); // subscribe from the socket

	// 3.2 orderbook data -> market data service -> algo execution service -> execution service -> historical data service
	// another data flow: execution service -> trade booking service -> position service -> risk service -> historical data service
	log(LogLevel::INFO, "Processing market data...");
	marketDataService.GetConnector()->Subscribe();
	log(LogLevel::INFO, "Market data processed");

	// 3.3 trade data -> trade booking service -> position service -> risk service -> historical data service
	log(LogLevel::INFO, "Processing trade data...");
	tradeBookingService.GetConnector()->Subscribe();
	log(LogLevel::INFO, "Trade data processed");

	// 3.4 inquiry data -> inquiry service -> historical data service
	log(LogLevel::INFO, "Processing inquiry data...");
	inquiryService.GetConnector()->Subscribe();
	log(LogLevel::INFO, "Inquiry data processed");
}
