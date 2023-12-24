/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @author Boyu Yang
 */
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{

public:

  // ctor for a position
  Position() = default;
  Position(const T &productId);

  // Get the product
  const T& GetProduct() const;

  // Get the position quantity
  long GetPosition(string &book);

  // Get the aggregate position
  long GetAggregatePosition();

  //  send position to risk service through listener
  void AddPosition(string &book, long position);

  // object printer
  template<typename U>
  friend ostream& operator<<(ostream& os, const Position<U>& position);

private:
  T product;
  map<string,long> bookPositionMap;

};


template<typename T>
Position<T>::Position(const T &productId) :
  product(productId)
{
}

template<typename T>
const T& Position<T>::GetProduct() const
{
  return product;
}

template<typename T>
long Position<T>::GetPosition(string &book)
{
  return bookPositionMap[book];
}

template<typename T>
long Position<T>::GetAggregatePosition()
{
  long sum = 0;
  for (auto it = bookPositionMap.begin(); it != bookPositionMap.end(); ++it)
  {
    sum += it->second;
  }
  return sum;
}

template<typename T>
void Position<T>::AddPosition(string &book, long position)
{
  if (bookPositionMap.find(book) == bookPositionMap.end())
  {
    bookPositionMap.insert(pair<string,long>(book,position));
  }
  else
  {
    bookPositionMap[book] += position;
  }
}

template<typename T>
ostream& operator<<(ostream& os, const Position<T>& position)
{
  T product = position.GetProduct();
  string productId = product.GetProductId();
	vector<string> _positions;
	for (auto& p : position.bookPositionMap)
	{
		string _book = p.first;
		string _position = to_string(p.second);
		_positions.push_back(_book);
		_positions.push_back(_position);
	}
	vector<string> _strings;
	_strings.push_back(productId);
	_strings.insert(_strings.end(), _positions.begin(), _positions.end());
  string _str = join(_strings, ",");
  os << _str;
  return os;
}

// Pre-declaration of a listener used to subscribe data from trade booking service
template<typename T>
class PositionServiceListener;

/**
 * Position Service to manage positions across multiple books and securities.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string,Position <T> >
{
private:
  map<string,Position<T>> positionMap;
  vector<ServiceListener<Position<T>>*> listeners;
  PositionServiceListener<T>* positionlistener;

public:
  // ctor and dtor
  PositionService();
  ~PositionService()=default;

  // Get data on our service given a key
  Position<T>& GetData(string key);

  // The callback that a Connector should invoke for any new or updated data
  void OnMessage(Position<T> &data);

  // Add a listener to the Service for callbacks on add, remove, and update events
  // for data to the Service.
  void AddListener(ServiceListener<Position<T>> *listener);

  // Get all listeners on the Service.
  const vector<ServiceListener<Position<T>>*>& GetListeners() const;

  // Get the special listener for trade booking service
  PositionServiceListener<T>* GetPositionListener();

  // Add a trade to the service
  void AddTrade(const Trade<T> &trade);

};

template<typename T>
PositionService<T>::PositionService()
{
  positionlistener = new PositionServiceListener<T>(this);
}

template<typename T>
Position<T>& PositionService<T>::GetData(string key)
{
  return positionMap[key];
}

/**
 * OnMessage() used to be called by an input connector to subscribe data from socket
 * no need to implement here.
 */
template<typename T>
void PositionService<T>::OnMessage(Position<T> &data)
{
}

template<typename T>
void PositionService<T>::AddListener(ServiceListener<Position<T>> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<Position<T>>*>& PositionService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
PositionServiceListener<T>* PositionService<T>::GetPositionListener()
{
  return positionlistener;
}

/**
 * AddTrade() method is used to subscribe data from trade booking service,
 * transfer Trade<T> data to Position<T> data and save it to the service.
 */
template<typename T>
void PositionService<T>::AddTrade(const Trade<T> &trade)
{
  T product = trade.GetProduct();
  string productId = product.GetProductId();
  string book = trade.GetBook();
  long quantity = (trade.GetSide() == BUY) ? trade.GetQuantity() : -trade.GetQuantity();
  if (positionMap.find(productId) == positionMap.end())
  {
    Position<T> position(product);
    position.AddPosition(book,quantity);
    positionMap.insert(pair<string,Position<T>>(productId,position));
  }
  else
  {
    positionMap[productId].AddPosition(book,quantity);
  }
  for (auto& listener: listeners)
  {
    listener->ProcessAdd(positionMap[productId]);
  }

}

/**
 * Listener class for PositionService.
 * Used to subscribe data from trade booking service instead of connector.
 * Type T is the product type.
 */
template<typename T>
class PositionServiceListener : public ServiceListener<Trade<T>>
{
private:
  PositionService<T>* positionservice;

public:
  // ctor
  PositionServiceListener(PositionService<T>* _positionservice);

  // Listener callback to process an add event to the Service
  void ProcessAdd(Trade<T> &data);

  // Listener callback to process a remove event to the Service
  void ProcessRemove(Trade<T> &data);

  // Listener callback to process an update event to the Service
  void ProcessUpdate(Trade<T> &data);

};

template<typename T>
PositionServiceListener<T>::PositionServiceListener(PositionService<T>* _positionservice)
{
  positionservice = _positionservice;
}

/**
 * ProcessAdd() method is used to subscribe data from trade booking service,
 * and then call AddTrade() method to add the trade to the service.
 */
template<typename T>
void PositionServiceListener<T>::ProcessAdd(Trade<T> &data)
{
  positionservice->AddTrade(data);
}

template<typename T>
void PositionServiceListener<T>::ProcessRemove(Trade<T> &data)
{
}

template<typename T>
void PositionServiceListener<T>::ProcessUpdate(Trade<T> &data)
{
}



#endif
