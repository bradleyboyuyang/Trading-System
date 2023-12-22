/**
 * historicaldataservice.hpp
 * historicaldataservice.hpp
 *
 * @author Boyu Yang
 * Defines the data types and Service for historical data.
 */
#ifndef HISTORICAL_DATA_SERVICE_HPP
#define HISTORICAL_DATA_SERVICE_HPP

#include "soa.hpp"
#include "streamingservice.hpp"
#include "riskservice.hpp"
#include "executionservice.hpp"
#include "inquiryservice.hpp"
#include "positionservice.hpp"
#include "utils.hpp"

enum ServiceType {POSITION, RISK, EXECUTION, STREAMING, INQUIRY};


// pre declaration
template<typename T>
class HistoricalDataConnector;
template<typename T>
class HistoricalDataServiceListener;


/**
 * Service for processing and persisting historical data to a persistent store.
 * Keyed on some persistent key.
 * Type T is the data type to persist.
 */
template<typename T>
class HistoricalDataService : Service<string,T>
{
private:
  map<string, T> dataMap; // store data keyed by some persistent key
  vector<ServiceListener<T>*> listeners; // list of listeners to this service
  HistoricalDataConnector<T>* connector; // connector related to this server
  ServiceType type; // type of the service
  HistoricalDataServiceListener<T>* historicalservicelistener; // listener to this service

public:
  // ctor and dtor
  HistoricalDataService(ServiceType _type);  
  ~HistoricalDataService()=default;

  // Get data on our service given a key
  T& GetData(string key) override;

  // The callback that a Connector should invoke for any new or updated data
  void OnMessage(T& data) override;

  // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
  void AddListener(ServiceListener<T> *listener) override;

  // Get all listeners on the Service.
  const vector< ServiceListener<T>* >& GetListeners() const override;

  // Get the special listener for historical data service
  HistoricalDataServiceListener<T>* GetHistoricalDataServiceListener();

  // Get the connector
  HistoricalDataConnector<T>* GetConnector();

  // Get the type of the service
	ServiceType GetServiceType() const;

  // Persist data to a store
  // call the connector to persist/publish data to an external store (such as KDB database)
  void PersistData(string persistKey, T& data);

};

template<typename T>
HistoricalDataService<T>::HistoricalDataService(ServiceType _type)
{
  type = _type;
  historicalservicelistener = new HistoricalDataServiceListener<T>(this); // listener related to this server
  connector = new HistoricalDataConnector<T>(this); // connector related to this server
}

template<typename T>
T& HistoricalDataService<T>::GetData(string key)
{
  return dataMap[key];
}

/**
 * OnMessage() used to be called by input connector to subscribe data
 * The service only has a publish-only connector and hence no need to implement here.
 */
template<typename T>
void HistoricalDataService<T>::OnMessage(T& data)
{
}

template<typename T>
void HistoricalDataService<T>::AddListener(ServiceListener<T> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<T>* >& HistoricalDataService<T>::GetListeners() const
{
  return listeners;
}

template<typename T>
HistoricalDataServiceListener<T>* HistoricalDataService<T>::GetHistoricalDataServiceListener()
{
  return historicalservicelistener;
}

template<typename T>
HistoricalDataConnector<T>* HistoricalDataService<T>::GetConnector()
{
  return connector;
}

template<typename T>
ServiceType HistoricalDataService<T>::GetServiceType() const
{
  return type;
}

// Historical data service listener subscribes data from position, risk, execution, streaming and inquiry services.
// call the connector to persist/publish data to an external store (such as KDB database)
// NOTE: since data from different services are keyed by different keys, we need to pass in the key as function parameter as well
template<typename T>
void HistoricalDataService<T>::PersistData(string persistKey, T& data)
{
  // save position/risk/execution/inquiry/streaming data to the service
  if (dataMap.find(persistKey) == dataMap.end())
    dataMap.insert(pair<string, T>(persistKey, data));
  else
    dataMap[persistKey] = data;

  // persist/publish data to an external data store
  connector->Publish(data);
}

/**
* Historical Data Connector publishing data from Historical Data Service.
* Type T is the data type to persist.
 */
template<typename T>
class HistoricalDataConnector : public Connector<T>
{
private:
  HistoricalDataService<T>* service;

public:
  // ctor
  HistoricalDataConnector(HistoricalDataService<T>* _service);
  // Publish-only connector, publish to external source
  void Publish(T& data);
};

template<typename T>
HistoricalDataConnector<T>::HistoricalDataConnector(HistoricalDataService<T>* _service)
: service(_service)
{
}

/**
 * Publish data to the Connector
 * call the connector to persist/publish data to an external store (such as KDB database)
 * for data from different services, obtain the string representation of these objects and vend out 
 * into positions.txt, risk.txt, executions.txt, allinquiries.txt, streaming.txt
 */
template<typename T>
void HistoricalDataConnector<T>::Publish(T& data)
{
  ServiceType type = service->GetServiceType();
  ofstream outFile;
  string fileName;
  switch (type)
  {
    case POSITION:
      fileName = "../res/positions.txt";
      break;
    case RISK:
      fileName = "../res/risk.txt";
      break;
    case EXECUTION:
      fileName = "../res/executions.txt";
      break;
    case STREAMING:
      fileName = "../res/streaming.txt";
      break;
    case INQUIRY:
      fileName = "../res/allinquiries.txt";
      break;
    default:
      break;
  }
  outFile.open(fileName, ios::app);
  if (outFile.is_open())
  {
    // need overloading operator<< for different data types
    outFile << getTime() << "," << data << endl;
  }
  outFile.close();
}

/**
* Historical Data Service Listener subscribing data to Historical Data.
* Type T is the data type to persist.
*/
template<typename T>
class HistoricalDataServiceListener : public ServiceListener<T>
{
private:
  HistoricalDataService<T>* service;

public:
  // ctor
  HistoricalDataServiceListener(HistoricalDataService<T>* _service);
  // Listener callback to process an add event to the Service
  void ProcessAdd(Position<Bond>& data);
  void ProcessAdd(PV01<Bond>& data);
  void ProcessAdd(PriceStream<Bond>& data);
  void ProcessAdd(ExecutionOrder<Bond>& data);
  void ProcessAdd(Inquiry<Bond>& data);

  // Listener callback to process a remove event to the Service
  void ProcessRemove(T& data) override;
  // Listener callback to process an update event to the Service
  void ProcessUpdate(T& data) override;
};

template<typename T>
HistoricalDataServiceListener<T>::HistoricalDataServiceListener(HistoricalDataService<T>* _service)
{
  service = _service;
}

/**
 * Historical data service listener subscribes data from position, risk, execution, streaming and inquiry services.
 * ProcessAdd() thus calls PersistData() method to let connector persist/publish data to external data store (such as a KDB database)
 * different services have different keys
 * if the service type is POISITION, RISK, STREAMING, the key is the product identifier
*/
template<typename T>
void HistoricalDataServiceListener<T>::ProcessAdd(Position<Bond>& data)
{
  string persistKey = data.GetProduct().GetProductId();
  service->PersistData(persistKey, data);
}

template<typename T>
void HistoricalDataServiceListener<T>::ProcessAdd(PV01<Bond>& data)
{
  string persistKey = data.GetProduct().GetProductId();
  service->PersistData(persistKey, data);
}

template<typename T>
void HistoricalDataServiceListener<T>::ProcessAdd(PriceStream<Bond>& data)
{
  string persistKey = data.GetProduct().GetProductId();
  service->PersistData(persistKey, data);
}

template<typename T>
void HistoricalDataServiceListener<T>::ProcessAdd(ExecutionOrder<Bond>& data)
{
    string persistKey = data.GetOrderId();
    service->PersistData(persistKey, data);
}

template<typename T>
void HistoricalDataServiceListener<T>::ProcessAdd(Inquiry<Bond>& data)
{
    string persistKey = data.GetInquiryId();
    service->PersistData(persistKey, data);
}


template<typename T>
void HistoricalDataServiceListener<T>::ProcessRemove(T& data)
{
}

template<typename T>
void HistoricalDataServiceListener<T>::ProcessUpdate(T& data)
{
}

#endif
