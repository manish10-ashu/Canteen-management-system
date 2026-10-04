#ifndef FOOD_H
#define FOOD_H

#include <string>
using namespace std;

class Food {
public:
    int id;
    string name;
    string category;
    float price;
    bool available;

    Food* next;

    Food(int id, string name, string category, float price, bool available = true);

    void display();
};

class FoodList {
private:
    Food* head;

public:
    FoodList();

    void addFood(int id, string name, string category, float price);
    void displayMenu();
    void searchFood(int id);
    void updateFood(int id);
    void deleteFood(int id);

    Food* findFood(int id);
    void saveToFile();
    void loadFromFile();
};

#endif