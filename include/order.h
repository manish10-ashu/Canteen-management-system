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

    // New multiple-item order
    void placeOrder(int orderId, Cart& cart,
                    int customerId, string customerName);

    void displayOrders();
    void processOrder();
    void displayHistory();
    void cancelOrder(int orderId);

    void displayCustomerOrders(int customerId);
    void cancelCustomerOrder(int orderId, int customerId);
    void displayCustomerHistory(int customerId);

    void saveHistoryToFile();
    void loadHistoryFromFile();
};

#endif