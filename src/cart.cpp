#include "../include/Cart.h"
#include "../include/Food.h"
#include <iostream>

using namespace std;

CartItem::CartItem(int id, string name, float p, int q)
{
    foodId = id;
    foodName = name;
    price = p;
    quantity = q;
    next = nullptr;
}

Cart::Cart()
{
    head = nullptr;
}


// ==================== ADD ITEM ====================

void Cart::addItem(FoodList& menu, int foodId, int quantity)
{
    Food* food = menu.findFood(foodId);

    if (food == nullptr)
    {
        cout << "Food not found." << endl;
        return;
    }

    if (!food->available)
    {
        cout << "Food is currently unavailable." << endl;
        return;
    }

    if (quantity <= 0)
    {
        cout << "Invalid quantity." << endl;
        return;
    }

    CartItem* temp = head;

    // If item already exists, increase quantity
    while (temp != nullptr)
    {
        if (temp->foodId == foodId)
        {
            temp->quantity += quantity;

            cout << "Quantity updated in cart." << endl;

            return;
        }

        temp = temp->next;
    }

    // Create new cart item
    CartItem* newItem =
        new CartItem(
            foodId,
            food->name,
            food->price,
            quantity
        );

    if (head == nullptr)
    {
        head = newItem;
        cout << "Food added to cart." << endl;
        return;
    }

    temp = head;

    while (temp->next != nullptr)
    {
        temp = temp->next;
    }

    temp->next = newItem;

    cout << "Food added to cart." << endl;
}


// ==================== DISPLAY CART ====================

void Cart::displayCart()
{
    if (head == nullptr)
    {
        cout << "\nCart is empty." << endl;
        return;
    }

    CartItem* temp = head;

    cout << "\n========== CART ==========" << endl;

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

    cout << "Total Amount: Rs. "
         << calculateTotal() << endl;
}


// ==================== CALCULATE TOTAL ====================

float Cart::calculateTotal()
{
    float total = 0;

    CartItem* temp = head;

    while (temp != nullptr)
    {
        total += temp->price * temp->quantity;

        temp = temp->next;
    }

    return total;
}


// ==================== CHECK EMPTY ====================

bool Cart::isEmpty()
{
    return head == nullptr;
}


// ==================== REMOVE ITEM ====================

void Cart::removeItem(int foodId)
{
    if (head == nullptr)
    {
        cout << "\nCart is empty." << endl;
        return;
    }

    CartItem* temp = head;
    CartItem* previous = nullptr;

    while (temp != nullptr)
    {
        if (temp->foodId == foodId)
        {
            // Removing first item
            if (previous == nullptr)
            {
                head = temp->next;
            }
            else
            {
                previous->next = temp->next;
            }

            delete temp;

            cout << "Item removed from cart." << endl;

            return;
        }

        previous = temp;
        temp = temp->next;
    }

    cout << "Food item not found in cart." << endl;
}


// ==================== UPDATE QUANTITY ====================

void Cart::updateQuantity(int foodId, int quantity)
{
    if (quantity <= 0)
    {
        cout << "Invalid quantity." << endl;
        return;
    }

    CartItem* temp = head;

    while (temp != nullptr)
    {
        if (temp->foodId == foodId)
        {
            temp->quantity = quantity;

            cout << "Quantity updated successfully." << endl;

            return;
        }

        temp = temp->next;
    }

    cout << "Food item not found in cart." << endl;
}


// ==================== CLEAR CART ====================

void Cart::clearCart()
{
    CartItem* temp;

    while (head != nullptr)
    {
        temp = head;
        head = head->next;

        delete temp;
    }
}


// ==================== GET HEAD ====================

CartItem* Cart::getHead()
{
    return head;
}