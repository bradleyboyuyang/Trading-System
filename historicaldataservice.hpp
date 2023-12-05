/**
 * historicaldataservice.hpp
 * historicaldataservice.hpp
 *
 * @author Boyu Yang
 * Defines the data types and Service for historical data.
 */
#ifndef HISTORICAL_DATA_SERVICE_HPP
#define HISTORICAL_DATA_SERVICE_HPP


#include "soa.hpp"

enum ServiceType {POSITION, RISK, EXECUTION, STREAMING, INQUIRY};


// pre declaration
template<typename T>
class HistoricalDataConnector;



/**
 * Service for processing and persisting historical data to a persistent store.
 * Keyed on some persistent key.
 * Type T is the data type to persist.
 */
template<typename T>
class HistoricalDataService : Service<string,T>
{

public:
  // ctor and dtor
  HistoricalDataService();
  ~HistoricalDataService()=default;
  HistoricalDataService(ServiceType _type);  


  // Persist data to a store
  void PersistData(string persistKey, const T& data) = 0;

};

#endif
