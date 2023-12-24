/**
 * products.hpp 
 * models Bond, Interest Rate Swap, Future products, etc.
 *
 * @author Boyu Yang
 */

#ifndef PRODUCTS_HPP
#define PRODUCTS_HPP

#include <iostream>
#include <string>

#include "boost/date_time/gregorian/gregorian.hpp"

using namespace std;
using namespace boost::gregorian;

// Product Types
enum ProductType { IRSWAP, BOND, FUTURE };

/**
 * Definition of a base Product class
 */
class Product
{

public:
  // Product ctor
  Product(string _productId, ProductType _productType);

  // Return the product identifier
  string GetProductId() const;

  // Return the Product Type for this Product
  ProductType GetProductType() const;

private:
  string productId; // product id variable
  ProductType productType; // product type variable
};

// Types of bond identifiers: ISIN (used primarily in Europe) and CUSIP (for US)
enum BondIdType { CUSIP, ISIN };

/**
 * Modeling of a Bond Product
 */
class Bond : public Product
{
public:
  // Bond ctor
  Bond(string _productId, BondIdType _bondIdType, string _ticker, float _coupon, date _maturityDate);
  Bond();

  // Return the ticker of the bond
  string GetTicker() const;

  // Return the coupon of the bond
  float GetCoupon() const;

  // Return the maturity date of the bond
  date GetMaturityDate() const;

  // Return the bond identifier type
  BondIdType GetBondIdType() const;

  // Overload the << operator to print out the bond
  friend ostream& operator<<(ostream &output, const Bond &bond);

private:
  string productId; // product identifier variable
  BondIdType bondIdType; // bond id type variable
  string ticker; // ticker variable
  float coupon; // coupon variable
  date maturityDate; // maturity date variable
};

// Day Count convention values
enum DayCountConvention { THIRTY_THREE_SIXTY, ACT_THREE_SIXTY, ACT_THREE_SIXTY_FIVE};

// Payment Frequency values
enum PaymentFrequency { QUARTERLY, SEMI_ANNUAL, ANNUAL };

// Index on the floating leg of an IR Swap
enum FloatingIndex { LIBOR, EURIBOR };

// Tenor on the floating leg of an IR Swap
enum FloatingIndexTenor { TENOR_1M, TENOR_3M, TENOR_6M, TENOR_12M };

// Currency for the IR Swap
enum Currency { USD, EUR, GBP };

// IR Swap type
enum SwapType { SPOT, FORWARD, IMM, MAC, BASIS };

// IR Swap leg type (i.e. outright is one leg, curve is two legs, fly is three legs
enum SwapLegType { OUTRIGHT, CURVE, FLY };

/**
 * Modeling of an Interest Rate Swap Product
 */
class IRSwap : public Product
{
public:
  // IRSwap ctor
  IRSwap(string productId, DayCountConvention _fixedLegDayCountConvention, DayCountConvention _floatingLegDayCountConvention, PaymentFrequency _fixedLegPaymentFrequency, FloatingIndex _floatingIndex, FloatingIndexTenor _floatingIndexTenor, date _effectiveDate, date _terminationDate, Currency _currency, int termYears, SwapType _swapType, SwapLegType _swapLegType);
  IRSwap();

  // Return the day count convention on the fixed leg of the IR Swap
  DayCountConvention GetFixedLegDayCountConvention() const;

  // Return the day count convention on the floating leg of the IR Swap
  DayCountConvention GetFloatingLegDayCountConvention() const;

  // Return the payment frequency on the fixed leg of the IR Swap
  PaymentFrequency GetFixedLegPaymentFrequency() const;

  // Return the index on the floating leg of the IR Swap
  FloatingIndex GetFloatingIndex() const;

  // Return the tenor on the floating leg of the IR Swap
  FloatingIndexTenor GetFloatingIndexTenor() const;

  // Return the effective date of the IR Swap (i.e. when the IR Swap starts)
  date GetEffectiveDate() const;

  // Return the termination date of the IR Swap (i.e. when the IR Swap ends)
  date GetTerminationDate() const;

  // Return the currency of the IR Swap
  Currency GetCurrency() const;

  // Return the term in years of the IR Swap
  int GetTermYears() const;

  // Return the swap type of the IR Swap
  SwapType GetSwapType() const;

  // Return the swap leg type of the IR Swap
  SwapLegType GetSwapLegType() const;

  // Overload the << operator to print the IR Swap
  friend ostream& operator<<(ostream &output, const IRSwap &swap);

private:
  DayCountConvention fixedLegDayCountConvention; // fixed leg daycount convention variable
  DayCountConvention floatingLegDayCountConvention; // floating leg daycount convention variable
  PaymentFrequency fixedLegPaymentFrequency; // fixed leg payment freq
  FloatingIndex floatingIndex; // floating leg index
  FloatingIndexTenor floatingIndexTenor; // floating leg tenor
  date effectiveDate; // effective date
  date terminationDate; // termination date
  Currency currency; // currency
  int termYears; // term in years
  SwapType swapType; // swap type
  SwapLegType swapLegType; // swap leg type

  // return a string represenation for the day count convention
  string ToString(DayCountConvention dayCountConvention) const;

  // return a string represenation for the payment frequency
  string ToString(PaymentFrequency paymentFrequency) const;

  // return a string representation for the floating index
  string ToString(FloatingIndex floatingIndex) const;

  // return a string representation of the flaoting index tenor
  string ToString(FloatingIndexTenor floatingIndexTenor) const;

  // return a string representation of the currency
  string ToString(Currency currency) const;

  // return a string representation of the swap type
  string ToString(SwapType swapType) const;

  // return a string representation of the swap leg type
  string ToString(SwapLegType swapLegType) const;
};

Product::Product(string _productId, ProductType _productType)
{
  productId = _productId;
  productType = _productType;
}

string Product::GetProductId() const
{
  return productId;
}

ProductType Product::GetProductType() const
{
  return productType;
}

Bond::Bond(string _productId, BondIdType _bondIdType, string _ticker, float _coupon, date _maturityDate) : Product(_productId, BOND)
{
  bondIdType = _bondIdType;
  ticker = _ticker;
  coupon = _coupon;
  maturityDate = _maturityDate;
}

Bond::Bond() : Product(0, BOND)
{
}

string Bond::GetTicker() const
{
  return ticker;
}

float Bond::GetCoupon() const
{
  return coupon;
}

date Bond::GetMaturityDate() const
{
  return maturityDate;
}

BondIdType Bond::GetBondIdType() const
{
  return bondIdType;
}

ostream& operator<<(ostream &output, const Bond &bond)
{
  output << bond.ticker << " " << bond.coupon << " " << bond.GetMaturityDate();
  return output;
}

IRSwap::IRSwap(string _productId, DayCountConvention _fixedLegDayCountConvention, DayCountConvention _floatingLegDayCountConvention, PaymentFrequency _fixedLegPaymentFrequency, FloatingIndex _floatingIndex, FloatingIndexTenor _floatingIndexTenor, date _effectiveDate, date _terminationDate, Currency _currency, int _termYears, SwapType _swapType, SwapLegType _swapLegType)  : Product(_productId, IRSWAP)
{
  fixedLegDayCountConvention =_fixedLegDayCountConvention;
  floatingLegDayCountConvention =_floatingLegDayCountConvention;
  fixedLegPaymentFrequency =_fixedLegPaymentFrequency;
  floatingIndex =_floatingIndex;
  floatingIndexTenor =_floatingIndexTenor;
  effectiveDate =_effectiveDate;
  terminationDate =_terminationDate;
  currency =_currency;
  termYears = _termYears;
  swapType =_swapType;
  swapLegType = _swapLegType;
}

IRSwap::IRSwap() : Product(0, IRSWAP)
{
}

DayCountConvention IRSwap::GetFixedLegDayCountConvention() const
{
  return fixedLegDayCountConvention;
}

DayCountConvention IRSwap::GetFloatingLegDayCountConvention() const
{
  return floatingLegDayCountConvention;
}

PaymentFrequency IRSwap::GetFixedLegPaymentFrequency() const
{
  return fixedLegPaymentFrequency;
}

FloatingIndex IRSwap::GetFloatingIndex() const
{
  return floatingIndex;
}

FloatingIndexTenor IRSwap::GetFloatingIndexTenor() const
{
  return floatingIndexTenor;
}

date IRSwap::GetEffectiveDate() const
{
  return effectiveDate;
}

date IRSwap::GetTerminationDate() const
{
  return terminationDate;
}

Currency IRSwap::GetCurrency() const
{
  return currency;
}

int IRSwap::GetTermYears() const
{
  return termYears;
}

SwapType IRSwap::GetSwapType() const
{
  return swapType;
}

SwapLegType IRSwap::GetSwapLegType() const
{
  return swapLegType;
}


ostream& operator<<(ostream &output, const IRSwap &swap)
{
  output << "fixedDayCount:" << swap.ToString(swap.GetFixedLegDayCountConvention()) << " floatingDayCount:" << swap.ToString(swap.GetFloatingLegDayCountConvention()) << " paymentFreq:" << swap.ToString(swap.GetFixedLegPaymentFrequency()) << " " << swap.ToString(swap.GetFloatingIndexTenor()) << swap.ToString(swap.GetFloatingIndex()) << " effective:" << swap.GetEffectiveDate() << " termination:" << swap.GetTerminationDate() << " " << swap.ToString(swap.GetCurrency()) << " " << swap.GetTermYears() << "yrs " << swap.ToString(swap.GetSwapType()) << " " << swap.ToString(swap.GetSwapLegType());
  return output;
}

string IRSwap::ToString(DayCountConvention dayCountConvention) const
{
  switch (dayCountConvention) {
  case THIRTY_THREE_SIXTY: return "30/360";
  case ACT_THREE_SIXTY: return "Act/360";
  default: return "";
  }
}

string IRSwap::ToString(PaymentFrequency paymentFrequency) const
{
  switch (paymentFrequency) {
  case QUARTERLY: return "Quarterly";
  case SEMI_ANNUAL: return "Semi-Annual";
  case ANNUAL: return "Annual";
  default: return "";
  }
}

string IRSwap::ToString(FloatingIndex floatingIndex) const
{
  switch (floatingIndex) {
  case LIBOR: return "LIBOR";
  case EURIBOR: return "EURIBOR";
  default: return "";
  }
}

string IRSwap::ToString(FloatingIndexTenor floatingIndexTenor) const
{ 
  switch(floatingIndexTenor) {
  case TENOR_1M: return "1m";
  case TENOR_3M: return "3m";
  case TENOR_6M: return "6m";
  case TENOR_12M: return "12m";
  default: return "";
  }
}

string IRSwap::ToString(Currency currency) const
{ 
  switch(currency) {
  case USD: return "USD";
  case EUR: return "EUR";
  case GBP: return "GBP";
  default: return "";
  }
}

string IRSwap::ToString(SwapType swapType) const
{ 
  switch(swapType) {
  case SPOT: return "Standard";
  case FORWARD: return "Forward";
  case IMM: return "IMM";
  case MAC: return "MAC";
  case BASIS: return "Basis";
  default: return "";
  }
}

string IRSwap::ToString(SwapLegType swapLegType) const
{ 
  switch(swapLegType) {
  case OUTRIGHT: return "Outright";
  case CURVE: return "Curve";
  case FLY: return "Fly";
  default: return "";
  }
}

// future type
enum FutureType {EQUITY_INDEX, CURRENCY, INTEREST_RATE, COMMODITY, METAL, FX};
// interest rate future type
enum InterestRateFutureType {EURODOLLAR, FED_FUNDS, SWAP, BOND_FUTURE, SWAP_RATE, NONE_TYPE};
// exchange type
enum ExchangeType {CBOT, CMX, NYBOT, KCBT, MGE, MATIE, SFE, NYM, LIFFE, EUREX, ICE};
// delivery month
enum DeliveryMonth {JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC};

/**
 * Modeling of a Future Product
 */
class Future: public Product
{
public:
  // future ctor
  Future(string _productId, FutureType _futuretype, InterestRateFutureType _interestRateFutureType, ExchangeType _exchange, DeliveryMonth _deliveryMonth, float _tickSize, string _underlyingProductId, int _contractSize, date _futuresContractDate);
  Future(); // default ctor

  // return the future type
  FutureType GetFutureType() const;

  // return the interest rate future type
  InterestRateFutureType GetInterestRateFutureType() const;

  // return the exchange type
  ExchangeType GetExchangeType() const;

  // return the delivery month
  DeliveryMonth GetDeliveryMonth() const;

  // return the tick size
  float GetTickSize() const;

  // return the underlying product id
  string GetUnderlyingProductId() const;

  // return the contract size
  int GetContractSize() const;

  // return the futures contract date
  date GetFuturesContractDate() const;

  // overload the << operator to print the future
  friend ostream& operator<<(ostream &output, const Future &future);

private:
  FutureType futureType; // future type variable
  InterestRateFutureType interestRateFutureType; // interest rate future type variable
  ExchangeType exchange; // exchange type variable
  DeliveryMonth deliveryMonth; // delivery month variable
  float tickSize; // tick size variable
  string underlyingProductId; // underlying product id variable
  int contractSize; // contract size variable
  date futuresContractDate; // futures contract date variable

  // return a string representation of the future type
  string ToString(FutureType futureType) const;

  // return a string representation of the interest rate future type
  string ToString(InterestRateFutureType interestRateFutureType) const;

  // return a string representation of the exchange type
  string ToString(ExchangeType exchange) const;

  // return a string representation of the delivery month
  string ToString(DeliveryMonth deliveryMonth) const;
};

Future::Future(string _productId, FutureType _futuretype, InterestRateFutureType _interestRateFutureType, ExchangeType _exchange, DeliveryMonth _deliveryMonth, float _tickSize, string _underlyingProductId, int _contractSize, date _futuresContractDate): Product(_productId, FUTURE)
{
  futureType = _futuretype;
  interestRateFutureType = _interestRateFutureType;
  exchange = _exchange;
  deliveryMonth = _deliveryMonth;
  tickSize = _tickSize;
  underlyingProductId = _underlyingProductId;
  contractSize = _contractSize;
  futuresContractDate = _futuresContractDate;
} 

Future::Future() : Product(0, FUTURE)
{
}

FutureType Future::GetFutureType() const
{
  return futureType;
}

InterestRateFutureType Future::GetInterestRateFutureType() const
{
  return interestRateFutureType;
}

ExchangeType Future::GetExchangeType() const
{
  return exchange;
}

DeliveryMonth Future::GetDeliveryMonth() const
{
  return deliveryMonth;
}

float Future::GetTickSize() const
{
  return tickSize;
}

string Future::GetUnderlyingProductId() const
{
  return underlyingProductId;
}


int Future::GetContractSize() const
{
  return contractSize;
}

date Future::GetFuturesContractDate() const
{
  return futuresContractDate;
}

ostream& operator<<(ostream &output, const Future &future)
{
  output << "FutureType:" << future.ToString(future.GetFutureType()) << " InterestRateFutureType:" << future.ToString(future.GetInterestRateFutureType()) << " Exchange:" << future.ToString(future.GetExchangeType()) << " DeliveryMonth:" << future.ToString(future.GetDeliveryMonth()) << " TickSize:" << future.GetTickSize() << " UnderlyingProductId:" << future.GetUnderlyingProductId() << " ContractSize:" << future.GetContractSize() << " FuturesContractDate:" << future.GetFuturesContractDate();
  return output;
}

string Future::ToString(FutureType futureType) const
{
  switch(futureType) {
  case EQUITY_INDEX: return "Equity Index";
  case CURRENCY: return "Currency";
  case INTEREST_RATE: return "Interest Rate";
  case COMMODITY: return "Commodity";
  case METAL: return "Metal";
  case FX: return "FX";
  default: return "";
  }
}

string Future::ToString(InterestRateFutureType interestRateFutureType) const
{
  switch(interestRateFutureType) {
  case EURODOLLAR: return "Eurodollar";
  case FED_FUNDS: return "Fed Funds";
  case SWAP: return "Swap";
  case BOND_FUTURE: return "Bond Future";
  case SWAP_RATE: return "Swap Rate";
  case NONE_TYPE: return "None";
  default: return "";
  }
}

string Future::ToString(ExchangeType exchange) const
{
  switch(exchange) {
  case CBOT: return "CBOT";
  case CMX: return "CMX";
  case NYBOT: return "NYBOT";
  case KCBT: return "KCBT";
  case MGE: return "MGE";
  case MATIE: return "MATIE";
  case SFE: return "SFE";
  case NYM: return "NYM";
  case LIFFE: return "LIFFE";
  case EUREX: return "EUREX";
  case ICE: return "ICE";
  default: return "";
  }
}

string Future::ToString(DeliveryMonth deliveryMonth) const
{
  switch(deliveryMonth) {
  case JAN: return "Jan";
  case FEB: return "Feb";
  case MAR: return "Mar";
  case APR: return "Apr";
  case MAY: return "May";
  case JUN: return "Jun";
  case JUL: return "Jul";
  case AUG: return "Aug";
  case SEP: return "Sep";
  case OCT: return "Oct";
  case NOV: return "Nov";
  case DEC: return "Dec";
  default: return "";
  }
}

// bond future type
enum BondFutureType {TWO_YR, THREE_YR, FIVE_YR, SEVEN_YR, TEN_YR, TWENTY_YR};

/**
 * Modeling of a Bond Future Product
 */
class BondFuture: public Future
{
public:
  // bond future ctor
  BondFuture(string _productId, FutureType _futuretype, InterestRateFutureType _interestRateFutureType, ExchangeType _exchange, DeliveryMonth _deliveryMonth, float _tickSize, string _underlyingProductId, int _contractSize, date _futuresContractDate, BondFutureType _bondFutureType);
  BondFuture(); // default ctor

  // return the bond future type
  BondFutureType GetBondFutureType() const;

  // overload the << operator to print the bond future
  friend ostream& operator<<(ostream &output, const BondFuture &bondFuture);

private:
  BondFutureType bondFutureType; // bond future type variable

  // return a string representation of the bond future type
  string ToString(BondFutureType bondFutureType) const;
};

BondFuture::BondFuture(string _productId, FutureType _futuretype, InterestRateFutureType _interestRateFutureType, ExchangeType _exchange, DeliveryMonth _deliveryMonth, float _tickSize, string _underlyingProductId, int _contractSize, date _futuresContractDate, BondFutureType _bondFutureType) 
    : Future(_productId, _futuretype, _interestRateFutureType, _exchange, _deliveryMonth, _tickSize, _underlyingProductId, _contractSize, _futuresContractDate)
{
  bondFutureType = _bondFutureType;
}

BondFutureType BondFuture::GetBondFutureType() const
{
  return bondFutureType;
}

ostream& operator<<(ostream &output, const BondFuture &bondFuture)
{
  output << static_cast<const Future&>(bondFuture); // Inherit the Future class operator

  output << " BondFutureType: " << bondFuture.ToString(bondFuture.GetBondFutureType()); // Print additional information (bond type)

  return output;
}


string BondFuture::ToString(BondFutureType bondFutureType) const
{
  switch(bondFutureType) {
  case TWO_YR: return "2Yr";
  case THREE_YR: return "3Yr";
  case FIVE_YR: return "5Yr";
  case SEVEN_YR: return "7Yr";
  case TEN_YR: return "10Yr";
  case TWENTY_YR: return "20Yr";
  default: return "";
  }
}

/**
*Modeling a Eurodollar Future Product
*/
class EurodollarFuture: public Future
{
public:
  // eurodollar future ctor
  EurodollarFuture(string _productId, FutureType _futuretype, InterestRateFutureType _interestRateFutureType, ExchangeType _exchange, DeliveryMonth _deliveryMonth, float _tickSize, string _underlyingProductId, int _contractSize, date _futuresContractDate, float _liborRate);
  EurodollarFuture(); // default ctor

  // return the libor rate
  float GetLiborRate() const;

  // overload the << operator to print the eurodollar future
  friend ostream& operator<<(ostream &output, const EurodollarFuture &eurodollarFuture);

private:
  float liborRate; // libor rate variable
};

EurodollarFuture::EurodollarFuture(string _productId, FutureType _futuretype, InterestRateFutureType _interestRateFutureType, ExchangeType _exchange, DeliveryMonth _deliveryMonth, float _tickSize, string _underlyingProductId, int _contractSize, date _futuresContractDate, float _liborRate) 
    : Future(_productId, _futuretype, _interestRateFutureType, _exchange, _deliveryMonth, _tickSize, _underlyingProductId, _contractSize, _futuresContractDate)
{
  liborRate = _liborRate;
}

float EurodollarFuture::GetLiborRate() const
{
  return liborRate;
}

ostream& operator<<(ostream &output, const EurodollarFuture &eurodollarFuture)
{
  output << static_cast<const Future&>(eurodollarFuture); // Inherit the Future class operator
  // Print additional information (libor rate)
  output << " LiborRate:" << eurodollarFuture.GetLiborRate();
  return output;
}


#endif
