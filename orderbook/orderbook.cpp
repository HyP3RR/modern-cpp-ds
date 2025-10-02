#include "order.hpp"
#include <cstddef>
#include <map>


// high level view of orderbook (price, total quantity)
// this can be sent as snapshot stream

//struct LevelInfo {
//  Price price_;
// Quantity quantity_;
//};

using LevelInfos = std::map<Price, Quantity>; //very simple to handle

// for vector, need to do O(n) search for updates
// can say action happens on top of orderbook to optimise
// so we can keep it sorted with aggresive area being first
class OrderbookLevelInfos {
public:
  OrderbookLevelInfos(const LevelInfos &bids, const LevelInfos &asks)
      : bids_(bids), asks_(asks) {}

  const LevelInfos& GetBids() { return bids_; }
  const LevelInfos& GetAsks() {return asks_;}
private:
  LevelInfos bids_;
  LevelInfos asks_;  

};


class OrderBook {
private:
  struct OrderEntry {
    OrderPointer order_{nullptr};
    OrderPointers::iterator location_;
    };

  std::map<Price, OrderPointers, std::greater<Price>> bids_;
  std::map<Price, OrderPointers, std::less<Price>> asks_;

  std::map<OrderId, OrderEntry> orders_;

  bool CanMatch(Side side, Price price) const {
    //if it can match
    }
    Trades MatchOrder() {
      //loop and keep matching order. return each trade done
    }
    
public:
  Trades AddOrder(OrderPointer order) {
    // return trades if done, and add current order.
    // do sanity check to avoid repeting OrderId.
    // handle according to orderType
  }

  void CancelOrder(OrderId orderId) {}

  Trades ModifyOrder(OrderModify order) {
    //cancel + add logic, by converting OrderModify to pointer etc.
    }
    std::size_t Size() const { // size of orders_
    }
    OrderbookLevelInfos GetOrderInfos() const {
      // snapshot of orderbook returned, create and send.
      // either create on the fly here and send
      // or maintain the LevelInfo and make both synchronous
// can make it on the fly in a vector to keep it simple.       

      
      }
      
};
