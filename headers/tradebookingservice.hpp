/**
 * tradebookingservice.hpp
 * Defines the data types and Service for trade booking.
 * Trade booking service has a special listener to subscribe data from execution service.
 *
 * @author Boyu Yang
 */
#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include <string>
#include <map>
#include <fstream>
#include <boost/asio.hpp>
#include <thread>
#include <chrono>

#include "soa.hpp"
#include "utils.hpp"
#include "executionservice.hpp"

// Trade sides
enum Side { BUY, SELL };

/**
 * Trade object with a price, side, and quantity on a particular book.
 * Type T is the product type.
 */
template<typename T>
class Trade
{

public:

  // ctor for a trade
  Trade() = default;
  Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side);

  // Get the product
  const T& GetProduct() const;

  // Get the trade ID
  const string& GetTradeId() const;

  // Get the mid price
  double GetPrice() const;

  // Get the book
  const string& GetBook() const;

  // Get the quantity
  long GetQuantity() const;

  // Get the side
  Side GetSide() const;

private:
  T product;
  string tradeId;
  double price;
  string book;
  long quantity;
  Side side;

};


template<typename T>
Trade<T>::Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side) :
  product(_product)
{
  tradeId = _tradeId;
  price = _price;
  book = _book;
  quantity = _quantity;
  side = _side;
}

template<typename T>
const T& Trade<T>::GetProduct() const
{
  return product;
}

template<typename T>
const string& Trade<T>::GetTradeId() const
{
  return tradeId;
}

template<typename T>
double Trade<T>::GetPrice() const
{
  return price;
}

template<typename T>
const string& Trade<T>::GetBook() const
{
  return book;
}

template<typename T>
long Trade<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
Side Trade<T>::GetSide() const
{
  return side;
}

// forward declaration of an inbound connector and a trade booking listener that subscribes data from execution service
template<typename T>
class TradeBookingServiceListener;
template<typename T>
class TradeDataConnector;

/**
 * Trade Booking Service to book trades to a particular book.
 * Keyed on trade id.
 * Type T is the product type.
 */
template<typename T>
class TradeBookingService : public Service<string,Trade <T> >
{
private:
  map<string, Trade<T>> tradeMap; // store trade data keyed by trade id
  vector<ServiceListener<Trade<T>>*> listeners; // list of listeners to this service
  TradeBookingServiceListener<T>* tradebookinglistener;
  string host; // host name for inbound connector
  string port; // port number for inbound connector
  TradeDataConnector<T>* connector; // connector related to this server

public:
  // ctor and dtor
  TradeBookingService(const string& _host, const string& _port);
  ~TradeBookingService()=default;

  // Get data
  Trade<T>& GetData(string key);

  // The callback that a Connector should invoke for any new or updated data
  void OnMessage(Trade<T> &data);

  // Add a listener to the Service for callbacks on add, remove, and update events
  // for data to the Service.
  void AddListener(ServiceListener<Trade<T>> *listener);

  // Get all listeners on the Service.
  const vector< ServiceListener<Trade<T>>* >& GetListeners() const;

  // get the connector
  TradeDataConnector<T>* GetConnector();

  // Get associated trade book listener
  TradeBookingServiceListener<T>* GetTradeBookingServiceListener();

  void BookTrade(Trade<T> &trade);

};

template<typename T>
TradeBookingService<T>::TradeBookingService(const string& _host, const string& _port)
: host(_host), port(_port)
{
  connector = new TradeDataConnector<T>(this, host, port); // connector related to this server
  tradebookinglistener = new TradeBookingServiceListener<T>(this); // listener related to this server
}


template<typename T>
Trade<T>& TradeBookingService<T>::GetData(string key)
{
  return tradeMap[key];
}

template<typename T>
void TradeBookingService<T>::OnMessage(Trade<T> &data)
{
  string key = data.GetTradeId();
  if (tradeMap.find(key) != tradeMap.end())
    tradeMap[key] = data;
  else
    tradeMap.insert(pair<string, Trade<T>>(key, data));

  for(auto& listener : listeners)
    listener->ProcessAdd(data);
}

template<typename T>
void TradeBookingService<T>::AddListener(ServiceListener<Trade<T>> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<Trade<T>>* >& TradeBookingService<T>::GetListeners() const
{
  return listeners;
}

template<typename T>
TradeDataConnector<T>* TradeBookingService<T>::GetConnector()
{
  return connector;
}

template<typename T>
TradeBookingServiceListener<T>* TradeBookingService<T>::GetTradeBookingServiceListener()
{
  return tradebookinglistener;
}


/**
 * Book a trade and send the information to the listener
 */
template<typename T>
void TradeBookingService<T>::BookTrade(Trade<T> &trade)
{
  // flow the data to listeners
  // before this, make sure the special listener is added to the service
  // As we notify the special listener, ProcessAdd() will be called to connect data between different services
  for(auto& l : listeners)
    l->ProcessAdd(trade);

}

/**
* Connector that subscribes data from socket to trade booking service.
* Type T is the product type.
*/
template<typename T>
class TradeDataConnector : public Connector<Trade<T>>
{
private:
  TradeBookingService<T>* service;
  string host; // host name
  string port; // port number
  boost::asio::io_service io_service; // io service
  boost::asio::ip::tcp::socket socket; // socket

  void handle_read(const boost::system::error_code& ec, std::size_t length, boost::asio::ip::tcp::socket* socket, boost::asio::streambuf* request);
  void start_accept(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::io_service* io_service);

public:
  // ctor
  TradeDataConnector(TradeBookingService<T>* _service, const string& _host, const string& _port);
  // dtor: close the socket
  ~TradeDataConnector();

  // Publish data to the Connector
  void Publish(Trade<T> &data) override;

  // Subscribe data from the Connector
  void Subscribe();

};

template<typename T>
TradeDataConnector<T>::TradeDataConnector(TradeBookingService<T>* _service, const string& _host, const string& _port)
: service(_service), host(_host), port(_port), socket(io_service)
{
}

template<typename T>
TradeDataConnector<T>::~TradeDataConnector()
{
  socket.close();
}

template<typename T>
void TradeDataConnector<T>::start_accept(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::io_service* io_service) {
  boost::asio::ip::tcp::socket* socket = new boost::asio::ip::tcp::socket(*io_service);
  acceptor->async_accept(*socket, [this, socket, acceptor, io_service](const boost::system::error_code& ec) {
    if (!ec) {
      boost::asio::streambuf* request = new boost::asio::streambuf;
      boost::asio::async_read_until(*socket, *request, "\n", std::bind(&TradeDataConnector<T>::handle_read, this, std::placeholders::_1, std::placeholders::_2, socket, request));
    }
    start_accept(acceptor, io_service); // accept the next connection
  });
}


template<typename T>
void TradeDataConnector<T>::handle_read(const boost::system::error_code& ec, std::size_t length, boost::asio::ip::tcp::socket* socket, boost::asio::streambuf* request) {
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
      vector<string> tokens;
      stringstream lineStream(line);
      string token;
      while(getline(lineStream, token, ','))
        tokens.push_back(token);

      // create a trade object
      string productId = tokens[0];
      // create product object based on product id
      T product = getProductObject<T>(productId);
      string tradeId = tokens[1];
      double price = convertPrice(tokens[2]);
      string book = tokens[3];
      long quantity = stol(tokens[4]);
      Side side = tokens[5] == "BUY" ? BUY : SELL;
      Trade<T> trade(product, tradeId, price, book, quantity, side);

      // flows data to trade booking service
      service->OnMessage(trade);
    }

    boost::asio::async_read_until(*socket, *request, "\n", std::bind(&TradeDataConnector<T>::handle_read, this, std::placeholders::_1, std::placeholders::_2, socket, request));
  } else {
    delete request; // delete the streambuf when we're done with it
    delete socket; // delete the socket when we're done with it
  }
}

template<typename T>
void TradeDataConnector<T>::Publish(Trade<T> &data)
{
}

template<typename T>
void TradeDataConnector<T>::Subscribe()
{
  log(LogLevel::NOTE, "Trade data server listening on " + host + ":" + port);
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

/**
 * Trade Booking Execution Listener subscribing from execution service.
 * Basically, this listener is used to subscribe data from execution service,
 * transfer the ExecutionOrder<T> data to Trade<T> data, and call BookTrade()
 * method to publish the Trade<T> data to Trade Booking Service.
 */
template<typename T>
class TradeBookingServiceListener : public ServiceListener<ExecutionOrder<T>>
{
private:
  TradeBookingService<T>* service;
  long count;

public:
  // ctor
  TradeBookingServiceListener(TradeBookingService<T>* _service);
  // dtor
  ~TradeBookingServiceListener()=default;

  // Listener callback to process an add event to the Service
  void ProcessAdd(ExecutionOrder<T> &data) override;

  // Listener callback to process a remove event to the Service
  void ProcessRemove(ExecutionOrder<T> &data) override;

  // Listener callback to process an update event to the Service
  void ProcessUpdate(ExecutionOrder<T> &data) override;

};

template<typename T>
TradeBookingServiceListener<T>::TradeBookingServiceListener(TradeBookingService<T>* _service)
{
  service = _service;
  count = 0;
}


/**
 * ProcessAdd() method is used to transfer ExecutionOrder<T> data to Trade<T> data,
 * and then call BookTrade() method to publish the Trade<T> data to Trade Booking Service.
 */
template<typename T>
void TradeBookingServiceListener<T>::ProcessAdd(ExecutionOrder<T> &data)
{
  T product = data.GetProduct();
  string orderId = data.GetOrderId();
  double price = data.GetPrice();
  PricingSide pside = data.GetSide();  
  long vQty = data.GetVisibleQuantity();
  long hQty = data.GetHiddenQuantity();
  long quantity = vQty + hQty;
  Side side;
  switch (pside){
    case BID:
      side = BUY;
      break;
    case OFFER:
      side = SELL;
      break;
  }
  string book;
  // switch book based on count
  count++;
  switch (count%3)
  {
  case 0:
    book = "TRSY1";
    break;
  case 1:
    book = "TRSY2";
    break;
  case 2:
    book = "TRSY3";
    break;
  }

  Trade<T> trade(product, orderId, price, book, quantity, side);

  // book the trade
  service->BookTrade(trade);
}

template<typename T>
void TradeBookingServiceListener<T>::ProcessRemove(ExecutionOrder<T> &data)
{
}

template<typename T>
void TradeBookingServiceListener<T>::ProcessUpdate(ExecutionOrder<T> &data)
{
}


#endif






