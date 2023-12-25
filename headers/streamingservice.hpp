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
 * Forward declaration of StreamOutputConnector and StreamingServiceListener.
 * As described, streaming service needs a publish-only connector to publish streams (which notify the listener and print the execution data)
 * into a separate process which listens to the streams on the socket via its own Connector.
 * Type T is the product type.
 */
template<typename T>
class StreamOutputConnector;
template<typename T>
class StreamingServiceListener;

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
  string host; // host name for inbound connector
  string port; // port number for inbound connector
  StreamOutputConnector<T>* connector; // connector related to this server
  StreamingServiceListener<T>* streamingservicelistener; // listener related to this server

public:
  // ctor and dtor
  StreamingService(const string& _host, const string& _port);
  ~StreamingService()=default;

  // Get data on our service given a key
  PriceStream<T>& GetData(string key) override;

  // The callback that a Connector should invoke for any new or updated data
  void OnMessage(PriceStream<T>& data) override;

  // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
  void AddListener(ServiceListener<PriceStream<T>> *listener) override;

  // Get all listeners on the Service.
  const vector< ServiceListener<PriceStream<T>>* >& GetListeners() const override;

  // Get the special listener for streaming service
  StreamingServiceListener<T>* GetStreamingServiceListener();

  // Get the connector
  StreamOutputConnector<T>* GetConnector();

  // Publish two-way prices (called by the publish-only connector to publish streams)
  void PublishPrice(const PriceStream<T>& priceStream);

  // called by streaming service listener to subscribe data from algo streaming service
  void AddPriceStream(const AlgoStream<T>& algoStream);

};

template<typename T>
StreamingService<T>::StreamingService(const string& _host, const string& _port)
{
  host = _host;
  port = _port;
  connector = new StreamOutputConnector<T>(this, host, port); // connector related to this server
  streamingservicelistener = new StreamingServiceListener<T>(this); // listener related to this server
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
StreamingServiceListener<T>* StreamingService<T>::GetStreamingServiceListener()
{
  return streamingservicelistener;
}

template<typename T>
StreamOutputConnector<T>* StreamingService<T>::GetConnector()
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
 * StreamOutputConnector: publish data to socket.
 * Type T is the product type.
 */
template<typename T>
class StreamOutputConnector
{
private:
  StreamingService<T>* service;
  string host; // host name
  string port; // port number
  boost::asio::io_service io_service; // io service
  boost::asio::ip::tcp::socket socket; // socket

  void handle_read(const boost::system::error_code& ec, std::size_t length, boost::asio::ip::tcp::socket* socket, boost::asio::streambuf* request);
  void start_accept(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::io_service* io_service);

public:
  // ctor
  StreamOutputConnector(StreamingService<T>* _service, const string& _host, const string& _port);
  // dtor: close the socket
  ~StreamOutputConnector();

  // Publish data to the socket
  void Publish(const PriceStream<T>& data);

  // Here the subscribe is used to open a process and listen to the socket
  void Subscribe();

};

template<typename T>
StreamOutputConnector<T>::StreamOutputConnector(StreamingService<T>* _service, const string& _host, const string& _port)
: service(_service), host(_host), port(_port), socket(io_service)
{

}

template<typename T>
StreamOutputConnector<T>::~StreamOutputConnector()
{
  socket.close();
}

template<typename T>
void StreamOutputConnector<T>::start_accept(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::io_service* io_service) {
  boost::asio::ip::tcp::socket* socket = new boost::asio::ip::tcp::socket(*io_service);
  acceptor->async_accept(*socket, [this, socket, acceptor, io_service](const boost::system::error_code& ec) {
    if (!ec) {
      boost::asio::streambuf* request = new boost::asio::streambuf;
      boost::asio::async_read_until(*socket, *request, "\r", std::bind(&StreamOutputConnector<T>::handle_read, this, std::placeholders::_1, std::placeholders::_2, socket, request));
    }
    start_accept(acceptor, io_service); // accept the next connection
  });
}

template<typename T>
void StreamOutputConnector<T>::handle_read(const boost::system::error_code& ec, std::size_t length, boost::asio::ip::tcp::socket* socket, boost::asio::streambuf* request) {
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

    boost::asio::async_read_until(*socket, *request, "\n", std::bind(&StreamOutputConnector<T>::handle_read, this, std::placeholders::_1, std::placeholders::_2, socket, request));
  } else {
    delete request; // delete the streambuf when we're done with it
    delete socket; // delete the socket when we're done with it
  }
}

/**
 * Publish() method is used by the publish-only connector to publish streams.
 */
template<typename T>
void StreamOutputConnector<T>::Publish(const PriceStream<T>& data)
{
  // connect to the socket
  boost::asio::ip::tcp::resolver resolver(io_service);
  boost::asio::ip::tcp::resolver::query query(host, port);
  boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  boost::asio::connect(socket, endpoint_iterator);

  // print the price stream data
  T product = data.GetProduct();
  string productId = product.GetProductId();
  PriceStreamOrder bid = data.GetBidOrder();
  PriceStreamOrder offer = data.GetOfferOrder();

  string dataLine = string("Price Stream ") + "(Product " + productId + "): \n"
    + "\tBid\t" + "Price: " + std::to_string(bid.GetPrice()) + "\tVisibleQuantity: " + std::to_string(bid.GetVisibleQuantity())
    + "\tHiddenQuantity: " + std::to_string(bid.GetHiddenQuantity()) + "\n"
    + "\tAsk\t" + "Price: " + std::to_string(offer.GetPrice()) + "\tVisibleQuantity: " + std::to_string(offer.GetVisibleQuantity())
    + "\tHiddenQuantity: " + std::to_string(offer.GetHiddenQuantity()) + "\n";

  // publish the data string to socket
  // asynchronous operation ensures server gets all data
  boost::asio::async_write(socket, boost::asio::buffer(dataLine + "\r"), [](boost::system::error_code /*ec*/, std::size_t /*length*/) {});
}

template<typename T>
void StreamOutputConnector<T>::Subscribe()
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
