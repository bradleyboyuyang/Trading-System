/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Boyu Yang
 */
#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <string>
#include "soa.hpp"

/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template<typename T>
class Price
{

public:

  // ctor for a price
  Price() = default;
  Price(const T &_product, double _mid, double _bidOfferSpread);

  // Get the product
  const T& GetProduct() const;

  // Get the mid price
  double GetMid() const;

  // Get the bid/offer spread around the mid
  double GetBidOfferSpread() const;

  // object printer
  template<typename U>
  friend ostream& operator<<(ostream& os, const Price<U>& price);

private:
  const T& product;
  double mid;
  double bidOfferSpread;

};

template<typename T>
Price<T>::Price(const T &_product, double _mid, double _bidOfferSpread) 
: product(_product), mid(_mid), bidOfferSpread(_bidOfferSpread)
{
}

template<typename T>
const T& Price<T>::GetProduct() const
{
  return product;
}

template<typename T>
double Price<T>::GetMid() const 
{
  return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const
{
  return bidOfferSpread;
}

template<typename T>
ostream& operator<<(ostream& os, const Price<T>& price)
{
  os << "Price Object (Product: " << price.product << ", Mid Price: " << price.mid << ", Bid/Offer Spread: " << price.bidOfferSpread << ")" << endl;
  return os;
}

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PricingService : public Service<string,Price <T> >
{
};

#endif
