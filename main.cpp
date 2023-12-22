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
    genOrderBook(bonds, pricePath, marketDataPath, 39373, 10000);
    log(LogLevel::INFO, "Generating trade data...");
	genTrades(bonds, tradePath, 39373);
    log(LogLevel::INFO, "Generating inquiry data...");
	genInquiries(bonds, inquiryPath, 39373);
    log(LogLevel::INFO, "Generating data finished.");

    // 2. Start trading service
    log(LogLevel::INFO, "Starting trading system...");
    // 2.1 create services
    log(LogLevel::INFO, "Initializing service components...");
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
	log(LogLevel::INFO, "Trading services initialized.");

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

	// 3. Start trading system data flows
	// 3.1 price data -> pricing service -> algo streaming service -> streaming service -> historical data service
	// another data flow: pricing service -> GUI service -> GUI data output
	cout << fixed << setprecision(6);
    log(LogLevel::INFO, "Processing price data...");
	pricingService.GetConnector()->Subscribe(pricePath);
	log(LogLevel::INFO, "Price data processed");

	// 3.2 orderbook data -> market data service -> algo execution service -> execution service -> historical data service
	// another data flow: execution service -> trade booking service -> position service -> risk service -> historical data service
	log(LogLevel::INFO, "Processing market data...");
	marketDataService.GetConnector()->Subscribe(marketDataPath);
	log(LogLevel::INFO, "Market data processed");

	// 3.3 trade data -> trade booking service -> position service -> risk service -> historical data service
	log(LogLevel::INFO, "Processing trade data...");
	tradeBookingService.GetConnector()->Subscribe(tradePath);
	log(LogLevel::INFO, "Trade data processed");

	// 3.4 inquiry data -> inquiry service -> historical data service
	log(LogLevel::INFO, "Processing inquiry data...");
	inquiryService.GetConnector()->Subscribe(inquiryPath);
	log(LogLevel::INFO, "Inquiry data processed");

}
