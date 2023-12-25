/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Boyu Yang
 */
#ifndef PRICINGservice_HPP
#define PRICINGservice_HPP

#include <string>
#include <map>
#include <fstream>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>

#include "soa.hpp"
#include "utils.hpp"

/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template<typename T>
class Price
{

public:
  // default ctor (needed for map data structure later)
  Price() = default;

  // ctor for a price
  Price(const T& _product, double _mid, double _bidOfferSpread);

  // Get the product
  const T& GetProduct() const;

  // Get the mid price
  double GetMid() const;

  // Get the bid/offer spread around the mid
  double GetBidOfferSpread() const;

  // object printer
  template<typename U>
  friend ostream& operator<<(ostream& os, const Price<U>& price);

private:
  T product;
  double mid;
  double bidOfferSpread;

};

template<typename T>
Price<T>::Price(const T& _product, double _mid, double _bidOfferSpread) 
: product(_product), mid(_mid), bidOfferSpread(_bidOfferSpread)
{
}

template<typename T>
const T& Price<T>::GetProduct() const
{
  return product;
}

template<typename T>
double Price<T>::GetMid() const 
{
  return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const
{
  return bidOfferSpread;
}

template<typename T>
ostream& operator<<(ostream& os, const Price<T>& price)
{
  T product = price.GetProduct();
	string _product = product.GetProductId();
  double mid = price.GetMid();
  double bidOfferSpread = price.GetBidOfferSpread();
	string _mid = convertPrice(mid);
	string _bidOfferSpread = convertPrice(bidOfferSpread);

	vector<string> _strings;
	_strings.push_back(_product);
	_strings.push_back(_mid);
	_strings.push_back(_bidOfferSpread);
  string _str = join(_strings, ",");
  os << _str;

  return os;
}

// forward declaration of PriceDataConnector
template<typename T>
class PriceDataConnector;

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PricingService : public Service<string, Price<T>>
{
private:
  map<string, Price<T>> priceMap; // store price data keyed by product identifier
  vector<ServiceListener<Price<T>>*> listeners; // list of listeners to this service
  string host; // host name for inbound connector
  string port; // port number for inbound connector
  PriceDataConnector<T>* connector; // connector related to this server

public:
  // ctor
  PricingService(const string& _host, const string& _port);
  // dtor
  ~PricingService() = default;


  // Get data on our service given a key
  Price<T>& GetData(string key) override;

  // The callback that a Connector should invoke for any new or updated data
  void OnMessage(Price<T> &data) override;

  // Add a listener to the Service for callbacks on add, remove, and update events
  // for data to the Service.
  void AddListener(ServiceListener<Price<T>> *listener) override;

  // Get all listeners on the Service.
  const vector< ServiceListener<Price<T>>* >& GetListeners() const override;

  // Get the connector
  PriceDataConnector<T>* GetConnector();

};

template<typename T>
PricingService<T>::PricingService(const string& _host, const string& _port)
: host(_host), port(_port)
{
  connector = new PriceDataConnector<T>(this, host, port); // connector related to this server
}

template<typename T>
Price<T>& PricingService<T>::GetData(string key)
{
  return priceMap[key];
}

template<typename T>
void PricingService<T>::OnMessage(Price<T> &data)
{
    // flow data
    string key = data.GetProduct().GetProductId();
    // update the price map
    if (priceMap.find(key) != priceMap.end()) {priceMap.erase(key);}
    priceMap.insert(pair<string, Price<Bond> > (key, data));

    // flow the data to listeners
    for (auto& l : listeners) {
        l -> ProcessAdd(data);
    }
}

template<typename T>
void PricingService<T>::AddListener(ServiceListener<Price<T>> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<Price<T>>* >& PricingService<T>::GetListeners() const
{
  return listeners;
}

template<typename T>
PriceDataConnector<T>* PricingService<T>::GetConnector()
{
  return connector;
}

/**
 * PriceDataConnector: an inbound connector that subscribes data from socket to pricing service.
 * Type T is the product type.
 */
template<typename T>
class PriceDataConnector : public Connector<Price<T>>
{
private:
  PricingService<T>* service; 
  string host; // host name
  string port; // port number
  boost::asio::io_service io_service; // io service
  boost::asio::ip::tcp::socket socket; // socket

  void handle_read(const boost::system::error_code& ec, std::size_t length, boost::asio::ip::tcp::socket* socket, boost::asio::streambuf* request);
  void start_accept(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::io_service* io_service);

public:
  // ctor
  PriceDataConnector(PricingService<T>* _service, const string& _host, const string& _port);
  // dtor
  ~PriceDataConnector();

  // Publish data to the Connector
  void Publish(Price<T> &data) override;

  // Subscribe data from socket
  void Subscribe();

};

template<typename T>
PriceDataConnector<T>::PriceDataConnector(PricingService<T>* _service, const string& _host, const string& _port)
: service(_service), host(_host), port(_port), socket(io_service)
{
}

template<typename T>
PriceDataConnector<T>::~PriceDataConnector()
{
    socket.close();
}

template<typename T>
void PriceDataConnector<T>::start_accept(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::io_service* io_service) {
  boost::asio::ip::tcp::socket* socket = new boost::asio::ip::tcp::socket(*io_service);
  acceptor->async_accept(*socket, [this, socket, acceptor, io_service](const boost::system::error_code& ec) {
    if (!ec) {
      boost::asio::streambuf* request = new boost::asio::streambuf;
      boost::asio::async_read_until(*socket, *request, "\n", std::bind(&PriceDataConnector<T>::handle_read, this, std::placeholders::_1, std::placeholders::_2, socket, request));
    }
    start_accept(acceptor, io_service); // accept the next connection
  });
}

template<typename T>
void PriceDataConnector<T>::handle_read(const boost::system::error_code& ec, std::size_t length, boost::asio::ip::tcp::socket* socket, boost::asio::streambuf* request) {
  if (!ec) {
    // get the entire data
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
      // process each line
      vector<string> lineData;
      stringstream lineStream(line); // turn the line into a stream
      string word;
      while (getline(lineStream, word, ','))  
      {
        lineData.push_back(word);
      }
      string timestamp = lineData[0];
      string productId = lineData[1];
      double bid = convertPrice(lineData[2]);
      double ask = convertPrice(lineData[3]);
      double spread = stod(lineData[4]);
      double mid = (bid + ask) / 2.0;
      // create product object based on product id
      T product = getProductObject<T>(productId);
      // create price object based on product, mid price and bid/offer spread
      Price<T> price(product, mid, spread);
      // publish data to service
      service->OnMessage(price);
    }

    boost::asio::async_read_until(*socket, *request, "\n", std::bind(&PriceDataConnector<T>::handle_read, this, std::placeholders::_1, std::placeholders::_2, socket, request));
  } else {
    delete request; // delete the streambuf when we're done with it
    delete socket; // delete the socket when we're done with it
  }
}
// inbound connector, does nothing
template <typename T>
void PriceDataConnector<T>::Publish(Price<T> &data)
{ 
}

template<typename T>
void PriceDataConnector<T>::Subscribe()
{
  log(LogLevel::NOTE, "Price data server listening on " + host + ":" + port);
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
    socket.close();
    return;
  }
}

#endif
