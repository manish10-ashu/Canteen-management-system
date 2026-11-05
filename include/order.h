#ifndef ORDER_H
#define ORDER_H

#include <string>

using namespace std;

class FoodList;
class Cart;

class OrderItem
{
public:
    int foodId;
    string foodName;
    float price;
    int quantity;

    OrderItem* next;

    OrderItem(int id, string name, float price, int quantity);
};

class Order
{
public:
    int orderId;
    int customerId;
    string customerName;
    float totalAmount;
    string status;      // Pending | Preparing | Delivered

    OrderItem* items;

    Order* next;

    Order(int orderId, int customerId,
          string customerName);

    void addItem(int foodId, string foodName,
                 float price, int quantity);

    void calculateTotal();
    void display();
    void displayBill();
};

class OrderQueue
{
private:
    Order* front;
    Order* rear;

    Order* historyHead;
    Order* historyTail;

public:
    OrderQueue();

    // Interactive (console) version - shows the bill and asks to confirm.
    void placeOrder(int orderId, Cart& cart,
                    int customerId, string customerName);

    // Programmatic version - used by the website API, no prompts.
    void placeOrderConfirmed(int orderId, Cart& cart,
                             int customerId, string customerName);

    void displayOrders();
    void processOrder();

    // Programmatic status change used by the website API.
    // Pending/Preparing stay in the queue; Delivered moves to history.
    bool updateOrderStatus(int orderId, string status);

    void displayHistory();
    void cancelOrder(int orderId);

    void displayCustomerOrders(int customerId);
    void cancelCustomerOrder(int orderId, int customerId);
    void displayCustomerHistory(int customerId);

    int getNextOrderId();

    Order* getPendingHead();
    Order* getHistoryHead();

    // Rebuild an order from persisted state (used when the backend feeds the
    // database state into the core on stdin). Links it into the FIFO queue or
    // the history list according to its status and returns it so the caller
    // can attach its items.
    Order* addOrderFromState(int orderId, int customerId,
                             string customerName, string status);
};

#endif
