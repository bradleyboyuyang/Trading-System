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
#include <thread>

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

// join a string vector with a delimiter
string join(const vector<string>& v, const string& delimiter) {
    string res;
    for (const auto& s : v) {
        res += s + delimiter;
    }
    return res.substr(0, res.size() - delimiter.size());
}


// get the system time and return in milliseconds format (e.g. 2023-12-23 22:42:44.260)
string getTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // change the system time to local time
    std::tm now_tm = *std::localtime(&now_c);

    // use stringstream to format the time
    stringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");

    // get the milliseconds separately
    auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    ss << '.' << std::setfill('0') << std::setw(3) << milli.count();

    return ss.str();
}

// change time point format to string 
string getTime(std::chrono::system_clock::time_point now) {
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);
    stringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    ss << '.' << std::setfill('0') << std::setw(3) << milli.count();
    return ss.str();
}


enum class LogLevel {
    INFO,
    NOTE,
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
        case LogLevel::NOTE:
            levelStr = "NOTE";
            color = CYAN;
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

// get Product object from identifier
// Define a type for a function that takes no arguments and returns a T
template <typename T>
using ProductConstructor = std::function<T()>;

// Define a map from CUSIPs to product constructors
template <typename T>
std::map<string, ProductConstructor<T>> productConstructors = {
    {"9128283H1", []() { return Bond("9128283H1", CUSIP, "US2Y", 0.01750, from_string("2019/11/30")); }},
    {"9128283L2", []() { return Bond("9128283L2", CUSIP, "US3Y", 0.01875, from_string("2020/12/15")); }},
    {"912828M80", []() { return Bond("912828M80", CUSIP, "US5Y", 0.02000, from_string("2022/11/30")); }},
    {"9128283J7", []() { return Bond("9128283J7", CUSIP, "US7Y", 0.02125, from_string("2024/11/30")); }},
    {"9128283F5", []() { return Bond("9128283F5", CUSIP, "US10Y", 0.02250, from_string("2027/12/15")); }},
    {"912810TW8", []() { return Bond("912810TW8", CUSIP, "US20Y", 0.02500, from_string("2037/12/15")); }},
    {"912810RZ3", []() { return Bond("912810RZ3", CUSIP, "US30Y", 0.02750, from_string("2047/12/15")); }},
};

template <typename T>
T getProductObject(const string& cusip) {
    auto it = productConstructors<T>.find(cusip);
    if (it == productConstructors<T>.end()) {
        throw std::invalid_argument("Unknown CUSIP: " + cusip);
    }
    return it->second();
}


// function to calculate PV01
double calculate_pv01(double face_value, double coupon_rate, double yield_rate, int years_to_maturity, int frequency) {
    double coupon = face_value * coupon_rate / frequency;
    double pv01 = 0.0;
    for (int t = 1; t <= years_to_maturity * frequency; ++t) {
        double discount_factor = 1.0 / pow(1.0 + yield_rate / frequency, t);
        pv01 += coupon * discount_factor;
    }
    pv01 += face_value / pow(1.0 + yield_rate / frequency, years_to_maturity * frequency);
    double pv01_initial = pv01;
    yield_rate += 0.0001;
    pv01 = 0.0;
    for (int t = 1; t <= years_to_maturity * frequency; ++t) {
        double discount_factor = 1.0 / pow(1.0 + yield_rate / frequency, t);
        pv01 += coupon * discount_factor;
    }
    pv01 += face_value / pow(1.0 + yield_rate / frequency, years_to_maturity * frequency);
    double pv01_change = pv01_initial - pv01;
    return pv01_change;
}


// Define a map from CUSIPs to PV01
// Current yield for 2,3,5,7,10,20,30 year US treasury bonds: 0.0464, 0.0440, 0.0412, 0.043, 0.0428, 0.0461, 0.0443
std::map<string, double> pv01 = {
    {"9128283H1", calculate_pv01(1000, 0.01750, 0.0464, 2, 2)},
    {"9128283L2", calculate_pv01(1000, 0.01875, 0.0440, 3, 2)},
    {"912828M80", calculate_pv01(1000, 0.02000, 0.0412, 5, 2)},
    {"9128283J7", calculate_pv01(1000, 0.02125, 0.0430, 7, 2)},
    {"9128283F5", calculate_pv01(1000, 0.02250, 0.0428, 10, 2)},
    {"912810TW8", calculate_pv01(1000, 0.02500, 0.0461, 20, 2)},
    {"912810RZ3", calculate_pv01(1000, 0.02750, 0.0443, 30, 2)},
};

// Get unit PV01 value from CUSIP
double getPV01(const string& cusip) {
    auto it = pv01.find(cusip);
    if (it == pv01.end()) {
        throw std::invalid_argument("Unknown CUSIP: " + cusip);
    }
    return it->second;
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
    std::uniform_real_distribution<double> dist(1.0/128.0, 1.0/64.0);
    return dist(gen);
}

// Generate random ID with numbers and letters
string GenerateRandomId(long length)
{
    string id = "";
    for (int j = 0; j < length; ++j) {
        int random = rand() % 36;
        if (random < 10) {
            id += to_string(random);
        } else {
            id += static_cast<char>('A' + random - 10);
        }
    }
    return id;
}

/**
 * 1. Generate prices that oscillate between 99 and 101 and write to prices.txt
 * 2. Generate order book data with fivel levels of bids and offers and write to marketdata.txt
 */
void genOrderBook(const vector<string>& products, const string& priceFile, const string& orderbookFile, long long seed, const int numDataPoints) {
    std::ofstream pFile(priceFile);
    std::ofstream oFile(orderbookFile);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> ms_dist(1, 20); // simulate milliseconds increments

    // price file format: Timestamp, CUSIP, Bid, Ask
    // orderbook file format: Timestamp, CUSIP, Bid1, BidSize1, Ask1, AskSize1, Bid2, BidSize2, Ask2, AskSize2, Bid3, BidSize3, Ask3, AskSize3, Bid4, BidSize4, Ask4, AskSize4, Bid5, BidSize5, Ask5, AskSize5

    for (const auto& product : products) {
        double midPrice = 99.00;
        bool priceIncreasing = true;
        bool spreadIncreasing = true;
        double fixSpread = 1.0/128.0;
        auto curTime = std::chrono::system_clock::now();
        string timestamp;

        // number of data points
        for (int i = 0; i < numDataPoints; ++i) {

            // generate price data
            double randomSpread = genRandomSpread(gen);
            curTime += std::chrono::milliseconds(ms_dist(gen));
            timestamp = getTime(curTime);

            double randomBid = midPrice - randomSpread / 2.0;
            double randomAsk = midPrice + randomSpread / 2.0;
            pFile << timestamp << "," << product << "," << convertPrice(randomBid) << "," << convertPrice(randomAsk) << "," << randomSpread << endl;

            // generate order book data
            oFile << timestamp << "," << product;
            for (int level=1; level<=5; ++level){
                double fixBid = midPrice - fixSpread * level / 2.0;
                double fixAsk = midPrice + fixSpread * level / 2.0;
                int size = level * 1'000'000;
                oFile << "," << convertPrice(fixBid) << "," << size << "," << convertPrice(fixAsk) << "," << size;
            }
            oFile << endl;

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

/**
 * Generate trades data
 */
void genTrades(const vector<string>& products, const string& tradeFile, long long seed) {
    vector<string> books = {"TRSY1", "TRSY2", "TRSY3"};
    vector<long> quantities = {1000000, 2000000, 3000000, 4000000, 5000000};
    std::ofstream tFile(tradeFile);
    std::mt19937 gen(seed);

    for (const auto& product : products) {
        for (int i = 0; i < 10; ++i) {
            string side = (i % 2 == 0) ? "BUY" : "SELL";
            // generate a 12 digit random trade id with number and letters
            string tradeId = GenerateRandomId(12);
            // generate random buy price 99-100 and random sell price 100-101 with given seed
            std::uniform_real_distribution<double> dist(side == "BUY" ? 99.0 : 100.0, side == "BUY" ? 100.0 : 101.0);
            double price = dist(gen);
            long quantity = quantities[i % quantities.size()];
            string book = books[i % books.size()];

        tFile << product << "," << tradeId << "," << convertPrice(price) << "," << book << "," << quantity << "," << side << endl;
        }
    }

    tFile.close();
}

/**
 * Generate inquiry data
 */
void genInquiries(const vector<string>& products, const string& inquiryFile, long long seed){
    std::ofstream iFile(inquiryFile);
    std::mt19937 gen(seed);
    vector<long> quantities = {1000000, 2000000, 3000000, 4000000, 5000000};

    for (const auto& product : products) {
        for (int i = 0; i < 10; ++i) {
            string side = (i % 2 == 0) ? "BUY" : "SELL";
            // generate a 12 digit random inquiry id with number and letters
            string inquiryId = GenerateRandomId(12);
            // generate random buy price 99-100 and random sell price 100-101 with given seed
            std::uniform_real_distribution<double> dist(side == "BUY" ? 99.0 : 100.0, side == "BUY" ? 100.0 : 101.0);
            double price = dist(gen);
            long quantity = quantities[i % quantities.size()];
            string status = "RECEIVED";

        iFile << inquiryId << "," << product << "," << side << "," << quantity << "," << convertPrice(price) << "," << status << endl;
        }
    }
}

// function to check whether a thread has completed and join it if yes
// introduce the try-catch block as required
void join(std::thread& t) {
    if (t.joinable()) {
        t.join();
    }
    else {
        try {
            t.join();
        }
        catch (std::exception& e) {
            std::cout << "Exception caught: " << e.what() << std::endl;
        }
    }
}

#endif
