/**
 * streamingservice.hpp
 * Defines the data types and Service for price streams.
 *
 * @author Boyu Yang
 */
#ifndef STREAMING_SERVICE_HPP
#define STREAMING_SERVICE_HPP

#include "soa.hpp"
#include "algostreamingservice.hpp"

/**
 * Forward declaration of StreamingServiceConnector
 * As described, streaming service needs a publish-only connector to publish streams (which notify the listener and print the execution data)
 * into a separate process which listens to the streams on the socket via its own Connector.
 * Type T is the product type.
 */
template<typename T>
class StreamingServiceConnector;


/**
 * Streaming service to publish two-way prices.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class StreamingService : public Service<string,PriceStream <T> >
{
private:
  map<string, PriceStream<T>> priceStreamMap; // store price stream data keyed by product identifier
  vector<ServiceListener<PriceStream<T>>*> listeners; // list of listeners to this service
  StreamingServiceConnector<T>* connector; // connector related to this server

public:
  // ctor and dtor
  StreamingService();
  ~StreamingService()=default;

  // Get data on our service given a key
  PriceStream<T>& GetData(string key) override;

  // The callback that a Connector should invoke for any new or updated data
  void OnMessage(PriceStream<T>& data) override;

  // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
  void AddListener(ServiceListener<PriceStream<T>> *listener) override;

  // Get all listeners on the Service.
  const vector< ServiceListener<PriceStream<T>>* >& GetListeners() const override;

  // Get the connector
  StreamingServiceConnector<T>* GetConnector();

  // Publish two-way prices (called by the publish-only connector to publish streams)
  void PublishPrice(const PriceStream<T>& priceStream);

  // called by streaming service listener to subscribe data from algo streaming service
  void AddPriceStream(const AlgoStream<T>& algoStream);

};

template<typename T>
StreamingService<T>::StreamingService()
{
}

template<typename T>
PriceStream<T>& StreamingService<T>::GetData(string key)
{
  return priceStreamMap[key];
}

/**
 * OnMessage() used to be called by input connector to subscribe data
 * no need to implement here.
 */
template<typename T>
void StreamingService<T>::OnMessage(PriceStream<T>& data)
{
}

template<typename T>
void StreamingService<T>::AddListener(ServiceListener<PriceStream<T>> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<PriceStream<T>>* >& StreamingService<T>::GetListeners() const
{
  return listeners;
}

template<typename T>
StreamingServiceConnector<T>* StreamingService<T>::GetConnector()
{
  return connector;
}

// call the publish-only connector to publish streams
template<typename T>
void StreamingService<T>::PublishPrice(const PriceStream<T>& priceStream)
{
  connector->Publish(priceStream);
}

// called by streaming service listener to subscribe data from algo streaming service
template<typename T>
void StreamingService<T>::AddPriceStream(const AlgoStream<T>& algoStream)
{
  PriceStream<T> priceStream = algoStream.GetPriceStream();
  string key = priceStream.GetProduct().GetProductId();
  // update the price stream map
  if (priceStreamMap.find(key) != priceStreamMap.end()) {priceStreamMap.erase(key);}
  priceStreamMap.insert(pair<string, PriceStream<T> > (key, priceStream));

  // flow the data to listeners
  for (auto& l : listeners) {
      l -> ProcessAdd(priceStream);
  }
}

/**
 * StreamingServiceConnector: publish data to streaming service.
 * Type T is the product type.
 */
template<typename T>
class StreamingServiceConnector : public Connector<PriceStream<T>>
{
private:
  StreamingService<T>* service;

public:
  // ctor
  StreamingServiceConnector(StreamingService<T>* _service);
  // dtor
  ~StreamingServiceConnector()=default;

  // Publish data to the Connector
  void Publish(const PriceStream<T>& data) override;

};

template<typename T>
StreamingServiceConnector<T>::StreamingServiceConnector(StreamingService<T>* _service)
{
  service = _service;
}

/**
 * Publish() method is used by the publish-only connector to publish streams.
 */
template<typename T>
void StreamingServiceConnector<T>::Publish(const PriceStream<T>& data)
{
  // print the price stream data
  T product = data.GetProduct();
  string productId = product.GetProductId();
  PriceStreamOrder bid = data.GetBidOrder();
  PriceStreamOrder offer = data.GetOfferOrder();

  cout << "Price Stream " << "(Product " << productId << "): \n"
      <<"\tBid\t"<<"Price: "<<bid.GetPrice()<<"\tVisibleQuantity: "<<bid.GetVisibleQuantity()
      <<"\tHiddenQuantity: "<<bid.GetHiddenQuantity()<<"\n"
      <<"\tAsk\t"<<"Price: "<<offer.GetPrice()<<"\tVisibleQuantity: "<<offer.GetVisibleQuantity()
      <<"\tHiddenQuantity: "<<offer.GetHiddenQuantity()<<"\n";
}

/**
* Streaming Service Listener subscribing data from Algo Streaming Service to Streaming Service.
* Type T is the product type.
*/
template<typename T>
class StreamingServiceListener : public ServiceListener<AlgoStream<T>>
{
private:
  StreamingService<T>* streamingService;

public:
  // ctor
  StreamingServiceListener(StreamingService<T>* _streamingService);

  // Listener callback to process an add event to the Service
  void ProcessAdd(AlgoStream<T>& data) override;

  // Listener callback to process a remove event to the Service
  void ProcessRemove(AlgoStream<T>& data) override;

  // Listener callback to process an update event to the Service
  void ProcessUpdate(AlgoStream<T>& data) override;

};

template<typename T>
StreamingServiceListener<T>::StreamingServiceListener(StreamingService<T>* _streamingService)
{
  streamingService = _streamingService;
}

/**
 * ProcessAdd() method is used by listener to subscribe data from Algo Streaming Service to Streaming Service.
 * It calls PublishPrice() method and the AddPriceStream() method 
 */
template<typename T>
void StreamingServiceListener<T>::ProcessAdd(AlgoStream<T>& data)
{
  // save algo stream info into streaming service
  // directly pass in AlgoStream<T> type and transit to PriceStream<T> type inside the function
  streamingService->AddPriceStream(data);

  // call the connector to publish price streams
  PriceStream<T> priceStream = data.GetPriceStream();
  streamingService->PublishPrice(priceStream);
}

template<typename T>
void StreamingServiceListener<T>::ProcessRemove(AlgoStream<T>& data)
{
}

template<typename T>
void StreamingServiceListener<T>::ProcessUpdate(AlgoStream<T>& data)
{
}

#endif
