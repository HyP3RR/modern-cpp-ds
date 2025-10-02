#include <cstdint>
#include <memory>
#include <list>
#include <stdexcept>
#include <vector>


//support for add, modify and cancel too

enum class OrderType {
  GoodTillCancel,
  FillAndKill,
  MarketOrder
  };

enum class Side {
  Buy,
  Sell
};

using Quantity = std::uint32_t;
using Price = std::int32_t;
using OrderId = std::uint64_t;








class Order {
public:
  Order(OrderId orderId, Price price, Quantity quantity, Side side, OrderType ordertype) : orderId_(orderId), price_(price) , initialQuantity_(quantity), remainingQuantity_(initialQuantity_), side_(side), orderType_(ordertype){}

      OrderId GetOrderId() const   {return orderId_;}
      Price GetPrice() const { return price_; }
      Side GetSide() const { return side_; }
      Quantity GetQuantity() const { return remainingQuantity_; }
      
  void Fill(const Quantity quantity) {
    if (quantity > remainingQuantity_) {
      throw std::logic_error("fill quantity > remaining quantity");
    }
    remainingQuantity_ -= quantity;
    }
  
private:
  OrderId orderId_;
  Price price_;
  Quantity initialQuantity_;
  Quantity remainingQuantity_;
  Side side_;
  OrderType orderType_;  
  //add agent id for self match prevention
};


using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;
// orderpointers for a fixed given price
// can use vector also


// for add -> direct new order obj suffices
// for cancel -> orderId sufficent
// for modif -> new abstraction to implement cancel + add, via this type to indicate modif.

//for trade confirm -> trade object with buy and sell side info

class OrderModify {
public:
  OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
      : orderId_(orderId), price_(price), side_(side), quantity_(quantity) {}
      OrderId GetOrderId() const   {return orderId_;}
      Price GetPrice() const { return price_; }
      Side GetSide() const { return side_; }
      Quantity GetQuantity() const { return quantity_; }


      
  OrderPointer ToOrderPointer(OrderType type) const {
    //works more like a cancel-replace, specify type and return the shared ptr to modified
    //need to delete old one tho
    return std::make_shared<Order>( GetOrderId(), GetPrice(), GetQuantity(),  GetSide(),type);
      }

    private:
      OrderId orderId_;
      Price price_;
      Side side_;
      Quantity quantity_;
};


struct TradeInfo {
  OrderId orderId_;
  Price price_;
  Quantity quantity_;
};

class Trade {
public:
  Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade) : bidTrade_(bidTrade) , askTrade_(askTrade) {}

  const TradeInfo &GetBidTrade() { return bidTrade_; }
  const TradeInfo &GetAskTrade() {return askTrade_;}

private:
  TradeInfo bidTrade_;
  TradeInfo askTrade_;
  
};

using Trades = std::vector<Trade> ;
