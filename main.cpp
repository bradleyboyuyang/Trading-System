#include <iostream>
#include <string>

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
#include "algostreamingservice.hpp"
#include "guiservice.hpp"
#include "utils.hpp"

using namespace std;


int main(int, char**){
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
    // test Price class
    Price<Bond> price1(Bond("912828M72", CUSIP, "T", 30, date(2028, Oct, 31)), 100.0, 0.01);
    cout << price1 << endl;


}
