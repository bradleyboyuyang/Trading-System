#include "headers/fileconnector.hpp"

int main(){
  const string tradePath = "../data/trades.txt";
  const string host = "localhost";
  const string port = "3002";
  FileConnector<Bond> marketFileConnector(tradePath, host, port);
  marketFileConnector.Subscribe();
}