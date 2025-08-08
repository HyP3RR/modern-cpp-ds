#include <iostream>
#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <variant>
#include <optional>
#include <tuple>
#include <format>
#include <cstdint>
#include <format>

/*
basic exchange side orderbook.
 */

enum class OrderType {
  GoodTillCancel,
  FillAndKill

};

enum class Side { Buy, Sell };

using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;

struct LevelInfo {

  Price price_;
  Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;

class OrderBookLevelInfos {
public:
  OrderBookLevelInfos(const LevelInfos &bids, const LevelInfos &asks)
      : bids_(bids), asks_(asks) {}
  const LevelInfos& GetBids() const {return bids_;}
  const LevelInfos &GetAsks() const { return asks_; }

private:
  LevelInfos bids_;
  LevelInfos asks_;
};

class Order {
public:
  Order(OrderType order, OrderId orderId, Side side, Price price,
        Quantity quantity)
      : orderType_(order), orderId_(orderId), side_(side), price_(price),
        initialQuantity_(quantity), remainingQuantity_(quantity) {}
  OrderId GetOrderId() const { return orderId_; }
  Side GetSide() const {return side_;}
  Price GetPrice() const {return price_;}
  OrderType GetOrderType() const { return orderType_; }
  Quantity GetInitialQuantity() const { return initialQuantity_; }
  Quantity GetRemaninigQuantity() const { return remainingQuantity_; }
  Quantity GetFilledQuantity() const {
    return GetInitialQuantity() - GetRemaninigQuantity();
  }
  void Fill(Quantity quantity) {
    if (quantity > GetRemaninigQuantity()) {
      throw std::logic_error(std::format(
					 "Order ({}) cannot be filled for more than remaining quantity."), GetOrderId())
    };
    remainingQuantity_ -= quantity;
  }

private:
  OrderType orderType_;
  OrderId orderId_;
  Side side_;
  Price price_;
  Quantity initialQuantity_;
  Quantity remainingQuantity_;  

};


using OrderPointer = std::shared_ptr<Order>; //might be shared b/w bid and asks
using OrderPointers =
    std::list<OrderPointer>; // replace with vector as more cache friendly.

class OrderModify {
public:
  OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
      : orderId_(orderId), price_(price), side_(side), quantity_(quantity) {}
      OrderId GetOrderId() const   {return orderId_;}
      Price GetPrice() const { return price_; }
      Side GetSide() const { return side_; }
      Quantity GetQuantity() const { return quantity_; }


      // converting a given order that already exists, transforing it into a new
      // order
      OrderPointer ToOrderPointer(OrderType type) const {
	return std::make_shared<Order>(type, GetOrderId(), GetSide(), GetPrice(), GetQuantity());
      }

    private:
      OrderId orderId_;
      Price price_;
      Side side_;
      Quantity quantity_;
};


// order matched
struct TradeInfo {
      OrderId orderId_;
      Price price_;
      Quantity quantity_;      
};

class Trade {
    public:
      Trade(const TradeInfo &bidTrade, const TradeInfo &askTrade)
          : bidTrade_(bidTrade), askTrade_(askTrade) {}
      const TradeInfo &GetBidTrade() const { return bidTrade_; }
      const TradeInfo &GetAskTrade() const { return askTrade_; }

    private:
      TradeInfo bidTrade_;
      TradeInfo askTrade_;
           
};

using Trades = std::vector<Trade>;

//orderbook,  with sorted orders of bid and ask with appropriate algo (proRata etc).

int main() {


}  
