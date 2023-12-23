# Trading-System
 A scalable, extensible, and maintainable low-latency trading system framework designed under service-oriented architecture (SOA). 

## Service Architecture

We give an example of a trading system for seven US Treasury securities, associated with real-time price and orderbook flows, data streaming, user inquiries, algorithm order execution, risk monitor, logging modules, etc.



### Connector

A connector is a component that flows data into trading system from connectivity source (e.g. a socket, database, etc). Connectors can be subscribe-only or publish-only, or both.
A file connector (also an outbound connector) can both `Subscribe` and `Publish` data. It subscribes data from outside data source and publishes data to a socket with specified host and port. An inbound connector can then subscribes data from the socket and flows data into the trading system through calling `Service.OnMessage()`. It can also publish data to outside source using `Service.Publish()`.


### Data Flow

External data flows into the trading system through connectors and service listeners:

1. price data -> pricing service -> algo streaming service -> streaming service -> historical data service

2. price data -> pricing service -> GUI service -> GUI output

3. orderbook data -> market data service -> algo execution service -> execution service -> historical data service

4. execution service -> trade booking service -> position service -> risk service -> historical data service

5. trade data -> trade booking service -> position service -> risk service -> historical data service

6. inquiry data -> inquiry service -> historical data service



## Deployment

```bash
# install boost and cmake tools
sudo apt-get update
sudo apt-get install libboost-all-dev
sudo apt install cmake

# compile and run executables
mkdir build
cd build
cmake ..
make
./tradingsystem
```



## Scripts
- Core components
  - `main`: main file that prepares trading system, set up interactions among services, and **start different service components simultaneously using multi-threading**
  - `products`: define the class for the trading products, can be treasury bonds, interest rate swaps, future, commodity, or any user-defined product object
  - `historicaldataservice`: a last-step service that listens to position service, risk service, execution service, streaming service, and inquiry service; persist objects it receives and saves the data into database (usually data centers, KDB database, etc)
  - `utils`: time displayer, data generator, and risk calculator

- Price data components

  - `pricingservice`: read in price data from the socket to the system through an inbound connector

  - `algostreamingservice`: listen to pricing service, flow in data of `Price<T>` and generate data of `AlgoStream<T>`  

  - `streamingservice`: listen to algo streaming service, flow in data of `AlgoStream<T>` and record bid/ask prices into `priceStream<T>`, publish streams via socket in a separate process
  - `guiservice`: a GUI component that listens to streaming prices that should be throttled with a 300 millisecond throttle., register a service listener on the pricing service and output the updates with a timestamp with millisecond precision to a file `gui.txt`.

- Orderbook data components
  - `marketdataservice`: read in orderbook data from the socket to the system through an inbound connector
  - `algoexecutionservice`: listen to market data service, flow in data of `Orderbook<T>` and turn into execution data `AlgoExecution<T>`
  - `executionservice`: listen to algo execution service, flow in data of `AlgoExecution<T>` and record order information into `ExecutionOrder<T>`, publish order executions via socket in a separate process
- Trade data components
  - `tradebookingservice`: read in trade data, listen to execution service at the same time, flow in `ExecutionOrder<T>` and turn in trade data of type `Trade<T>`
  - `positionservice`: listen to trade booking service, flow in `Trade<T>` data and turn into `Position<T>`
  - `riskservice`: listen to position service, flow in `Position<T>` data and calculate corresponding position risks, such as `PV01<T>`. 
- Inquiry data components
  - `inquiryservice`: read in user inquiry data, interact with connectors and deal with inquiries

- Data and results

  - `data`: data source for price data, orderbook updates, user inquiries, and trade data, can be replaced by other connectivity source (a database, socket, etc)

  - `res`: results published by the system, including processed queries, executed orders, positions, risk monitor, data streaming, and GUI output.

## Note
The trading system is designed to be scalable, extensible, and maintainable. Multi-threading and asynchronous programming ensures low-latency, high-throughput, and high-performance. The system is also designed to be modularized, with each service component being independent and loosely coupled with others. New trading products can be added into `products.hpp`, new services, listeners, connectors can all be easily added and integrated into the whole system.