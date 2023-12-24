/**
 * guiservice.hpp
 * Defines the data types and Service for GUI output.
 *
 * @author Boyu Yang
 */

#ifndef GUI_SERVICE_HPP
#define GUI_SERVICE_HPP

#include "soa.hpp"  
#include "utils.hpp"
#include "pricingservice.hpp"

// forward declaration of GUIConnector and GUIServiceListener
template<typename T>
class GUIConnector;
template<typename T>
class GUIServiceListener;


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
    GUIServiceListener<T>* guiservicelistener; // listener related to this server
    int throttle; // throttle of the service   
    std::chrono::system_clock::time_point startTime; // start time

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

    // Get the special listener for GUI service
    GUIServiceListener<T>* GetGUIServiceListener();

    // Get the connector
    GUIConnector<T>* GetConnector();

    // Get the throttle
    int GetThrottle() const;

    // Publish the throttled price through connector
    void PublishThrottledPrice(Price<T>& price);

};

template<typename T>
GUIService<T>::GUIService()
{
    connector = new GUIConnector<T>(this); // connector related to this server
    guiservicelistener = new GUIServiceListener<T>(this); // listener related to this server
    throttle = 300; // default throttle 
    startTime = std::chrono::system_clock::now(); // start time
}

template<typename T>
Price<T>& GUIService<T>::GetData(string key)
{
    return priceMap[key];
}

// no need to implement OnMessage
template<typename T>
void GUIService<T>::OnMessage(Price<T>& data)
{
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
GUIServiceListener<T>* GUIService<T>::GetGUIServiceListener()
{
    return guiservicelistener;
}

template<typename T>
GUIConnector<T>* GUIService<T>::GetConnector()
{
    return connector;
}

template<typename T>
int GUIService<T>::GetThrottle() const
{
    return throttle;
}

template<typename T>
void GUIService<T>::PublishThrottledPrice(Price<T>& price)
{
    // only publish price to GUI if the time interval is larger than throttle
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
    if (diff.count() > throttle) {
        // update the time
        startTime = now;
        // publish the price
        connector->Publish(price);
    }
}

/**
* GUI Connector publishing data from GUI Service.
* Type T is the product type.
*/
template<typename T>
class GUIConnector : public Connector<Price<T>>
{
private:
    GUIService<T>* service;

public:
    // ctor
    GUIConnector(GUIService<T>* _service);
    // Publish data to the Connector
    // If subscribe-only, then this does nothing
    void Publish(Price<T> &data) override;
};

template<typename T>
GUIConnector<T>::GUIConnector(GUIService<T>* _service)
{
    service = _service;
}

// publish to external source gui.txt
template<typename T>
void GUIConnector<T>::Publish(Price<T> &data)
{
    ofstream outFile;
    outFile.open("../res/gui.txt", ios::app);
    // need overloading operator<< for Price<T>
    outFile << getTime() << "," << data << endl;
    outFile.close();
}

/**
* GUI Service Listener subscribing data from Pricing Service to GUI Service.
* Type T is the product type.
*/
template<typename T>
class GUIServiceListener : public ServiceListener<Price<T>>
{
private:
    GUIService<T>* guiService;

public:
    // ctor
    GUIServiceListener(GUIService<T>* _guiService);
    // Listener callback to process an add event to the Service
    void ProcessAdd(Price<T>& price) override;
    // Listener callback to process a remove event to the Service
    void ProcessRemove(Price<T>& price) override;
    // Listener callback to process an update event to the Service
    void ProcessUpdate(Price<T>& price) override;
};

template<typename T>
GUIServiceListener<T>::GUIServiceListener(GUIService<T>* _guiService)
{
    guiService = _guiService;
}

template<typename T>
void GUIServiceListener<T>::ProcessAdd(Price<T>& price)
{
    guiService->PublishThrottledPrice(price);
}

template<typename T>
void GUIServiceListener<T>::ProcessRemove(Price<T>& price)
{
}

template<typename T>
void GUIServiceListener<T>::ProcessUpdate(Price<T>& price)
{
}

#endif