/**
 * utils.hpp
 * A collection of utility functions
 *
 * @author Boyu Yang
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <fstream>
#include <random>

#include "products.hpp"

using namespace std;

// Color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"


// get the system time and return in milliseconds format (e.g. 2023-12-23 22:42:44.260)
std::string getTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // change the system time to local time
    std::tm now_tm = *std::localtime(&now_c);

    // use stringstream to format the time
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");

    // get the milliseconds separately
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    ss << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();

    return ss.str();
}

// change time point format to string 
std::string getTime(std::chrono::system_clock::time_point now) {
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    ss << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();
    return ss.str();
}


enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

// log messages with different levels (colors)
void log(LogLevel level, const string& message) {
    string levelStr;
    string color;
    switch (level) {
        case LogLevel::INFO:
            levelStr = "INFO";
            color = GREEN;
            break;
        case LogLevel::WARNING:
            levelStr = "WARNING";
            color = YELLOW;
            break;
        case LogLevel::ERROR:
            levelStr = "ERROR";
            color = RED;
            break;
    }
    cout << color << getTime() << " [" << levelStr << "] " << message << RESET << endl;
}

// create Bond objects (2Y, 3Y, 5Y, 7Y, 10Y, 20Y, 30Y) from CUSIP
Bond createBond(string cusip)
{
	Bond bond;
	if (cusip == "9128283H1") bond = Bond("9128283H1", CUSIP, "US2Y", 0.01750, from_string("2019/11/30"));
	if (cusip == "9128283L2") bond = Bond("9128283L2", CUSIP, "US3Y", 0.01875, from_string("2020/12/15"));
	if (cusip == "912828M80") bond = Bond("912828M80", CUSIP, "US5Y", 0.02000, from_string("2022/11/30"));
	if (cusip == "9128283J7") bond = Bond("9128283J7", CUSIP, "US7Y", 0.02125, from_string("2024/11/30"));
	if (cusip == "9128283F5") bond = Bond("9128283F5", CUSIP, "US10Y", 0.02250, from_string("2027/12/15"));
	if (cusip == "912810RZ3") bond = Bond("912810RZ3", CUSIP, "US30Y", 0.02750, from_string("2047/12/15"));
	return bond;
}


// change US treasury prices from fractional notation to decimal notation
double convertPrice(const string& priceStr) {
    // if the price is in decimal notation, return it directly
    if (priceStr.find_first_of('-') == string::npos) {
        return stod(priceStr);
    }

    // if the price is in fractional notation, convert it to decimal notation
    // first, split the string into two parts
    size_t pos = priceStr.find_first_of('-');
    string str1 = priceStr.substr(0, pos);
    string str2 = priceStr.substr(pos+1, 2);
    string str3 = priceStr.substr(pos+3, 1);
    if (str3 == "+") {
        str3 = "4";
    }

    double res = stod(str1) + stod(str2)*1.0 / 32.0 + stod(str3)*1.0 / 256.0;
    return res;
}

// change prices from decimal notation to fractional notation
string convertPrice(double price) {
    int intPart = floor(price);
    double fraction = price - intPart;
    int xy = floor(fraction * 32);
    int z = static_cast<int>((fraction*256)) % 8;
    // be careful about corner cases
    return to_string(intPart) + "-" + (xy < 10 ? "0" : "") + std::to_string(xy) + (z == 4 ? "+" : std::to_string(z));

}

// generate oscillating spread between 1/64 and 1/128
double genRandomSpread(std::mt19937& gen) {
    std::uniform_int_distribution<> dis(2, 4); 
    return static_cast<double>(dis(gen)) / 256.0;
}

/**
 * 1. Generate bond prices that oscillate between 99 and 101 and write to prices.txt
 * 2. Generate order book data with fivel levels of bids and offers and write to marketdata.txt
 */
void genOrderBook(const std::vector<std::string>& bonds, const std::string& priceFile, const std::string& orderbookFile, long long seed) {
    std::ofstream pFile(priceFile);
    std::ofstream oFile(orderbookFile);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> ms_dist(1, 9); // simulate milliseconds increments

    // price file format: Timestamp, CUSIP, Bid, Ask
    pFile << "Timestamp,CUSIP,Bid,Ask\n";
    
    // orderbook file format: Timestamp, CUSIP, Bid1, BidSize1, Ask1, AskSize1, Bid2, BidSize2, Ask2, AskSize2, Bid3, BidSize3, Ask3, AskSize3, Bid4, BidSize4, Ask4, AskSize4, Bid5, BidSize5, Ask5, AskSize5
    oFile << "Timestamp,CUSIP,Bid1,BidSize1,Ask1,AskSize1,Bid2,BidSize2,Ask2,AskSize2,Bid3,BidSize3,Ask3,AskSize3,Bid4,BidSize4,Ask4,AskSize4,Bid5,BidSize5,Ask5,AskSize5\n";

    for (const auto& bond : bonds) {
        double midPrice = 99.00;
        bool priceIncreasing = true;
        bool spreadIncreasing = true;
        double fixSpread = 1.0/128.0;
        auto curTime = std::chrono::system_clock::now();
        string timestamp;

        // number of data points
        for (int i = 0; i < 1000; ++i) {

            // generate price data
            double randomSpread = genRandomSpread(gen);
            curTime += std::chrono::milliseconds(ms_dist(gen));
            timestamp = getTime(curTime);

            double randomBid = midPrice - randomSpread / 2;
            double randomAsk = midPrice + randomSpread / 2;

            pFile << timestamp << "," << bond << "," << convertPrice(randomBid) << "," << convertPrice(randomAsk) << "\n";

            // generate order book data
            oFile << timestamp << "," << bond;
            for (int level=1; level<=5; ++level){
                double fixBid = midPrice - fixSpread * level / 2.0;
                double fixAsk = midPrice + fixSpread * level / 2.0;
                int size = level * 1'000'000;
                oFile << "," << convertPrice(fixBid) << "," << size << "," << convertPrice(fixAsk) << "," << size;
            }
            oFile << "\n";

            // oscillate mid price
            if (priceIncreasing) {
                midPrice += 1.0 / 256.0;
                if (randomAsk >= 101.0) {
                    priceIncreasing = false;
                }
            } else {
                midPrice -= 1.0 / 256.0;
                if (randomBid <= 99.0) {
                    priceIncreasing = true;
                }
            }

            // oscillate spread
            if (spreadIncreasing) {
                fixSpread += 1.0 / 128.0;
                if (fixSpread >= 1.0 / 32.0) {
                    spreadIncreasing = false;
                }
            } else {
                fixSpread -= 1.0 / 128.0;
                if (fixSpread <= 1.0 / 128.0) {
                    spreadIncreasing = true;
                }
            }
        }
    }

    pFile.close();
    oFile.close();
}



#endif
