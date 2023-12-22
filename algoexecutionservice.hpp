/**
 * algoexecution.hpp
 * Defines the data types and Service for algo execution.
 *
 * @author Boyu Yang
 */
#ifndef ALGOEXECUTION_SERVICE_HPP
#define ALGOEXECUTION_SERVICE_HPP

#include <string>
#include "soa.hpp"  
#include "marketdataservice.hpp"
#include "utils.hpp"

enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

enum Market { BROKERTEC, ESPEED, CME };

/**
 * An execution order that can be placed on an exchange.
 * Type T is the product type.
 */
template<typename T>
class ExecutionOrder
{

public:

  // ctor for an order
  ExecutionOrder() = default; // needed for map data structure later
  ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

  // Get the product
  const T& GetProduct() const;

  // Get the pricing side
  PricingSide GetSide() const;

  // Get the order ID
  const string& GetOrderId() const;

  // Get the order type on this order
  OrderType GetOrderType() const;

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity
  long GetHiddenQuantity() const;

  // Get the parent order ID
  const string& GetParentOrderId() const;

  // Is child order?
  bool IsChildOrder() const;

  // object printer
  template<typename U>
  friend ostream& operator<<(ostream& os, const ExecutionOrder<U>& order);

private:
  T product;
  PricingSide side;
  string orderId;
  OrderType orderType;
  double price;
  long visibleQuantity;
  long hiddenQuantity;
  string parentOrderId;
  bool isChildOrder;

};

template<typename T>
ExecutionOrder<T>::ExecutionOrder(const T &_product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder) :
  product(_product)
{
  side = _side;
  orderId = _orderId;
  orderType = _orderType;
  price = _price;
  visibleQuantity = _visibleQuantity;
  hiddenQuantity = _hiddenQuantity;
  parentOrderId = _parentOrderId;
  isChildOrder = _isChildOrder;
}

template<typename T>
const T& ExecutionOrder<T>::GetProduct() const
{
  return product;
}

template<typename T>
PricingSide ExecutionOrder<T>::GetSide() const
{
  return side;
}


template<typename T>
const string& ExecutionOrder<T>::GetOrderId() const
{
  return orderId;
}

template<typename T>
OrderType ExecutionOrder<T>::GetOrderType() const
{
  return orderType;
}

template<typename T>
double ExecutionOrder<T>::GetPrice() const
{
  return price;
}

template<typename T>
long ExecutionOrder<T>::GetVisibleQuantity() const
{
  return visibleQuantity;
}

template<typename T>
long ExecutionOrder<T>::GetHiddenQuantity() const
{
  return hiddenQuantity;
}

template<typename T>
const string& ExecutionOrder<T>::GetParentOrderId() const
{
  return parentOrderId;
}

template<typename T>
bool ExecutionOrder<T>::IsChildOrder() const
{
  return isChildOrder;
}

template<typename T>
ostream& operator<<(ostream& os, const ExecutionOrder<T>& order)
{
  T product = order.GetProduct();
  string _product = product.GetProductId();
  string _orderId = order.GetOrderId();
  string _side = (order.GetSide()==BID? "Bid":"Ask");
  string _orderType;
  switch(order.GetOrderType()) {
      case FOK: _orderType = "FOK"; break;
      case MARKET: _orderType = "MARKET"; break;
      case LIMIT: _orderType = "LIMIT"; break;
      case STOP: _orderType = "STOP"; break;
      case IOC: _orderType = "IOC"; break;
  }
  string _price = convertPrice(order.GetPrice());
  string _visibleQuantity = to_string(order.GetVisibleQuantity());
  string _hiddenQuantity = to_string(order.GetHiddenQuantity());
  string _parentOrderId = order.GetParentOrderId();
  string _isChildOrder = (order.IsChildOrder()?"True":"False");

  vector<string> _strings;
  _strings.push_back(_product);
  _strings.push_back(_orderId);
  _strings.push_back(_side);
  _strings.push_back(_orderType);
  _strings.push_back(_price);
  _strings.push_back(_visibleQuantity);
  _strings.push_back(_hiddenQuantity);
  _strings.push_back(_parentOrderId);
  _strings.push_back(_isChildOrder);
  string _str = join(_strings, ",");
  os << _str;
  return os;
}

/**
 * Algo Execution Service to execute orders on market.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class AlgoExecution
{
private:
    ExecutionOrder<T> executionOrder;
    Market market;

public:
    // ctor for an order
    AlgoExecution() = default; // needed for map data structure later
    AlgoExecution(const ExecutionOrder<T> &_executionOrder, Market _market);

    // Get the execution order
    const ExecutionOrder<T>& GetExecutionOrder() const;

    // Get the market
    Market GetMarket() const;

};    

template<typename T>
AlgoExecution<T>::AlgoExecution(const ExecutionOrder<T> &_executionOrder, Market _market) :
  executionOrder(_executionOrder), market(_market)
{
}

template<typename T>
const ExecutionOrder<T>& AlgoExecution<T>::GetExecutionOrder() const
{
  return executionOrder;
}

template<typename T>
Market AlgoExecution<T>::GetMarket() const
{
  return market;
}

// forward declaration of AlgoExecutionServiceListener
template<typename T>
class AlgoExecutionServiceListener;


/**
 * Algo Execution Service to execute orders on market.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class AlgoExecutionService : public Service<string, AlgoExecution<T>>
{
private:
  map<string, AlgoExecution<T>> algoExecutionMap; // store algo execution data keyed by product identifier
  vector<ServiceListener<AlgoExecution<T>>*> listeners; // list of listeners to this service
  AlgoExecutionServiceListener<T>* algoexecservicelistener;
  double spread;
  long count;

public:
    // ctor
    AlgoExecutionService();
    // dtor
    ~AlgoExecutionService() = default;
    
    // Get data on our service given a key
    AlgoExecution<T>& GetData(string key);
    
    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(AlgoExecution<T>& data) override;
    
    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<AlgoExecution<T>> *listener) override;
    
    // Get all listeners on the Service.
    const vector< ServiceListener<AlgoExecution<T>>* >& GetListeners() const override;
    
    // Get the special listener for algo execution service
    AlgoExecutionServiceListener<T>* GetAlgoExecutionServiceListener();

    // Execute an algo order on a market, called by AlgoExecutionServiceListener to subscribe data from Algo Market Data Service to Algo Execution Service
    void AlgoExecuteOrder(OrderBook<T>& _orderBook);
    
};

template<typename T>
AlgoExecutionService<T>::AlgoExecutionService()
{
  count = 0;
  algoexecservicelistener = new AlgoExecutionServiceListener<T>(this); // listener related to this server
}

template<typename T>
AlgoExecution<T>& AlgoExecutionService<T>::GetData(string key)
{
  return algoExecutionMap[key];
}

/**
 * OnMessage() used to be called by input connector to subscribe data
 * no need to implement here.
 */
template<typename T>
void AlgoExecutionService<T>::OnMessage(AlgoExecution<T>& data)
{
}

template<typename T>
void AlgoExecutionService<T>::AddListener(ServiceListener<AlgoExecution<T>> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<AlgoExecution<T>>* >& AlgoExecutionService<T>::GetListeners() const
{
  return listeners;
}

template<typename T>
AlgoExecutionServiceListener<T>* AlgoExecutionService<T>::GetAlgoExecutionServiceListener()
{
  return algoexecservicelistener;
}

/**
 * Similar to AddExecutionOrder in executionservice.hpp
 * Execute an algo order on a market, called by AlgoExecutionServiceListener to subscribe data from Algo Market Data Service to Algo Execution Service
 * 1. Store the listened market orderbook data into algo execution map
 * 2. Flow the data to listeners
 */
template<typename T>
void AlgoExecutionService<T>::AlgoExecuteOrder(OrderBook<T>& _orderBook)
{
  // get the order book data
  T product = _orderBook.GetProduct();
  string key = product.GetProductId();
  string orderId = "Algo" + GenerateRandomId(11);
  string parentOrderId = "AlgoParent" + GenerateRandomId(5);

  // get the best bid and offer order and their corresponding price and quantity
  BidOffer bidOffer = _orderBook.GetBestBidOffer();
  Order bid = bidOffer.GetBidOrder();
  Order offer = bidOffer.GetOfferOrder();
  double bidPrice = bid.GetPrice();
  double offerPrice = offer.GetPrice();
  long bidQuantity = bid.GetQuantity();
  long offerQuantity = offer.GetQuantity();

  PricingSide side;
  double price;
  long quantity; 
  // only agressing when the spread is at its tightest (1/128)
  if (offerPrice-bidPrice <= 1.0/128.0){
    // alternating between bid and offer 
    // taking the opposite side of the book to cross the spread, i.e., market order
    if (count % 2 == 0) {
      side = BID;
      price = offerPrice; // BUY order takes best ask price
      quantity = bidQuantity;
    } else {
      side = OFFER;
      price = bidPrice; // SELL order takes best bid price
      quantity = offerQuantity;
    }
  }

  // update the count
  count++;

  // Create the execution order
  long visibleQuantity = quantity;
  long hiddenQuantity = 0;
  bool isChildOrder = false;
  OrderType orderType = MARKET; // market order
  ExecutionOrder<T> executionOrder(product, side, orderId, orderType, price, visibleQuantity, hiddenQuantity, parentOrderId, isChildOrder);

  // Create the algo execution
  Market market = BROKERTEC;
  AlgoExecution<T> algoExecution(executionOrder, market);

  // update the algo execution map
  if (algoExecutionMap.find(key) != algoExecutionMap.end()) {algoExecutionMap.erase(key);}
  algoExecutionMap.insert(pair<string, AlgoExecution<T>> (key, algoExecution));

  // flow the data to listeners
  for (auto& l : listeners) {
    l -> ProcessAdd(algoExecution);
  }
}


/**
* Algo Execution Service Listener subscribing data from Market Data Service to Algo Execution Service.
* Type T is the product type.
*/
template<typename T>
class AlgoExecutionServiceListener : public ServiceListener<OrderBook<T>>
{
private:
  AlgoExecutionService<T>* service;

public:
    // ctor
    AlgoExecutionServiceListener(AlgoExecutionService<T>* _service);
    // dtor
    ~AlgoExecutionServiceListener()=default;
    
    // Listener callback to process an add event to the Service
    void ProcessAdd(OrderBook<T> &data) override;
    
    // Listener callback to process a remove event to the Service
    void ProcessRemove(OrderBook<T> &data) override;
    
    // Listener callback to process an update event to the Service
    void ProcessUpdate(OrderBook<T> &data) override;
    
};

template<typename T>
AlgoExecutionServiceListener<T>::AlgoExecutionServiceListener(AlgoExecutionService<T>* _service)
{
  service = _service;
}

/**
 * ProcessAdd() method is used by listener to subscribe data from Market Data Service to Algo Execution Service.
 * It calls AlgoExecuteOrder() method, change the data type from OrderBook<T> to AlgoExecution<T> and notify the listeners.
 */
template<typename T>
void AlgoExecutionServiceListener<T>::ProcessAdd(OrderBook<T> &data)
{
  service->AlgoExecuteOrder(data);
}

template<typename T>
void AlgoExecutionServiceListener<T>::ProcessRemove(OrderBook<T> &data)
{
}

template<typename T>
void AlgoExecutionServiceListener<T>::ProcessUpdate(OrderBook<T> &data)
{
}


#endif