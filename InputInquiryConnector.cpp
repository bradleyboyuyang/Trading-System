#include "headers/fileconnector.hpp"

int main(){
  const string inquiryPath = "../data/inquiries.txt";
  const string host = "localhost";
  const string port = "3003";
  FileConnector<Bond> inquiryFileConnector(inquiryPath, host, port);
  inquiryFileConnector.Subscribe();
}