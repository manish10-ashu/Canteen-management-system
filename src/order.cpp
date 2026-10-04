#include "../include/Order.h"
#include "../include/Cart.h"
#include <iostream>
#include <fstream>

using namespace std;

// ==================== ORDER ITEM ====================

OrderItem::OrderItem(int id, string name, float p, int q)
{
    foodId = id;
    foodName = name;
    price = p;
    quantity = q;
    next = nullptr;
}


// ==================== ORDER ====================

Order::Order(int id, int cId, string cName)
{
    orderId = id;
    customerId = cId;
    customerName = cName;
    totalAmount = 0;

    items = nullptr;
    next = nullptr;
}

void Order::addItem(int foodId, string foodName,
                    float price, int quantity)
{
    OrderItem* newItem =
        new OrderItem(foodId, foodName, price, quantity);

    if (items == nullptr)
    {
        items = newItem;
        return;
    }

    OrderItem* temp = items;

    while (temp->next != nullptr)
    {
        temp = temp->next;
    }

    temp->next = newItem;
}

void Order::calculateTotal()
{
    totalAmount = 0;

    OrderItem* temp = items;

    while (temp != nullptr)
    {
        totalAmount += temp->price * temp->quantity;
        temp = temp->next;
    }
}

void Order::display()
{
    cout << "\nOrder ID: " << orderId << endl;
    cout << "Customer ID: " << customerId << endl;
    cout << "Customer Name: " << customerName << endl;

    cout << "\nItems:" << endl;

    OrderItem* temp = items;

    while (temp != nullptr)
    {
        cout << "Food ID: " << temp->foodId << endl;
        cout << "Food: " << temp->foodName << endl;
        cout << "Price: Rs. " << temp->price << endl;
        cout << "Quantity: " << temp->quantity << endl;
        cout << "Amount: Rs. "
             << temp->price * temp->quantity << endl;

        cout << "--------------------------" << endl;

        temp = temp->next;
    }

    cout << "Total Amount: Rs. " << totalAmount << endl;
}

void Order::displayBill()
{
    cout << "\n========== BILL ==========" << endl;

    cout << "Order ID: " << orderId << endl;
    cout << "Customer: " << customerName << endl;

    cout << "\nItems:" << endl;

    OrderItem* temp = items;

    while (temp != nullptr)
    {
        cout << temp->foodName
             << " x" << temp->quantity
             << " = Rs. "
             << temp->price * temp->quantity
             << endl;

        temp = temp->next;
    }

    cout << "--------------------------" << endl;
    cout << "Total Amount: Rs. "
         << totalAmount << endl;

    cout << "==========================" << endl;
}


// ==================== ORDER QUEUE ====================

OrderQueue::OrderQueue()
{
    front = nullptr;
    rear = nullptr;

    historyHead = nullptr;
    historyTail = nullptr;
}


// ==================== PLACE ORDER ====================

void OrderQueue::placeOrder(int orderId, Cart& cart,
                            int customerId,
                            string customerName)
{
    if (cart.isEmpty())
    {
        cout << "\nCart is empty." << endl;
        return;
    }

    Order* newOrder =
        new Order(orderId, customerId, customerName);

    /*
       Cart contains:

       CartItem -> CartItem -> CartItem

       We copy those items into:

       OrderItem -> OrderItem -> OrderItem
    */

    CartItem* cartItem = cart.getHead();

    while (cartItem != nullptr)
    {
        newOrder->addItem(
            cartItem->foodId,
            cartItem->foodName,
            cartItem->price,
            cartItem->quantity
        );

        cartItem = cartItem->next;
    }

    newOrder->calculateTotal();

    // Show bill before confirmation
    newOrder->displayBill();

    char confirm;

    cout << "\nConfirm order? (y/n): ";
    cin >> confirm;

    if (confirm != 'y' && confirm != 'Y')
    {
        cout << "Order cancelled." << endl;

        delete newOrder;
        return;
    }

    // Add order to FIFO queue
    if (front == nullptr)
    {
        front = newOrder;
        rear = newOrder;
    }
    else
    {
        rear->next = newOrder;
        rear = newOrder;
    }

    cout << "\nOrder placed successfully!" << endl;
    cout << "Order ID: " << orderId << endl;
    cout << "Order is now pending." << endl;

    // Empty cart after successful order
    cart.clearCart();
}


// ==================== DISPLAY PENDING ORDERS ====================

void OrderQueue::displayOrders()
{
    if (front == nullptr)
    {
        cout << "\nNo pending orders." << endl;
        return;
    }

    cout << "\n========== PENDING ORDERS ==========" << endl;

    Order* temp = front;

    while (temp != nullptr)
    {
        temp->display();

        cout << "====================================" << endl;

        temp = temp->next;
    }
}


// ==================== PROCESS ORDER ====================

void OrderQueue::processOrder()
{
    if (front == nullptr)
    {
        cout << "\nNo pending orders." << endl;
        return;
    }

    Order* temp = front;

    cout << "\n========== PROCESSING ORDER ==========" << endl;

    temp->display();

    cout << "\nOrder is being prepared..." << endl;

    cout << "\nPress Enter when order is ready for delivery...";

    cin.ignore();
    cin.get();

    cout << "\nOrder is ready for delivery." << endl;

    temp->displayBill();

    cout << "\nPayment Status: Pending" << endl;

    cout << "\nPress Enter after receiving payment...";

    cin.get();

    cout << "\nPayment received successfully." << endl;
    cout << "Order completed!" << endl;

    // Remove from pending queue
    front = front->next;

    if (front == nullptr)
    {
        rear = nullptr;
    }

    temp->next = nullptr;

    // Add to history
    if (historyHead == nullptr)
    {
        historyHead = temp;
        historyTail = temp;
    }
    else
    {
        historyTail->next = temp;
        historyTail = temp;
    }

    saveHistoryToFile();

    cout << "\nOrder moved to history." << endl;
}


// ==================== DISPLAY HISTORY ====================

void OrderQueue::displayHistory()
{
    if (historyHead == nullptr)
    {
        cout << "\nNo completed orders." << endl;
        return;
    }

    cout << "\n========== ORDER HISTORY ==========" << endl;

    Order* temp = historyHead;

    while (temp != nullptr)
    {
        temp->display();

        cout << "===================================" << endl;

        temp = temp->next;
    }
}


// ==================== CANCEL ORDER ====================

void OrderQueue::cancelOrder(int orderId)
{
    if (front == nullptr)
    {
        cout << "\nNo pending orders." << endl;
        return;
    }

    Order* temp = front;
    Order* previous = nullptr;

    while (temp != nullptr)
    {
        if (temp->orderId == orderId)
        {
            if (previous == nullptr)
            {
                front = temp->next;
            }
            else
            {
                previous->next = temp->next;
            }

            if (temp == rear)
            {
                rear = previous;
            }

            delete temp;

            cout << "\nOrder cancelled successfully." << endl;

            return;
        }

        previous = temp;
        temp = temp->next;
    }

    cout << "\nOrder not found." << endl;
}


// ==================== CUSTOMER PENDING ORDERS ====================

void OrderQueue::displayCustomerOrders(int customerId)
{
    if (front == nullptr)
    {
        cout << "\nYou have no pending orders." << endl;
        return;
    }

    Order* temp = front;
    bool found = false;

    cout << "\n========== MY PENDING ORDERS ==========" << endl;

    while (temp != nullptr)
    {
        if (temp->customerId == customerId)
        {
            temp->display();

            cout << "=======================================" << endl;

            found = true;
        }

        temp = temp->next;
    }

    if (!found)
    {
        cout << "You have no pending orders." << endl;
    }
}


// ==================== CUSTOMER CANCEL ====================

void OrderQueue::cancelCustomerOrder(int orderId,
                                     int customerId)
{
    if (front == nullptr)
    {
        cout << "\nNo pending orders." << endl;
        return;
    }

    Order* temp = front;
    Order* previous = nullptr;

    while (temp != nullptr)
    {
        if (temp->orderId == orderId)
        {
            if (temp->customerId != customerId)
            {
                cout << "\nThis order does not belong to you."
                     << endl;

                return;
            }

            if (previous == nullptr)
            {
                front = temp->next;
            }
            else
            {
                previous->next = temp->next;
            }

            if (temp == rear)
            {
                rear = previous;
            }

            delete temp;

            cout << "\nYour order has been cancelled."
                 << endl;

            return;
        }

        previous = temp;
        temp = temp->next;
    }

    cout << "\nOrder not found." << endl;
}


// ==================== CUSTOMER HISTORY ====================

void OrderQueue::displayCustomerHistory(int customerId)
{
    if (historyHead == nullptr)
    {
        cout << "\nYou have no order history." << endl;
        return;
    }

    Order* temp = historyHead;
    bool found = false;

    cout << "\n========== MY ORDER HISTORY =========="
         << endl;

    while (temp != nullptr)
    {
        if (temp->customerId == customerId)
        {
            temp->display();

            cout << "======================================"
                 << endl;

            found = true;
        }

        temp = temp->next;
    }

    if (!found)
    {
        cout << "You have no completed orders." << endl;
    }
}


// ==================== SAVE HISTORY ====================

void OrderQueue::saveHistoryToFile()
{
    ofstream file("data/orders.txt");

    if (!file)
    {
        cout << "Unable to open orders file." << endl;
        return;
    }

    Order* order = historyHead;

    while (order != nullptr)
    {
        int itemCount = 0;

        OrderItem* item = order->items;

        while (item != nullptr)
        {
            itemCount++;
            item = item->next;
        }

        /*
            First line:

            orderId customerId customerName total itemCount

            Then itemCount lines:

            foodId foodName price quantity
        */

        file << order->orderId << " "
             << order->customerId << " "
             << order->customerName << " "
             << order->totalAmount << " "
             << itemCount << endl;

        item = order->items;

        while (item != nullptr)
        {
            file << item->foodId << " "
                 << item->foodName << " "
                 << item->price << " "
                 << item->quantity << endl;

            item = item->next;
        }

        order = order->next;
    }

    file.close();
}


// ==================== LOAD HISTORY ====================

void OrderQueue::loadHistoryFromFile()
{
    ifstream file("data/orders.txt");

    if (!file)
    {
        return;
    }

    while (true)
    {
        int orderId;
        int customerId;
        string customerName;
        float totalAmount;
        int itemCount;

        if (!(file >> orderId
                    >> customerId
                    >> customerName
                    >> totalAmount
                    >> itemCount))
        {
            break;
        }

        Order* newOrder =
            new Order(orderId,
                      customerId,
                      customerName);

        for (int i = 0; i < itemCount; i++)
        {
            int foodId;
            string foodName;
            float price;
            int quantity;

            file >> foodId
                 >> foodName
                 >> price
                 >> quantity;

            newOrder->addItem(
                foodId,
                foodName,
                price,
                quantity
            );
        }

        newOrder->calculateTotal();

        if (historyHead == nullptr)
        {
            historyHead = newOrder;
            historyTail = newOrder;
        }
        else
        {
            historyTail->next = newOrder;
            historyTail = newOrder;
        }
    }

    file.close();
}