/**
 * executionservice.hpp
 * Defines the data types and Service for executions.
 * Execution service does not need an input connector, since it flows in data from a listener from algo execution service.
 * But it needs an inner connector (publish only) to publish executions.
 * @author Boyu Yang
 */
#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

#include <string>
#include "soa.hpp"
#include "algoexecutionservice.hpp"

/**
 * Forward declaration of ExecutionServiceConnector and ExecutionServiceListener.
 * As described, execution service needs a publish-only connector to publish executions 
 * Type T is the product type.
 */
template<typename T>
class ExecutionServiceConnector;
template<typename T>
class ExecutionServiceListener;


/**
 * Service for executing orders on an exchange.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class ExecutionService : public Service<string,ExecutionOrder <T> >
{
private:
  map<string, ExecutionOrder<T>> executionOrderMap; // store execution order data keyed by product identifier
  vector<ServiceListener<ExecutionOrder<T>>*> listeners; // list of listeners to this service
  ExecutionServiceConnector<T>* connector; // connector related to this server
  ExecutionServiceListener<T>* executionservicelistener; // listener related to this server

public:
  // ctor and dtor
  ExecutionService();
  ~ExecutionService()=default;

  // Get data on our service given a key
  ExecutionOrder<T>& GetData(string key);

  // The callback that a Connector should invoke for any new or updated data
  void OnMessage(ExecutionOrder<T>& data) override;

  // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
  void AddListener(ServiceListener<ExecutionOrder<T>> *listener) override;

  // Get all listeners on the Service.
  const vector< ServiceListener<ExecutionOrder<T>>* >& GetListeners() const override;

  // Get the special listener for execution service
  ExecutionServiceListener<T>* GetExecutionServiceListener();

  // Get the connector
  ExecutionServiceConnector<T>* GetConnector();

  // Execute an order on a market, call the publish-only connector to publish executions
  void ExecuteOrder(const ExecutionOrder<T>& order, Market market);

  // called by ExecutionServiceListener to subscribe data from Algo Execution Service to Execution Service
  void AddExecutionOrder(const AlgoExecution<T>& algoExecution);

};

template<typename T>
ExecutionService<T>::ExecutionService()
{
  executionservicelistener = new ExecutionServiceListener<T>(this);
}

template<typename T>
ExecutionOrder<T>& ExecutionService<T>::GetData(string key)
{
  return executionOrderMap[key];
}

/**
 * OnMessage() used to be called by input connector to subscribe data
 * no need to implement here.
 */
template<typename T>
void ExecutionService<T>::OnMessage(ExecutionOrder<T>& data)
{
}

template<typename T>
void ExecutionService<T>::AddListener(ServiceListener<ExecutionOrder<T>> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<ExecutionOrder<T>>* >& ExecutionService<T>::GetListeners() const
{
  return listeners;
}

template<typename T>
ExecutionServiceListener<T>* ExecutionService<T>::GetExecutionServiceListener()
{
  return executionservicelistener;
}

template<typename T>
ExecutionServiceConnector<T>* ExecutionService<T>::GetConnector()
{
  return connector;
}

/**
 * Execute an order on a market, call the publish-only connector to publish executions via connector
 */
template<typename T>
void ExecutionService<T>::ExecuteOrder(const ExecutionOrder<T>& order, Market market)
{
  connector->Publish(order, market);
}

/**
 * Called by ExecutionServiceListener to subscribe data from Algo Execution Service to Execution Service
 * Store the listened algo execution order data into execution order map
 * Key should be the order id (unique)
 */
template<typename T>
void ExecutionService<T>::AddExecutionOrder(const AlgoExecution<T>& algoExecution)
{
  ExecutionOrder<T> executionOrder = algoExecution.GetExecutionOrder();
  string orderId = executionOrder.GetOrderId();
  if (executionOrderMap.find(orderId) != executionOrderMap.end()) {executionOrderMap.erase(orderId);}
  executionOrderMap.insert(pair<string, ExecutionOrder<T>> (orderId, executionOrder));
  
  // notify the listener
  for (auto& l : listeners) {
      l -> ProcessAdd(executionOrder);
  }
}

/**
 * ExecutionServiceConnector: publish data to execution service.
 * Type T is the product type.
 */
template<typename T>
class ExecutionServiceConnector : public Connector<ExecutionOrder<T>>
{
private:
  ExecutionService<T>* service; // execution service related to this connector

public:
  // ctor
  ExecutionServiceConnector(ExecutionService<T>* _service);
  // dtor
  ~ExecutionServiceConnector()=default;

  // Publish data to the Connector
  void Publish(const ExecutionOrder<T>& order, Market& market);

  // No Subscribe() method for publish-only connector
};

template<typename T>
ExecutionServiceConnector<T>::ExecutionServiceConnector(ExecutionService<T>* _service)
: service(_service)
{
}

/**
 * Publish() method is used by publish-only connector to publish executions.
 */
template<typename T>
void ExecutionServiceConnector<T>::Publish(const ExecutionOrder<T>& order, Market& market)
{
  // print the execution order data
  auto product = order.GetProduct();
  string order_type;
  switch(order.GetOrderType()) {
      case FOK: order_type = "FOK"; break;
      case MARKET: order_type = "MARKET"; break;
      case LIMIT: order_type = "LIMIT"; break;
      case STOP: order_type = "STOP"; break;
      case IOC: order_type = "IOC"; break;
  }
  string tradeMarket;
  switch(market) {
      case BROKERTEC: tradeMarket = "BROKERTEC"; break;
      case ESPEED: tradeMarket = "ESPEED"; break;
      case CME: tradeMarket = "CME"; break;
  }
  cout << "ExecutionOrder: \n"
      <<"\tProduct: " << product.GetProductId() <<"\tOrderId: "<<order.GetOrderId()<< "\tTrade Market: " << tradeMarket << "\n"
      <<"\tPricingSide: "<<(order.GetSide()==BID? "Bid":"Offer")
      <<"\tOrderType: "<<order_type<<"\t\tIsChildOrder: "<<(order.IsChildOrder()?"True":"False") <<"\n"
      <<"\tPrice: "<<order.GetPrice()<<"\tVisibleQuantity: "<<order.GetVisibleQuantity()
      <<"\tHiddenQuantity: "<<order.GetHiddenQuantity()<<endl<<endl;
}

/**
* Execution Service Listener subscribes data from Algo Execution Service to Execution Service.
* Type T is the product type. Inner data type is AlgoExecution<T>.
*/
template<typename T>
class ExecutionServiceListener : public ServiceListener<AlgoExecution<T>>
{
private:
  ExecutionService<T>* executionService;

public:
  // ctor
  ExecutionServiceListener(ExecutionService<T>* _executionService);
  // dtor
  ~ExecutionServiceListener()=default;

  // Listener callback to process an add event to the Service
  void ProcessAdd(AlgoExecution<T> &data) override;

  // Listener callback to process a remove event to the Service
  void ProcessRemove(AlgoExecution<T> &data) override;

  // Listener callback to process an update event to the Service
  void ProcessUpdate(AlgoExecution<T> &data) override;

};  

template<typename T>
ExecutionServiceListener<T>::ExecutionServiceListener(ExecutionService<T>* _executionService)
{
  executionService = _executionService;
}

/**
 * ProcessAdd() method is used by listener to subscribe data from Algo Execution Service to Execution Service.
 * It calls ExecuteOrder() method (for publishing through connector) and the AddExecutionOrder() method (for saving data into execution order map)
 */
template<typename T>
void ExecutionServiceListener<T>::ProcessAdd(AlgoExecution<T> &data)
{
  // save algo execution info into execution service
  // directly pass in AlgoExecution<T> type and transit to ExecutionOrder<T> type inside the function
  executionService->AddExecutionOrder(data);

  // call the connector to publish executions
  ExecutionOrder<T> executionOrder = data.GetExecutionOrder();
  Market market = data.GetMarket();
  executionService->ExecuteOrder(executionOrder, market);

}

template<typename T>
void ExecutionServiceListener<T>::ProcessRemove(AlgoExecution<T> &data)
{
}

template<typename T>
void ExecutionServiceListener<T>::ProcessUpdate(AlgoExecution<T> &data)
{
}




#endif
