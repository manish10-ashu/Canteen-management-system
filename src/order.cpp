#include "../include/order.h"
#include "../include/cart.h"
#include <iostream>

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
    status = "Pending";

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
    cout << "Status: " << status << endl;

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

// Copy the cart's linked list into a brand new order (shared by both the
// interactive and the programmatic entry points so the logic lives once).
static Order* buildOrderFromCart(int orderId, Cart& cart,
                                 int customerId,
                                 const string& customerName)
{
    Order* order = new Order(orderId, customerId, customerName);

    CartItem* cartItem = cart.getHead();

    while (cartItem != nullptr)
    {
        order->addItem(
            cartItem->foodId,
            cartItem->foodName,
            cartItem->price,
            cartItem->quantity
        );

        cartItem = cartItem->next;
    }

    order->calculateTotal();

    return order;
}


// ==================== PLACE ORDER (CONSOLE) ====================

void OrderQueue::placeOrder(int orderId, Cart& cart,
                            int customerId,
                            string customerName)
{
    if (cart.isEmpty())
    {
        cout << "\nCart is empty." << endl;
        return;
    }

    // Show the bill before confirmation.
    Order* preview =
        buildOrderFromCart(orderId, cart, customerId, customerName);

    preview->displayBill();

    char confirm;

    cout << "\nConfirm order? (y/n): ";
    cin >> confirm;

    if (confirm != 'y' && confirm != 'Y')
    {
        cout << "Order cancelled." << endl;
        return;
    }

    placeOrderConfirmed(orderId, cart, customerId, customerName);

    cout << "\nOrder placed successfully!" << endl;
    cout << "Order ID: " << orderId << endl;
    cout << "Order is now pending." << endl;
}


// ==================== PLACE ORDER (PROGRAMMATIC) ====================

void OrderQueue::placeOrderConfirmed(int orderId, Cart& cart,
                                     int customerId,
                                     string customerName)
{
    if (cart.isEmpty())
    {
        return;
    }

    Order* newOrder =
        buildOrderFromCart(orderId, cart, customerId, customerName);

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
    temp->status = "Delivered";

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

    cout << "\nOrder moved to history." << endl;
}


// ==================== UPDATE STATUS (PROGRAMMATIC) ====================

bool OrderQueue::updateOrderStatus(int orderId, string status)
{
    if (status == "Delivered")
    {
        // Find the order in the pending queue, unlink it and move it to history.
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

                temp->next = nullptr;
                temp->status = "Delivered";

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

                return true;
            }

            previous = temp;
            temp = temp->next;
        }

        return false;
    }

    // Pending / Preparing - just update the status in place.
    Order* temp = front;

    while (temp != nullptr)
    {
        if (temp->orderId == orderId)
        {
            temp->status = status;
            return true;
        }

        temp = temp->next;
    }

    return false;
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


// ==================== ACCESSORS / HELPERS ====================

Order* OrderQueue::getPendingHead()
{
    return front;
}

Order* OrderQueue::getHistoryHead()
{
    return historyHead;
}

int OrderQueue::getNextOrderId()
{
    int maxId = 1000;

    Order* temp = front;

    while (temp != nullptr)
    {
        if (temp->orderId > maxId)
        {
            maxId = temp->orderId;
        }

        temp = temp->next;
    }

    temp = historyHead;

    while (temp != nullptr)
    {
        if (temp->orderId > maxId)
        {
            maxId = temp->orderId;
        }

        temp = temp->next;
    }

    return maxId + 1;
}


// ==================== REBUILD FROM STATE ====================

// Re-create an order that was persisted outside the core (MongoDB). Links it
// into the FIFO queue or the history list based on its status and returns it
// so the caller can attach the line items and recompute the total.
Order* OrderQueue::addOrderFromState(int orderId, int customerId,
                                     string customerName, string status)
{
    Order* order = new Order(orderId, customerId, customerName);
    order->status = status;

    if (status == "Delivered")
    {
        if (historyHead == nullptr)
        {
            historyHead = order;
            historyTail = order;
        }
        else
        {
            historyTail->next = order;
            historyTail = order;
        }
    }
    else
    {
        if (front == nullptr)
        {
            front = order;
            rear = order;
        }
        else
        {
            rear->next = order;
            rear = order;
        }
    }

    return order;
}
