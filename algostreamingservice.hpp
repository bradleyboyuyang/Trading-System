/**
 * algostreamingservice.hpp
 * Defines the data types and Service for algo streams.
 *
 * @author Boyu Yang
 */

#ifndef ALGOSTREAMING_SERVICE_HPP
#define ALGOSTREAMING_SERVICE_HPP

#include "soa.hpp"
#include "utils.hpp"
#include "pricingservice.hpp"
#include "marketdataservice.hpp" // for PricingSide definition

/**
 * A price stream order with price and quantity (visible and hidden)
 */
class PriceStreamOrder
{

public:

  // ctor for an order
  PriceStreamOrder() = default; // needed for map data structure later
  PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);

  // The side on this order
  PricingSide GetSide() const;

  // Get the price on this order
  double GetPrice() const;

  // Get the visible quantity on this order
  long GetVisibleQuantity() const;

  // Get the hidden quantity on this order
  long GetHiddenQuantity() const;

  // object printer
  friend ostream& operator<<(ostream& os, const PriceStreamOrder& order);

private:
  double price;
  long visibleQuantity;
  long hiddenQuantity;
  PricingSide side;

};

PriceStreamOrder::PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side)
{
  price = _price;
  visibleQuantity = _visibleQuantity;
  hiddenQuantity = _hiddenQuantity;
  side = _side;
}

double PriceStreamOrder::GetPrice() const
{
  return price;
}

long PriceStreamOrder::GetVisibleQuantity() const
{
  return visibleQuantity;
}

long PriceStreamOrder::GetHiddenQuantity() const
{
  return hiddenQuantity;
}

PricingSide PriceStreamOrder::GetSide() const
{
  return side;
}

ostream& operator<<(ostream& os, const PriceStreamOrder& order)
{
	string _price = convertPrice(order.GetPrice());
	string _visibleQuantity = to_string(order.GetVisibleQuantity());
	string _hiddenQuantity = to_string(order.GetHiddenQuantity());
  PricingSide side = order.GetSide();
	string _side;
	switch (side)
	{
	case BID:
		_side = "BID";
		break;
	case OFFER:
		_side = "OFFER";
		break;
	}

	vector<string> _strings;
	_strings.push_back(_price);
	_strings.push_back(_visibleQuantity);
	_strings.push_back(_hiddenQuantity);
	_strings.push_back(_side);
  string _str = join(_strings, ",");
  os << _str;
  return os;
}

/**
 * Price Stream with a two-way market.
 * Type T is the product type.
 */
template<typename T>
class PriceStream
{

public:

  // ctor
  PriceStream() = default; // needed for map data structure later
  PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder);

  // Get the product
  const T& GetProduct() const;

  // Get the bid order
  const PriceStreamOrder& GetBidOrder() const;

  // Get the offer order
  const PriceStreamOrder& GetOfferOrder() const;

  // object printer
  template<typename U>
  friend ostream& operator<<(ostream& os, const PriceStream<U>& priceStream);

private:
  T product;
  PriceStreamOrder bidOrder;
  PriceStreamOrder offerOrder;

};

template<typename T>
PriceStream<T>::PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder) :
  product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

template<typename T>
const T& PriceStream<T>::GetProduct() const
{
  return product;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetBidOrder() const
{
  return bidOrder;
}

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetOfferOrder() const
{
  return offerOrder;
}

template<typename T>
ostream& operator<<(ostream& os, const PriceStream<T>& priceStream)
{
  T product = priceStream.GetProduct();
  string productId = product.GetProductId();
  PriceStreamOrder bidOrder = priceStream.GetBidOrder();
  PriceStreamOrder offerOrder = priceStream.GetOfferOrder();
  os << productId << "," << bidOrder << "," << offerOrder;
  return os;
}

/**
* An algo streaming that process algo streaming.
* Type T is the product type.
*/
template<typename T>
class AlgoStream
{
private:
    PriceStream<T> priceStream;

public:
    // ctor for an order
    AlgoStream() = default; // needed for map data structure later
    AlgoStream(const PriceStream<T> &_priceStream);

    // Get the price stream
    const PriceStream<T>& GetPriceStream() const;

};

template<typename T>
AlgoStream<T>::AlgoStream(const PriceStream<T> &_priceStream) :
  priceStream(_priceStream)
{
}

template<typename T>
const PriceStream<T>& AlgoStream<T>::GetPriceStream() const
{
  return priceStream;
}

// forward declaration
template<typename T>
class AlgoStreamingServiceListener;


/**
 * Algo Streaming Service to publish algo streams.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class AlgoStreamingService : public Service<string,AlgoStream <T> >
{
private:
  map<string, AlgoStream<T>> algoStreamMap; // store algo stream data keyed by product identifier
  vector<ServiceListener<AlgoStream<T>>*> listeners; // list of listeners to this service
  AlgoStreamingServiceListener<T>* algostreamlistener;
  long count;

public:
    // ctor and dtor
    AlgoStreamingService();
    ~AlgoStreamingService()=default;
    
    // Get data on our service given a key
    AlgoStream<T>& GetData(string key) override;
    
    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(AlgoStream<T>& data) override;
    
    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<AlgoStream<T>> *listener) override;
    
    // Get all listeners on the Service.
    const vector< ServiceListener<AlgoStream<T>>* >& GetListeners() const override;
    
    // Get the special listener for algo streaming service
    AlgoStreamingServiceListener<T>* GetAlgoStreamingListener();

    // Publish algo streams (called by algo streaming service listener to subscribe data from pricing service)
    void PublishAlgoStream(const Price<T>& price);
    
};

template<typename T>
AlgoStreamingService<T>::AlgoStreamingService()
{
  algostreamlistener = new AlgoStreamingServiceListener<T>(this);
}

template<typename T>
AlgoStream<T>& AlgoStreamingService<T>::GetData(string key)
{
  return algoStreamMap[key];
}

/**
 * OnMessage() used to be called by input connector to subscribe data
 * no need to implement here.
 */
template<typename T>
void AlgoStreamingService<T>::OnMessage(AlgoStream<T>& data)
{
}

template<typename T>
void AlgoStreamingService<T>::AddListener(ServiceListener<AlgoStream<T>> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<AlgoStream<T>>* >& AlgoStreamingService<T>::GetListeners() const
{
  return listeners;
}

template<typename T>
AlgoStreamingServiceListener<T>* AlgoStreamingService<T>::GetAlgoStreamingListener()
{
  return algostreamlistener;
}

/**
 * PublishAlgoStream() method is used by listener to subscribe data from pricing service.
 * It transites the data structure from Price<T> to AlgoStream<T>, save in the service, and notify the listeners.
 */
template<typename T>
void AlgoStreamingService<T>::PublishAlgoStream(const Price<T>& price)
{
  T product = price.GetProduct();
  string key = product.GetProductId();
  double mid = price.GetMid();
  double spread = price.GetBidOfferSpread();
  double bidPrice = mid - spread/2;
  double offerPrice = mid + spread/2;
  // alternate visible size between 1000000 and 2000000
  long visibleQuantity = (count % 2 == 0) ? 1000000 : 2000000;
  // hidden size is twice the visible size
  long hiddenQuantity = visibleQuantity * 2;

  count++;

  // create bid order and offer order
  PriceStreamOrder bidOrder(bidPrice, visibleQuantity, hiddenQuantity, BID);
  PriceStreamOrder offerOrder(offerPrice, visibleQuantity, hiddenQuantity, OFFER);
  // create price stream
  PriceStream<T> priceStream(product, bidOrder, offerOrder);
  // create algo stream
  AlgoStream<T> algoStream(priceStream);

  // update the algo stream map
  if (algoStreamMap.find(key) != algoStreamMap.end()) {algoStreamMap.erase(key);}
  algoStreamMap.insert(pair<string, AlgoStream<T>> (key, algoStream));

  // notify the listeners
  for (auto& listener : listeners)
  {
    listener->ProcessAdd(algoStream);
  }
}

/**
 * Algo Streaming Service Listener to subscribe data from pricing service.
 * Type T is the product type.
 */
template<typename T>
class AlgoStreamingServiceListener : public ServiceListener<Price<T>>
{
private:
  AlgoStreamingService<T>* algoStreamingService;

public:
    // ctor
    AlgoStreamingServiceListener(AlgoStreamingService<T>* _algoStreamingService);
    
    // Listener callback to process an add event to the Service
    void ProcessAdd(Price<T>& price) override;
    
    // Listener callback to process a remove event to the Service
    void ProcessRemove(Price<T>& price) override;
    
    // Listener callback to process an update event to the Service
    void ProcessUpdate(Price<T>& price) override;
    
};

template<typename T>
AlgoStreamingServiceListener<T>::AlgoStreamingServiceListener(AlgoStreamingService<T>* _algoStreamingService)
{
  algoStreamingService = _algoStreamingService;
}

/**
 * ProcessAdd() method is used by listener to subscribe data from pricing service.
 * It calls GetPriceStream() method, change the data type from Price<T> to AlgoStream<T> and notify the listeners.
 */

template<typename T>
void AlgoStreamingServiceListener<T>::ProcessAdd(Price<T>& price)
{
  algoStreamingService->PublishAlgoStream(price);
}

template<typename T>
void AlgoStreamingServiceListener<T>::ProcessRemove(Price<T>& price)
{
}

template<typename T>
void AlgoStreamingServiceListener<T>::ProcessUpdate(Price<T>& price)
{
}


#endif