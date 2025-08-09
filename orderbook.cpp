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

  bool IsFilled() const {return GetRemaninigQuantity() == 0;}
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
//using price time priority instead of ProRata, complicated for now unnecessarily.
class Orderbook {
    private:
      struct OrderEntry {
        OrderPointer order_{nullptr};
	OrderPointers::iterator location_;
      };
      // in map, sort by price, time (price first obv)
      std::map<Price, OrderPointers, std::greater<Price>>
          bids_; // descending price -> buy

      
      std::map<Price, OrderPointers, std::less<Price>> asks_;

      
      std::unordered_map<OrderId, OrderEntry> orders_;
      // handling for fill and kill needed -> ie match orders jitna ho sake,
      // remaining quantity is killed

      
      bool CanMatch(Side side, Price price) const {
        if (side == Side::Buy) {
      if (asks_.empty())
        return false;

      // check if the bid/buy has price >= least selling price of sell order
      const auto &[bestAsk, ignore] = *asks_.begin();
      return price >= bestAsk;
        } else {
      if (bids_.empty())
        return false;

      const auto &[bestbid, ignore] = *bids_.begin();
      return price <= bestbid;      
      }
      }

      Trades MatchOrders() {
      Trades trades;
      trades.reserve(orders_.size());
      while (true) {
      if (bids_.empty() || asks_.empty())
        break;

      auto &[bidPrice, bids] = *(bids_.begin());
      auto &[askPrice, asks] = *asks_.begin();

      if (bidPrice < askPrice)
        break; // nothing to match

      while (bids.size() && asks.size()) { // while the bids and asks we got
	//arent empty
        auto &bid = bids.front(); // highest priority
        auto &ask = asks.front();

        Quantity quantity =
            std::min(bid->GetRemaninigQuantity(), ask->GetRemaninigQuantity());
        // quantity traded

        bid->Fill(quantity);
        ask->Fill(quantity); // trade

        if (bid->IsFilled()) {
          bids.pop_front();
          orders_.erase(bid->GetOrderId());
        }
        if (ask->IsFilled()) {
          asks.pop_front();
	  orders_.erase(ask->GetOrderId());
        }

        if (bids.empty())
          bids_.erase(bidPrice);

        if (asks.empty())
          asks_.erase(askPrice);

        trades.push_back(
			 Trade{TradeInfo{bid->GetOrderId(), bid->GetPrice(), quantity}, TradeInfo{ask->GetOrderId(), ask->GetPrice(), quantity}});
        
        }
      }
      if (!bids_.empty()) {
        auto &[ignore, bids] = *bids_.begin();
        auto &order = bids.front();
        if (order->GetOrderType() == OrderType::FillAndKill)
        CancelOrder(order->GetOrderId());
      }

      if (!asks_.empty()) {
        auto &[ignore, asks] = *(asks_.begin());
        auto &order = asks.front();
        if (order->GetOrderType() == OrderType::FillAndKill)
        CancelOrder(order->GetOrderId());
      }

      return trades;
      }





public: //define public api's
};


int main() {


}  
