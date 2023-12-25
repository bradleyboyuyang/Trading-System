/**
 * inquiryservice.hpp
 * Defines the data types and Service for customer inquiries.
 *
 * @author Boyu Yang
 */
#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "soa.hpp"
#include "utils.hpp"
#include "tradebookingservice.hpp"

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template<typename T>
class Inquiry
{

public:

  // ctor for an inquiry
  Inquiry() = default;
  Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state);

  // Get the inquiry ID
  const string& GetInquiryId() const;

  // Get the product
  const T& GetProduct() const;

  // Get the side on the inquiry
  Side GetSide() const;

  // Get the quantity that the client is inquiring for
  long GetQuantity() const;

  // Get the price that we have responded back with
  double GetPrice() const;

  // Set the price
  void SetPrice(double _price);

  // Get the current state on the inquiry
  InquiryState GetState() const;

  // Set the current state on the inquiry
  void SetState(InquiryState state);

  // object printer
  template<typename U>
  friend ostream& operator<<(ostream& os, const Inquiry<U>& inquiry);

private:
  string inquiryId;
  T product;
  Side side;
  long quantity;
  double price;
  InquiryState state;

};


template<typename T>
Inquiry<T>::Inquiry(string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state) :
  product(_product)
{
  inquiryId = _inquiryId;
  side = _side;
  quantity = _quantity;
  price = _price;
  state = _state;
}

template<typename T>
const string& Inquiry<T>::GetInquiryId() const
{
  return inquiryId;
}

template<typename T>
const T& Inquiry<T>::GetProduct() const
{
  return product;
}

template<typename T>
Side Inquiry<T>::GetSide() const
{
  return side;
}

template<typename T>
long Inquiry<T>::GetQuantity() const
{
  return quantity;
}

template<typename T>
double Inquiry<T>::GetPrice() const
{
  return price;
}

template<typename T>
void Inquiry<T>::SetPrice(double _price)
{
  price = _price;
}

template<typename T>
InquiryState Inquiry<T>::GetState() const
{
  return state;
}

template<typename T>
void Inquiry<T>::SetState(InquiryState _state)
{
  state = _state;
}

template<typename T>
ostream& operator<<(ostream& os, const Inquiry<T>& inquiry)
{
  string _inquiryId = inquiry.GetInquiryId();
  T product = inquiry.GetProduct();
  string _product = product.GetProductId();
  Side side = inquiry.GetSide();
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
  long _quantity = inquiry.GetQuantity();
  double _price = inquiry.GetPrice();
  string _state;
  switch (inquiry.GetState())
  {
  case RECEIVED:
    _state = "RECEIVED";
    break;
  case QUOTED:
    _state = "QUOTED";
    break;
  case DONE:
    _state = "DONE";
    break;
  case REJECTED:
    _state = "REJECTED";
    break;
  case CUSTOMER_REJECTED:
    _state = "CUSTOMER_REJECTED";
    break;
  }

  vector<string> _strings;
  _strings.push_back(_inquiryId);
  _strings.push_back(_product);
  _strings.push_back(_side);
  _strings.push_back(to_string(_quantity));
  _strings.push_back(convertPrice(_price));
  _strings.push_back(_state);
  string _str = join(_strings, ",");
  os << _str;
  return os;
}

// forward declaration of InquiryDataConnector
template<typename T>
class InquiryDataConnector;

/**
 * Service for customer inquiry objects.
 * Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
 * Type T is the product type.
 */
template<typename T>
class InquiryService : public Service<string,Inquiry <T> >
{
private:
  map<string, Inquiry<T>> inquiryMap;
  vector<ServiceListener<Inquiry<T>>*> listeners;
  InquiryDataConnector<T>* connector;
  string host; // host name for inbound connector
  string port; // port number for inbound connector

public:
  // ctor and dtor
  InquiryService(const string& _host, const string& _port);
  ~InquiryService()=default;

  // Get data on our service given a key
  Inquiry<T>& GetData(string key);

  // The callback that a Connector should invoke for any new or updated data
  void OnMessage(Inquiry<T> &data);

  // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service.
  void AddListener(ServiceListener<Inquiry<T>> *listener);

  // Get all listeners on the Service.
  const vector< ServiceListener<Inquiry<T>>* >& GetListeners() const;

  // Get the connector
  InquiryDataConnector<T>* GetConnector();

  // Send a quote back to the client
  void SendQuote(const string &inquiryId, double price);

  // Reject an inquiry from the client
  void RejectInquiry(const string &inquiryId);

};

template<typename T>
InquiryService<T>::InquiryService(const string& _host, const string& _port)
: host(_host), port(_port)
{
  connector = new InquiryDataConnector<T>(this, host, port);
}

template<typename T>
Inquiry<T>& InquiryService<T>::GetData(string key)
{
  return inquiryMap[key];
}

template<typename T>
void InquiryService<T>::OnMessage(Inquiry<T> &data)
{
  InquiryState state = data.GetState();
  string inquiryId = data.GetInquiryId();
  switch (state){
    case RECEIVED:
      // if inquiry is received, send back a quote to the connector via publish()
      connector->Publish(data);
      break;
    case QUOTED:
      // finish the inquiry with DONE status and send an update of the object
      data.SetState(DONE);
      // store the inquiry
      if (inquiryMap.find(inquiryId) != inquiryMap.end()) {inquiryMap.erase(inquiryId);}
      inquiryMap.insert(pair<string, Inquiry<T> > (inquiryId, data));
      // notify listeners
      for (auto& listener : listeners)
      {
        listener->ProcessAdd(data);
      }
      break;
    default:
      break;
  }


  // if inquiry is done, remove it from the map
  if (data.GetState() == DONE)
  {
    inquiryMap.erase(data.GetInquiryId());
  }
  // otherwise, update the inquiry
  else
  {
    inquiryMap[data.GetInquiryId()] = data;
  }

  // notify listeners
  for (auto& listener : listeners)
  {
    listener->ProcessAdd(data);
  }
}

template<typename T>
void InquiryService<T>::AddListener(ServiceListener<Inquiry<T>> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<Inquiry<T>>* >& InquiryService<T>::GetListeners() const
{
  return listeners;
}

template<typename T>
InquiryDataConnector<T>* InquiryService<T>::GetConnector()
{
  return connector;
}

template<typename T>
void InquiryService<T>::SendQuote(const string &inquiryId, double price)
{
  // get the inquiry
  Inquiry<T>& inquiry = inquiryMap[inquiryId];
  // update the inquiry
  inquiry.SetPrice(price);
  // notify listeners
  for (auto& listener : listeners)
  {
    listener->ProcessAdd(inquiry);
  }
}

template<typename T>
void InquiryService<T>::RejectInquiry(const string &inquiryId)
{
  // get the inquiry
  Inquiry<T>& inquiry = inquiryMap[inquiryId];
  // update the inquiry
  inquiry.SetState(REJECTED);
}

/**
* Inquiry data connector subscribing data from socket to Inquiry Service 
* Type T is the product type.
*/
template <typename T>
class InquiryDataConnector : public Connector<Inquiry<T>>
{
private:
  InquiryService<T>* service;
  string host; // host name
  string port; // port number
  boost::asio::io_service io_service; // io service
  boost::asio::ip::tcp::socket socket; // socket

  void handle_read(const boost::system::error_code& ec, std::size_t length, boost::asio::ip::tcp::socket* socket, boost::asio::streambuf* request);
  void start_accept(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::io_service* io_service);

public:
  // ctor
  InquiryDataConnector(InquiryService<T>* _service, const string& _host, const string& _port);
  // dtor: close the socket
  ~InquiryDataConnector();

  // Publish data to the Connector
  // If subscribe-only, then this does nothing
  void Publish(Inquiry<T> &data) override;

  // Subscribe data from socket
  void Subscribe();

  // Subcribe updated inquiry record from the connector
  void SubscribeUpdate(Inquiry<T>& data);
};

template<typename T>
InquiryDataConnector<T>::InquiryDataConnector(InquiryService<T>* _service, const string& _host, const string& _port)
: service(_service), host(_host), port(_port), socket(io_service)
{
}

template<typename T>
InquiryDataConnector<T>::~InquiryDataConnector()
{
    socket.close();
}

template<typename T>
void InquiryDataConnector<T>::start_accept(boost::asio::ip::tcp::acceptor* acceptor, boost::asio::io_service* io_service) {
  boost::asio::ip::tcp::socket* socket = new boost::asio::ip::tcp::socket(*io_service);
  acceptor->async_accept(*socket, [this, socket, acceptor, io_service](const boost::system::error_code& ec) {
    if (!ec) {
      boost::asio::streambuf* request = new boost::asio::streambuf;
      boost::asio::async_read_until(*socket, *request, "\n", std::bind(&InquiryDataConnector<T>::handle_read, this, std::placeholders::_1, std::placeholders::_2, socket, request));
    }
    start_accept(acceptor, io_service); // accept the next connection
  });
}

template<typename T>
void InquiryDataConnector<T>::handle_read(const boost::system::error_code& ec, std::size_t length, boost::asio::ip::tcp::socket* socket, boost::asio::streambuf* request) {
  if (!ec) {
    std::string data = std::string(boost::asio::buffers_begin(request->data()), boost::asio::buffers_end(request->data()));
    // find the last newline
    std::size_t last_newline = data.rfind('\n');
    if (last_newline != std::string::npos) {
      // consume only up to the last newline
      request->consume(last_newline + 1);
      // only process the data up to the last newline
      data = data.substr(0, last_newline);
    } else {
      // if there's no newline, don't process any data
      data.clear();
    }

    // split the data into lines
    std::stringstream ss(data);
    std::string line;
    while (std::getline(ss, line)) {
      // parse the line
      vector<string> tokens;
      stringstream lineStream(line);
      string token;
      while(getline(lineStream, token, ','))
        tokens.push_back(token);

      // create inquiry
      string inquiryId = tokens[0];
      string productId = tokens[1];
      T product = getProductObject<T>(productId);
      Side side = tokens[2] == "BUY" ? BUY : SELL;
      long quantity = stol(tokens[3]);
      double price = convertPrice(tokens[4]);
      InquiryState state = tokens[5] == "RECEIVED" ? RECEIVED : tokens[5] == "QUOTED" ? QUOTED : tokens[5] == "DONE" ? DONE : tokens[5] == "REJECTED" ? REJECTED : CUSTOMER_REJECTED;
      Inquiry<T> inquiry(inquiryId, product, side, quantity, price, state);
      service->OnMessage(inquiry);
    }

    boost::asio::async_read_until(*socket, *request, "\n", std::bind(&InquiryDataConnector<T>::handle_read, this, std::placeholders::_1, std::placeholders::_2, socket, request));
  } else {
    delete request; // delete the streambuf when we're done with it
    delete socket; // delete the socket when we're done with it
  }
}

// Transite the inquiry from RECEIVED to QUOTED and send back to the service
template <typename T>
void InquiryDataConnector<T>::Publish(Inquiry<T> &data)
{
  if (data.GetState() == RECEIVED){
    data.SetState(QUOTED);
    this->SubscribeUpdate(data);
  }
}

template<typename T>
void InquiryDataConnector<T>::Subscribe()
{
  log(LogLevel::NOTE, "Inquiry data server listening on " + host + ":" + port);
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

template<typename T>
void InquiryDataConnector<T>::SubscribeUpdate(Inquiry<T>& data)
{
  // send updated inquiry back to the service
  service->OnMessage(data);
}

#endif
