/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Boyu Yang
 */
#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <string>
#include <map>
#include <fstream>
#include "soa.hpp"

/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template<typename T>
class Price
{

public:

  // ctor for a price
  Price() = default;
  Price(const T &_product, double _mid, double _bidOfferSpread);

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
  const T& product;
  double mid;
  double bidOfferSpread;

};

template<typename T>
Price<T>::Price(const T &_product, double _mid, double _bidOfferSpread) 
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
  os << "Price Object (Product: " << price.product << ", Mid Price: " << price.mid << ", Bid/Offer Spread: " << price.bidOfferSpread << ")" << endl;
  return os;
}


// forward declaration of PricingConnector
template<typename T>
class PricingConnector;


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
  PricingConnector<T>* connector; // connector to interact with external data
  vector<ServiceListener<Price<T>>*> listeners; // list of listeners to this service
  
public:
  // ctor
  PricingService() = default;
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
};



template<typename T>
Price<T>& PricingService<T>::GetData(string key)
{
  return priceMap[key];
}

template<typename T>
void PricingService<T>::OnMessage(Price<T> &data)
{
  string key = data.GetProduct().GetProductId();
  priceMap[key] = data;
  for(auto& listener : listeners)
  {
    listener->ProcessAdd(data);
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




/**
 * Connector to a pricing source.
 * Type T is the product type.
 */
template<typename T>
class PricingConnector : public Connector<Price<T>>
{
private:
  PricingService<T>* service; // service to publish data to
  string file; // file name to read data from
  ifstream data; // data stream

public:
  // ctor
  PricingConnector(PricingService<T>* _service, string _file);
  // dtor
  ~PricingConnector();

  // Publish data to the Connector
  void Publish(Price<T> &data) override;

  // read data from file
  void ReadData();
};


template<typename T>
PricingConnector<T>::PricingConnector(PricingService<T>* _service, string _file)
: service(_service), file(_file)
{
  data.open(file);
  if(!data.is_open())
  {
    throw runtime_error("Cannot open file " + file);
  }
}

template<typename T>
PricingConnector<T>::~PricingConnector()
{
  data.close();
}

template<typename T>
void PricingConnector<T>::Publish(Price<T> &data)
{
  service->OnMessage(data);
}

template<typename T>
void PricingConnector<T>::ReadData()
{
  string line;
  while(getline(data, line))
  {
    stringstream ss(line);
    string productId;
    double mid, bidOfferSpread;
    ss >> productId >> mid >> bidOfferSpread;
    Price<T> price(T(productId), mid, bidOfferSpread);
    Publish(price);
  }
}


#endif
