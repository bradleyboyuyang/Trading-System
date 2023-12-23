# Trading-System
 A scalable, extensible, and maintainable low-latency trading system framework designed under service-oriented architecture (SOA). 

## Service Architecture

We give an example of a trading system for seven US Treasury securities, associated with real-time price and orderbook flows, data streaming, user inquiries, algorithm order execution, risk monitor, logging modules, and others.



Data flows into the trading system through the following branches:

- price data -> pricing service -> algo streaming service -> streaming service -> historical data service

- price data -> pricing service -> GUI service -> GUI output

- orderbook data -> market data service -> algo execution service -> execution service -> historical data service

- execution service -> trade booking service -> position service -> risk service -> historical data service

- trade data -> trade booking service -> position service -> risk service -> historical data service

- inquiry data -> inquiry service -> historical data service

## Scripts

- Data and results

  - `data`: data source for price data, orderbook updates, user inquiries, and trade data, can be replaced by other connectivity source (a database, socket, etc)

  - `res`: results published by the system, including processed queries, executed orders, positions, risk monitor, data streaming, and GUI output.

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
- `historicaldataservice`: a last-step service that listens to position service, risk service, execution service, streaming service, and inquiry service; persist objects it receives and saves the data into database (usually data centers, KDB database, etc)
- `products`: define the class for the trading products, can be treasury bonds, interest rate swaps, future, commodity, or any user-defined product object
- `utils`: utility functions include time displayer, data generator, and risk calculator
- `main`: main file that prepares trading system, set up interactions among services, and **start different service components simultaneously using multi-threading**



## Deployment

```bash
# install boost and cmake tools
sudo apt-get update
sudo apt-get install libboost-all-dev
sudo apt install cmake

# build and run executables
mkdir build
cd build
cmake ..
make
./tradingsystem
```

