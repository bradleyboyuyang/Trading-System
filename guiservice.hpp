/**
 * guiservice.hpp
 * Defines the data types and Service for GUI output.
 *
 * @author Boyu Yang
 */

#ifndef GUI_SERVICE_HPP
#define GUI_SERVICE_HPP

#include "soa.hpp"  
#include "pricingservice.hpp"

// forward declaration of GUIConnector
template<typename T>
class GUIConnector;

/**
* Service for outputing GUI with a certain throttle.
* Keyed on product identifier.
* Type T is the product type.
*/
template<typename T>
class GUIService : public Service<string,Price<T> >
{
private:
    map<string, Price<T>> priceMap; // store price data keyed by product identifier
    vector<ServiceListener<Price<T>>*> listeners; // list of listeners to this service
    GUIConnector<T>* connector; // connector related to this server

public:
    // ctor
    GUIService();
    // dtor
    ~GUIService()=default;

    // Get data on our service given a key
    Price<T>& GetData(string key) override;

    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(Price<T>& data) override;

    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<Price<T>> *listener) override;

    // Get all listeners on the Service.
    const vector< ServiceListener<Price<T>>* >& GetListeners() const override;

    // Get the connector
    GUIConnector<T>* GetConnector();

};

template<typename T>
GUIService<T>::GUIService()
{
    connector = new GUIConnector<T>(this); // connector related to this server
}

template<typename T>
Price<T>& GUIService<T>::GetData(string key)
{
    return priceMap[key];
}

template<typename T>
void GUIService<T>::OnMessage(Price<T>& data)
{
    priceMap[data.GetProduct().GetProductId()] = data;
    for(auto& listener : listeners)
    {
        listener->ProcessAdd(data);
    }
}

template<typename T>
void GUIService<T>::AddListener(ServiceListener<Price<T>> *listener)
{
    listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<Price<T>>* >& GUIService<T>::GetListeners() const
{
    return listeners;
}

template<typename T>
GUIConnector<T>* GUIService<T>::GetConnector()
{
    return connector;
}


#endif