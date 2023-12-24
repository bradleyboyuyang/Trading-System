#include "headers/fileconnector.hpp"

int main(){
  const string pricePath = "../data/prices.txt";
  const string host = "localhost";
  const string port = "3000";
  FileConnector<Bond> priceFileConnector(pricePath, host, port);
  priceFileConnector.Subscribe();
}