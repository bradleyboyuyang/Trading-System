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

// object printer (needed by ExecutionServiceConnector)
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
  os << "Execution Order Object (Product: " << order.product << ", Side: " << order.side << ", Order ID: " << order.orderId << ", Order Type: " << order.orderType << ", Price: " << order.price << ", Visible Quantity: " << order.visibleQuantity << ", Hidden Quantity: " << order.hiddenQuantity << ", Parent Order ID: " << order.parentOrderId << ", Is Child Order: " << order.isChildOrder << ")" << endl;
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
    double bidPrice;
    double offerPrice;

public:
    // ctor
    AlgoExecution() = default;
    AlgoExecution(const ExecutionOrder<T>& _executionOrder, Market _market, double _bidPrice, double _offerPrice);

    // Get the execution order
    const ExecutionOrder<T>& GetExecutionOrder() const;

    // Get the market
    Market GetMarket() const;

    // Get the bid price
    double GetBidPrice() const;

    // Get the offer price
    double GetOfferPrice() const;
};












#endif