/**
 * riskservice.hpp
 * Defines the data types and Service for fixed income risk.
 *
 * @author Boyu Yang
 */
#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "soa.hpp"
#include "positionservice.hpp"
#include "utils.hpp"

/**
 * PV01 risk.
 * Type T is the product type.
 */
template<typename T>
class PV01
{

public:

  // Default ctor
  PV01() = default;
  // ctor for a PV01 value
  PV01(const T &_product, double _pv01, long _quantity);

  // Get the product on this PV01 value
  const T& GetProduct() const;

  // Get the PV01 value
  double GetPV01() const;

  // Get the quantity that this risk value is associated with
  long GetQuantity() const;

  // Add quantity associated with this risk value
  void AddQuantity(long _quantity);

  // Object printer
  template<typename U>
  friend ostream& operator<<(ostream& os, const PV01<U>& pv01);

private:
  T product;
  double pv01;
  long quantity;

};

template<typename T>
PV01<T>::PV01(const T &_product, double _pv01, long _quantity) :
  product(_product)
{
  pv01 = _pv01;
  quantity = _quantity;
}

template<typename T>
const T& PV01<T>::GetProduct() const
{
  return product;
}

template<typename T>
double PV01<T>::GetPV01() const
{
  return pv01;
}

template<typename T>
long PV01<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
void PV01<T>::AddQuantity(long _quantity)
{
  quantity += _quantity;
}

template<typename T>
ostream& operator<<(ostream& os, const PV01<T>& pv01)
{
  T product = pv01.GetProduct();
	string _product = product.GetProductId();
  double pv01_value = pv01.GetPV01();
  long quantity = pv01.GetQuantity();
	string _pv01 = to_string(pv01_value);
	string _quantity = to_string(quantity);

	vector<string> _strings;
	_strings.push_back(_product);
	_strings.push_back(_pv01);
	_strings.push_back(_quantity);
  string _str = join(_strings, ",");
  os << _str;
  return os;
}

/**
 * A bucket sector to bucket a group of securities.
 * We can then aggregate bucketed risk to this bucket.
 * Type T is the product type.
 */
template<typename T>
class BucketedSector
{
public:
  // ctor for a bucket sector
  BucketedSector(const vector<T> &_products, string _name);

  // Get the products associated with this bucket
  const vector<T>& GetProducts() const;

  // Get the name of the bucket
  const string& GetName() const;

private:
  vector<T> products;
  string name;

};

template<typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) :
  products(_products)
{
  name = _name;
}

template<typename T>
const vector<T>& BucketedSector<T>::GetProducts() const
{
  return products;
}

template<typename T>
const string& BucketedSector<T>::GetName() const
{
  return name;
}

// forward declaration of RiskServiceListener
template<typename T>
class RiskServiceListener;


/**
 * Risk Service to vend out risk for a particular security and across a risk bucketed sector.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class RiskService : public Service<string,PV01 <T> >
{
private:
  vector<ServiceListener<PV01<T>>*> listeners;
  map<string, PV01<T>> pv01Map;
  RiskServiceListener<T>* riskservicelistener;

public:
  // ctor and dtor
  RiskService();
  ~RiskService()=default;

  // Get data on our service given a key
  PV01<T>& GetData(string key);

  // The callback that a Connector should invoke for any new or updated data
  void OnMessage(PV01<T> &data);

  // Add a listener to the Service for callbacks on add, remove, and update events
  // for data to the Service.
  void AddListener(ServiceListener<PV01<T>> *listener);

  // Get all listeners on the Service.
  const vector< ServiceListener<PV01<T>>* >& GetListeners() const;

  // Get the special listener for risk service
  RiskServiceListener<T>* GetRiskServiceListener();

  // Add a position that the service will risk
  void AddPosition(Position<T> &position);

  // Get the bucketed risk for the bucket sector
  const PV01< BucketedSector<T> >& GetBucketedRisk(const BucketedSector<T> &sector) const;

};

template<typename T>
RiskService<T>::RiskService()
{
  riskservicelistener = new RiskServiceListener<T>(this);
}

template<typename T>
PV01<T>& RiskService<T>::GetData(string key)
{
  return pv01Map[key];
}

/**
 * OnMessage() used to be called by connector to subscribe data
 * no need to implement here.
 */
template<typename T>
void RiskService<T>::OnMessage(PV01<T> &data)
{
}

template<typename T>
void RiskService<T>::AddListener(ServiceListener<PV01<T>> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<PV01<T>>* >& RiskService<T>::GetListeners() const
{
  return listeners;
}

template<typename T>
RiskServiceListener<T>* RiskService<T>::GetRiskServiceListener()
{
  return riskservicelistener;
}

template<typename T>
void RiskService<T>::AddPosition(Position<T> &position)
{
  T product = position.GetProduct();
  string productId = product.GetProductId();
  long quantity = position.GetAggregatePosition();
  // note: this gives the PV01 value for a single unit
  double pv01Val = getPV01(productId);

  // create a PV01 object and publish it to the service
  PV01<T> pv01(product, pv01Val, quantity);
  if (pv01Map.find(productId) != pv01Map.end()){
    pv01Map[productId].AddQuantity(quantity);
  }else{
    pv01Map.insert(pair<string, PV01<T>>(productId, pv01));
  }

  // notify listeners
  for(auto& listener : listeners)
    listener->ProcessAdd(pv01);
}

template<typename T>
const PV01<BucketedSector<T>>& RiskService<T>::GetBucketedRisk(const BucketedSector<T> &sector) const
{
  // create a bucketed sector object and publish it to the service
  vector<T>& products = sector.GetProducts();
  string name = sector.GetName();
  double pv01Val = 0.0;
  long quantity = 0;
  for (auto& product : products){
    string productId = product.GetProductId();
    if (pv01Map.find(productId) != pv01Map.end()){
      // total pv01 value for the sector is the weighted average
      pv01Val += pv01Map[productId].GetPV01()*pv01Map[productId].GetQuantity();
      // total quantity for the sector
      quantity += pv01Map[productId].GetQuantity();
    }
  }
  // note: for PV01 object of a sector, we store the total PV01 value instead of a single unit
  PV01<BucketedSector<T>> pv01(sector, pv01Val, quantity);
  return pv01;
}


/**
* Risk Service Listener subscribing data from Position Service to Risk Service.
* Type T is the product type.
*/
template<typename T>
class RiskServiceListener : public ServiceListener<Position<T>>
{
private:
  RiskService<T>* riskservice;

public:
  // ctor and dtor
  RiskServiceListener(RiskService<T>* _riskservice);
  ~RiskServiceListener()=default;

  // Listener callback to process an add event to the Service
  void ProcessAdd(Position<T> &data);

  // Listener callback to process a remove event to the Service
  void ProcessRemove(Position<T> &data);

  // Listener callback to process an update event to the Service
  void ProcessUpdate(Position<T> &data);

};

template<typename T>
RiskServiceListener<T>::RiskServiceListener(RiskService<T>* _riskservice)
{
  riskservice = _riskservice;
}

/**
 * ProcessAdd() method is used to transfer Position<T> data to PV01<T> data,
 * then publish the PV01<T> data to Risk Service.
 */
template<typename T>
void RiskServiceListener<T>::ProcessAdd(Position<T> &data)
{
  riskservice->AddPosition(data);
}

template<typename T>
void RiskServiceListener<T>::ProcessRemove(Position<T> &data)
{
}

template<typename T>
void RiskServiceListener<T>::ProcessUpdate(Position<T> &data)
{
}



#endif
