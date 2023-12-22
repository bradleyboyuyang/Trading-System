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
#include <vector>
#include "soa.hpp"
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

// forward declaration of connector and a trade booking listener that subscribes data from execution service
template<typename T>
class TradeBookingConnector;
template<typename T>
class TradeBookingListener;

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
  TradeBookingConnector<T>* connector;
  TradeBookingListener<T>* tradebooklistener;

public:
  // ctor and dtor
  TradeBookingService();
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

  // Get the connector
  TradeBookingConnector<T>* GetConnector();

  // Get associated trade book listener
  TradeBookingListener<T>* GetTradeBookingListener();

  // Book the trade
  void BookTrade(Trade<T> &trade);

};

template<typename T>
TradeBookingService<T>::TradeBookingService()
{
  connector = new TradeBookingConnector<T>(this);
  tradebooklistener = new TradeBookingListener<T>(this);
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
TradeBookingConnector<T>* TradeBookingService<T>::GetConnector()
{
  return connector;
}

template<typename T>
TradeBookingListener<T>* TradeBookingService<T>::GetTradeBookingListener()
{
  return tradebooklistener;
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
class TradeBookingConnector : public Connector<Trade<T>>
{
private:
  TradeBookingService<T>* service;

public:
  // ctor
  TradeBookingConnector(TradeBookingService<T>* _service);
  // dtor
  ~TradeBookingConnector()=default;

  // Publish data to the Connector
  void Publish(Trade<T> &data) override;

  // Subscribe data from the Connector
  void Subscribe(const string& dataFile);

};

template<typename T>
TradeBookingConnector<T>::TradeBookingConnector(TradeBookingService<T>* _service)
{
  service = _service;
}

template<typename T>
void TradeBookingConnector<T>::Publish(Trade<T> &data)
{
}

template<typename T>
void TradeBookingConnector<T>::Subscribe(const string& dataFile)
{
  ifstream file(dataFile);
  string line;
  while(getline(file, line))
  {
    // parse the line
    vector<string> tokens;
    stringstream ss(line);
    string token;
    while(getline(ss, token, ','))
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
}

/**
 * Trade Booking Execution Listener subscribing from execution service.
 * Basically, this listener is used to subscribe data from execution service,
 * transfer the ExecutionOrder<T> data to Trade<T> data, and call BookTrade()
 * method to publish the Trade<T> data to Trade Booking Service.
 */
template<typename T>
class TradeBookingListener : public ServiceListener<ExecutionOrder<T>>
{
private:
  TradeBookingService<T>* service;
  long count;

public:
  // ctor
  TradeBookingListener(TradeBookingService<T>* _service);
  // dtor
  ~TradeBookingListener()=default;

  // Listener callback to process an add event to the Service
  void ProcessAdd(ExecutionOrder<T> &data) override;

  // Listener callback to process a remove event to the Service
  void ProcessRemove(ExecutionOrder<T> &data) override;

  // Listener callback to process an update event to the Service
  void ProcessUpdate(ExecutionOrder<T> &data) override;

};

template<typename T>
TradeBookingListener<T>::TradeBookingListener(TradeBookingService<T>* _service)
{
  service = _service;
  count = 0;
}


/**
 * ProcessAdd() method is used to transfer ExecutionOrder<T> data to Trade<T> data,
 * and then call BookTrade() method to publish the Trade<T> data to Trade Booking Service.
 */
template<typename T>
void TradeBookingListener<T>::ProcessAdd(ExecutionOrder<T> &data)
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
void TradeBookingListener<T>::ProcessRemove(ExecutionOrder<T> &data)
{
}

template<typename T>
void TradeBookingListener<T>::ProcessUpdate(ExecutionOrder<T> &data)
{
}


#endif






