#include "headers/fileconnector.hpp"

int main(){
  const string marketDataPath = "../data/marketdata.txt";
  const string host = "localhost";
  const string port = "3001";
  FileConnector<Bond> marketFileConnector(marketDataPath, host, port);
  marketFileConnector.Subscribe();
}