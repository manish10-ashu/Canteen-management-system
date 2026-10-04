#ifndef CART_H
#define CART_H

#include <string>

using namespace std;

class FoodList;

class CartItem
{
public:
    int foodId;
    string foodName;
    float price;
    int quantity;

    CartItem* next;

    CartItem(int id, string name, float price, int quantity);
};

class Cart
{
private:
    CartItem* head;

public:
    Cart();

    void addItem(FoodList& menu, int foodId, int quantity);
    void displayCart();
    float calculateTotal();
    bool isEmpty();

    void removeItem(int foodId);
    void updateQuantity(int foodId, int quantity);

    void clearCart();

    CartItem* getHead();
};

#endif