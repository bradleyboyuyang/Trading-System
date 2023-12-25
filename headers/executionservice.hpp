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
 * Forward declaration of ExecutionOutputConnector and ExecutionServiceListener.
 * As described, execution service needs a publish-only connector to publish executions 
 * Type T is the product type.
 */
template<typename T>
class ExecutionOutputConnector;
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
  string host; // host name for inbound connector
  string port; // port number for inbound connector
  ExecutionOutputConnector<T>* connector; // connector related to this server
  ExecutionServiceListener<T>* executionservicelistener; // listener related to this server

public:
  // ctor and dtor
  ExecutionService(const string& _host, const string& _port);
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
  ExecutionOutputConnector<T>* GetConnector();

  // Execute an order on a market, call the publish-only connector to publish executions
  void ExecuteOrder(const ExecutionOrder<T>& order, Market market);

  // called by ExecutionServiceListener to subscribe data from Algo Execution Service to Execution Service
  void AddExecutionOrder(const AlgoExecution<T>& algoExecution);

};

template<typename T>
ExecutionService<T>::ExecutionService(const string& _host, const string& _port)
: host(_host), port(_port)
{
  connector = new ExecutionOutputConnector<T>(this, host, port); // connector related to this server
  executionservicelistener = new ExecutionServiceListener<T>(this); // listener related to this server
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
ExecutionOutputConnector<T>* ExecutionService<T>::GetConnector()
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
 * ExecutionOutputConnector: publish data to execution service.
 * Type T is the product type.
 */
template<typename T>
class ExecutionOutputConnector
{
private:
  ExecutionService<T>* service; // execution service related to this connector
  string host; // host name
  string port; // port number
  boost::asio::io_service io_service; // io service
  boost::asio::ip::tcp::socket socket; // socket

  void handle_read(const boost::system::error_code& ec, std::size_t length, boost::asio::ip::tcp::socket* socket, boost::asio::streambuf* request);
  void start_accept(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::io_service* io_service);

public:
  // ctor
  ExecutionOutputConnector(ExecutionService<T>* _service, const string& _host, const string& _port);
  // dtor: close the socket
  ~ExecutionOutputConnector();

  // Publish data to the Connector
  void Publish(const ExecutionOrder<T>& order, Market& market);

  // Here the subscribe is used to open a process and listen to the socket
  void Subscribe();
};

template<typename T>
ExecutionOutputConnector<T>::ExecutionOutputConnector(ExecutionService<T>* _service, const string& _host, const string& _port)
: service(_service), host(_host), port(_port), socket(io_service)
{
}

template<typename T>
ExecutionOutputConnector<T>::~ExecutionOutputConnector()
{
  socket.close();
}

template<typename T>
void ExecutionOutputConnector<T>::start_accept(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::io_service* io_service) {
  boost::asio::ip::tcp::socket* socket = new boost::asio::ip::tcp::socket(*io_service);
  acceptor->async_accept(*socket, [this, socket, acceptor, io_service](const boost::system::error_code& ec) {
    if (!ec) {
      boost::asio::streambuf* request = new boost::asio::streambuf;
      boost::asio::async_read_until(*socket, *request, "\r", std::bind(&ExecutionOutputConnector<T>::handle_read, this, std::placeholders::_1, std::placeholders::_2, socket, request));
    }
    start_accept(acceptor, io_service); // accept the next connection
  });
}

template<typename T>
void ExecutionOutputConnector<T>::handle_read(const boost::system::error_code& ec, std::size_t length, boost::asio::ip::tcp::socket* socket, boost::asio::streambuf* request) {
  if (!ec) {
    // get the entire data
    std::string data = std::string(boost::asio::buffers_begin(request->data()), boost::asio::buffers_end(request->data()));
    // find the last newline
    std::size_t last_newline = data.rfind('\r');
    if (last_newline != std::string::npos) {
      // consume only up to the last newline
      request->consume(last_newline + 1);
      // only process the data up to the last newline
      data = data.substr(0, last_newline);
    } else {
      // if there's no newline, don't process any data
      data.clear();
    }
    // server receives and prints data
    cout << data << endl;

    boost::asio::async_read_until(*socket, *request, "\n", std::bind(&ExecutionOutputConnector<T>::handle_read, this, std::placeholders::_1, std::placeholders::_2, socket, request));
  } else {
    delete request; // delete the streambuf when we're done with it
    delete socket; // delete the socket when we're done with it
  }
}

/**
 * Publish() method is used by publish-only connector to publish executions.
 */
template<typename T>
void ExecutionOutputConnector<T>::Publish(const ExecutionOrder<T>& order, Market& market)
{
  // connect to the socket
  boost::asio::ip::tcp::resolver resolver(io_service);
  boost::asio::ip::tcp::resolver::query query(host, port);
  boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  boost::asio::connect(socket, endpoint_iterator);

  // publish the execution order data to socket
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
  std::string dataLine = string("ExecutionOrder: \n")
    + "\tProduct: " + product.GetProductId() + "\tOrderId: " + order.GetOrderId() + "\tTrade Market: " + tradeMarket + "\n"
    + "\tPricingSide: " + (order.GetSide()==BID? "Bid":"Offer")
    + "\tOrderType: " + order_type + "\t\tIsChildOrder: " + (order.IsChildOrder()?"True":"False") + "\n"
    + "\tPrice: " + std::to_string(order.GetPrice()) + "\tVisibleQuantity: " + std::to_string(order.GetVisibleQuantity())
    + "\tHiddenQuantity: " + std::to_string(order.GetHiddenQuantity()) + "\n";

  // publish the data string to socket
  // asynchronous operation ensures server gets all data
  boost::asio::async_write(socket, boost::asio::buffer(dataLine + "\r"), [](boost::system::error_code /*ec*/, std::size_t /*length*/) {});
}

template<typename T>
void ExecutionOutputConnector<T>::Subscribe()
{
  log(LogLevel::NOTE, "Streaming output server listening on " + host + ":" + port);
  try {
  // connect to the socket
  boost::asio::ip::tcp::resolver resolver(io_service);
  boost::asio::ip::tcp::resolver::query query(host, port);
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
  boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint);
  start_accept(&acceptor, &io_service);
  io_service.run();
  }
  catch (std::exception& e){
    // throw error log
    log(LogLevel::ERROR, e.what());
    return;
  }
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
