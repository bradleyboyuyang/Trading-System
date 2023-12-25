/**
 * marketdataservice.hpp
 * Defines the data types and Service for order book market data.
 *
 * @author Boyu Yang
 */
#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <map>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <boost/asio.hpp>

#include "soa.hpp"
#include "utils.hpp"

using namespace std;

// Side for market data
enum PricingSide { BID, OFFER };

/**
 * A market data order with price, quantity, and side.
 */
class Order
{

public:
  // default ctor, need by the map data structure
  Order() = default;

  // ctor for an order
  Order(double _price, long _quantity, PricingSide _side);

  // Get the price on the order
  double GetPrice() const;

  // Get the quantity on the order
  long GetQuantity() const;

  // Get the side on the order
  PricingSide GetSide() const;

private:
  double price;
  long quantity;
  PricingSide side;

};

Order::Order(double _price, long _quantity, PricingSide _side) :
  price(_price), quantity(_quantity), side(_side)
{
}

double Order::GetPrice() const
{
  return price;
}

long Order::GetQuantity() const
{
  return quantity;
}

PricingSide Order::GetSide() const
{
  return side;
}


/**
 * Class representing a bid and offer order
 */
class BidOffer
{

public:

  // ctor for bid/offer
  BidOffer(const Order &_bidOrder, const Order &_offerOrder);

  // Get the bid order
  const Order& GetBidOrder() const;

  // Get the offer order
  const Order& GetOfferOrder() const;

private:
  Order bidOrder;
  Order offerOrder;

};

BidOffer::BidOffer(const Order &_bidOrder, const Order &_offerOrder) :
  bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

const Order& BidOffer::GetBidOrder() const
{
  return bidOrder;
}

const Order& BidOffer::GetOfferOrder() const
{
  return offerOrder;
}


/**
 * Order book with a bid and offer stack.
 * Type T is the product type.
 */
template<typename T>
class OrderBook
{

public:

  // default ctor, needed for the map data structure
  OrderBook() = default;

  OrderBook(const string& productId);

  // ctor for the order book
  OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack);

  // Get the product
  const T& GetProduct() const;

  // Get the bid stack
  vector<Order>& GetBidStack();

  // Get the offer stack
  vector<Order>& GetOfferStack();

  // Get the best bid/offer order
  BidOffer GetBestBidOffer() const;


private:
  T product;
  vector<Order> bidStack;
  vector<Order> offerStack;

};

template<typename T>
OrderBook<T>::OrderBook(const string& productId) 
: product(getProductObject<T>(productId)), bidStack(vector<Order>()), offerStack(vector<Order>())
{
}


template<typename T>
OrderBook<T>::OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack) :
  product(_product), bidStack(_bidStack), offerStack(_offerStack)
{
}

template<typename T>
const T& OrderBook<T>::GetProduct() const
{
  return product;
}

template<typename T>
vector<Order>& OrderBook<T>::GetBidStack()
{
  return bidStack;
}

template<typename T>
vector<Order>& OrderBook<T>::GetOfferStack()
{
  return offerStack;
}

template<typename T>
BidOffer OrderBook<T>::GetBestBidOffer() const
{
  // iterate bid stack and offer stack to find the best bid/offer order
  auto bestBid = std::max_element(bidStack.begin(), bidStack.end(), [](const Order& a, const Order& b) {return a.GetPrice() < b.GetPrice(); });
  auto bestOffer = std::min_element(offerStack.begin(), offerStack.end(), [](const Order& a, const Order& b) {return a.GetPrice() < b.GetPrice(); });
  return BidOffer(*bestBid, *bestOffer);
  
}

// forward declaration of MarketDataConnector
template<typename T>
class MarketDataConnector;

/**
 * Market Data Service which distributes market data
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class MarketDataService : public Service<string,OrderBook <T> >
{
private:
  map<string, OrderBook<T>> orderBookMap;
  vector<ServiceListener<OrderBook<T>>*> listeners;
  int bookDepth;
  string host; // host name for inbound connector
  string port; // port number for inbound connector
  MarketDataConnector<T>* connector; // connector related to this server

public:
  // ctor and dtor
  MarketDataService(const string& _host, const string& _port);
  ~MarketDataService()=default;

  // Get data on our service given a key
  OrderBook<T>& GetData(string key) override;

  // The callback that a Connector should invoke for any new or updated data
  void OnMessage(OrderBook<T>& data) override;

  // Add a listener to the Service for callbacks on add, remove, and update events
  void AddListener(ServiceListener<OrderBook<T>>* listener) override;

  // Get all listeners on the Service
  const vector<ServiceListener<OrderBook<T>>*>& GetListeners() const override;

  // Get the connector
  MarketDataConnector<T>* GetConnector();

  // Get the book depth
  int GetBookDepth() const;

  // Get the best bid/offer order
  const BidOffer& GetBestBidOffer(const string &productId);

  // Aggregate the order book
  const OrderBook<T>& AggregateDepth(const string &productId);

};


template<typename T>
MarketDataService<T>::MarketDataService(const string& _host, const string& _port)
: host(_host), port(_port)
{
  bookDepth = 5; // default book depth
  connector = new MarketDataConnector<T>(this, host, port); // connector related to this server
}

template<typename T>
OrderBook<T>& MarketDataService<T>::GetData(string key)
{
  // if the order book does not exist, create a new one
  if (orderBookMap.find(key) == orderBookMap.end()) {
    orderBookMap.insert(pair<string, OrderBook<T>>(key, OrderBook<T>(key)));
  }
  return orderBookMap[key];
}

template<typename T>
void MarketDataService<T>::OnMessage(OrderBook<T>& data)
{
  string key = data.GetProduct().GetProductId();
  if (orderBookMap.find(key) != orderBookMap.end()) { orderBookMap.erase(key); }
  orderBookMap.insert(pair<string, OrderBook<T>>(key, data));


  for (auto& listener : listeners)
  {
    listener->ProcessAdd(data);
  }
}

template<typename T>
void MarketDataService<T>::AddListener(ServiceListener<OrderBook<T>>* listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<OrderBook<T>>*>& MarketDataService<T>::GetListeners() const
{
  return listeners;
}

template<typename T>
MarketDataConnector<T>* MarketDataService<T>::GetConnector()
{
  return connector;
}

template<typename T>
int MarketDataService<T>::GetBookDepth() const
{
  return bookDepth;
}

template<typename T>
const BidOffer& MarketDataService<T>::GetBestBidOffer(const string &productId)
{
  return orderBookMap[productId].GetBestBidOffer();
}

template<typename T>
const OrderBook<T>& MarketDataService<T>::AggregateDepth(const string &productId)
{
  // get the order book
  OrderBook<T>& orderBook = orderBookMap[productId];
  // get the bid stack and offer stack
  vector<Order>& bidStack = orderBook.GetBidStack();
  vector<Order>& offerStack = orderBook.GetOfferStack();
  // aggregate the bid stack
  unordered_map<double, long> aggBidMap;
  for (auto& order : bidStack){
    double price = order.GetPrice();
    long quantity = order.GetQuantity();
    if (aggBidMap.find(price) != aggBidMap.end()) {
      aggBidMap[price] += quantity;
    }
    else {
      aggBidMap.insert(pair<double, long>(price, quantity));
    }
  }
  vector<Order> aggBid;
  for (auto& item : aggBidMap) {
    aggBid.push_back(Order(item.first, item.second, BID));
  }
  // (optional) sort the aggregated bid stack
  // sort(aggBid.begin(), aggBid.end(), [](const Order& a, const Order& b) {return a.GetPrice() > b.GetPrice(); });

  // aggregate the offer stack
  unordered_map<double, long> aggOfferMap;
  for (auto& order : offerStack) {
    double price = order.GetPrice();
    long quantity = order.GetQuantity();
    if (aggOfferMap.find(price) != aggOfferMap.end()) {
      aggOfferMap[price] += quantity;
    }
    else {
      aggOfferMap.insert(pair<double, long>(price, quantity));
    }
  }
  vector<Order> aggOffer;
  for (auto& item : aggOfferMap) {
    aggOffer.push_back(Order(item.first, item.second, OFFER));
  }
  // (optional) sort the aggregated offer stack
  // sort(aggOffer.begin(), aggOffer.end(), [](const Order& a, const Order& b) {return a.GetPrice() < b.GetPrice(); });

  // update the order book
  orderBook = OrderBook<T>(orderBook.GetProduct(), aggBid, aggOffer);
  return orderBook;
}

/**
* Market Data Connector subscribing data from socket to Market Data Service.
* Type T is the product type.
*/
template<typename T>
class MarketDataConnector : public Connector<OrderBook<T>>
{
private:
  MarketDataService<T>* service;
  string host; // host name
  string port; // port number
  boost::asio::io_service io_service; // io service
  boost::asio::ip::tcp::socket socket; // socket

  void handle_read(const boost::system::error_code& ec, std::size_t length, boost::asio::ip::tcp::socket* socket, boost::asio::streambuf* request);
  void start_accept(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::io_service* io_service);

public:
  // ctor
  MarketDataConnector(MarketDataService<T>* _service, const string& _host, const string& _port);
  // dtor: close the socket
  ~MarketDataConnector();

  // Publish data to the Connector
  void Publish(OrderBook<T>& data) override;

  // Subscribe data
  void Subscribe();

};

template<typename T>
MarketDataConnector<T>::MarketDataConnector(MarketDataService<T>* _service, const string& _host, const string& _port) 
: service(_service), host(_host), port(_port), socket(io_service)
{
}

template<typename T>
MarketDataConnector<T>::~MarketDataConnector()
{
  socket.close();
}

template<typename T>
void MarketDataConnector<T>::start_accept(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::io_service* io_service) {
  boost::asio::ip::tcp::socket* socket = new boost::asio::ip::tcp::socket(*io_service);
  acceptor->async_accept(*socket, [this, socket, acceptor, io_service](const boost::system::error_code& ec) {
    if (!ec) {
      boost::asio::streambuf* request = new boost::asio::streambuf;
      boost::asio::async_read_until(*socket, *request, "\n", std::bind(&MarketDataConnector<T>::handle_read, this, std::placeholders::_1, std::placeholders::_2, socket, request));
    }
    start_accept(acceptor, io_service); // accept the next connection
  });
}

template<typename T>
void MarketDataConnector<T>::handle_read(const boost::system::error_code& ec, std::size_t length, boost::asio::ip::tcp::socket* socket, boost::asio::streambuf* request) {
  if (!ec) {
    std::string data = std::string(boost::asio::buffers_begin(request->data()), boost::asio::buffers_end(request->data()));
    // find the last newline
    std::size_t last_newline = data.rfind('\n');
    if (last_newline != std::string::npos) {
      // consume only up to the last newline
      request->consume(last_newline + 1);
      // only process the data up to the last newline
      data = data.substr(0, last_newline);
    } else {
      // if there's no newline, don't process any data
      data.clear();
    }

    // split the data into lines
    std::stringstream ss(data);
    std::string line;
    while (std::getline(ss, line)) {
      // parse the line
      vector<string> lineData;
      stringstream lineStream(line); // turn the line into a stream
      string word;
      while (getline(lineStream, word, ','))
      {
        lineData.push_back(word);
      }
      string timestamp = lineData[0];
      string productId = lineData[1];
      OrderBook<T>& orderBook = service->GetData(productId);

      string bidPrice, bidQty, askPrice, askQty;
      Order bidOrder, askOrder;
      for (int k = 0; k < service->GetBookDepth(); k++){
        bidPrice = lineData[4*k+2];
        bidQty = lineData[4*k+3];
        askPrice = lineData[4*k+4];
        askQty = lineData[4*k+5];
        bidOrder = Order(convertPrice(bidPrice), stol(bidQty), BID);
        askOrder = Order(convertPrice(askPrice), stol(askQty), OFFER);
        orderBook.GetBidStack().push_back(bidOrder);
        orderBook.GetOfferStack().push_back(askOrder);
      }
      // aggregate the order book, get a copy
      OrderBook<T> aggOrderBook = service->AggregateDepth(productId);
      // publish the order book to the service
      service->OnMessage(aggOrderBook);
    }

    boost::asio::async_read_until(*socket, *request, "\n", std::bind(&MarketDataConnector<T>::handle_read, this, std::placeholders::_1, std::placeholders::_2, socket, request));
  } else {
    delete request; // delete the streambuf when we're done with it
    delete socket; // delete the socket when we're done with it
  }
}

template<typename T>
void MarketDataConnector<T>::Publish(OrderBook<T>& data)
{
}

template<typename T>
void MarketDataConnector<T>::Subscribe()
{
  log(LogLevel::NOTE, "Market data server listening on " + host + ":" + port);
  try {
  // connect to the socket
  boost::asio::ip::tcp::resolver resolver(io_service);
  boost::asio::ip::tcp::resolver::query query(host, port);
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
  boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint);
  start_accept(&acceptor, &io_service);
  io_service.run();
  }
  catch (std::exception& e){
    // throw error log
    log(LogLevel::ERROR, e.what());
    return;
  }
}
#endif
