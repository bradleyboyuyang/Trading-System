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
  vector<ServiceListener<Price<T>>*> listeners; // list of listeners to this service
  PricingConnector<T>* connector; // connector related to this server
  
public:
  // ctor
  PricingService();
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
  PricingConnector<T>* GetConnector();

};

template<typename T>
PricingService<T>::PricingService()
{
  connector = new PricingConnector<T>(this); // connector related to this server
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
PricingConnector<T>* PricingService<T>::GetConnector()
{
  return connector;
}


/**
 * PricingConnector: an inbound connector that subscribes data from socket to pricing service.
 * Type T is the product type.
 */
template<typename T>
class PricingConnector : public Connector<Price<T>>
{
private:
  PricingService<T>* service; 

public:
  // ctor
  PricingConnector(PricingService<T>* _service);
  // dtor
  ~PricingConnector()=default;

  // Publish data to the Connector
  // If subscribe-only, this does nothing
  void Publish(Price<T> &data) override;

  // Subscribe data
  void Subscribe(const string& dataFile);

};

template<typename T>
PricingConnector<T>::PricingConnector(PricingService<T>* _service)
: service(_service)
{
}

// inbound connector, does nothing
template <typename T>
void PricingConnector<T>::Publish(Price<T> &data)
{ 
}

template<typename T>
void PricingConnector<T>::Subscribe(const string& dataFile)
{
  // read data from file
  ifstream data(dataFile.c_str());
  if (data.is_open())
  {
    string line;
    // skip the first line
    getline(data, line);
    while (getline(data, line))
    {
      // split the line into a vector of strings
      vector<string> lineData;
      stringstream ss(line); // turn the string into a stream
      string word;
      while (getline(ss, word, ','))
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

      // flows data to pricing service
      service->OnMessage(price);
    }
  }
  else
  {
    // throw an error if the file is not open
    cout << std::make_error_condition(std::errc::no_such_file_or_directory).message() << endl;
  }
}




#endif
