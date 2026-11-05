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

    // Display fields so the website can render the menu the same way
    // it always has (emoji / description / colour) straight from the core.
    string emoji;
    string description;
    string color;

    Food* next;

    Food(int id, string name, string category, float price,
         bool available = true,
         string emoji = "",
         string description = "",
         string color = "");

    void display();
};

class FoodList {
private:
    Food* head;

public:
    FoodList();

    void addFood(int id, string name, string category, float price,
                 bool available = true,
                 string emoji = "",
                 string description = "",
                 string color = "");

    void displayMenu();
    void searchFood(int id);

    // Interactive (console) version - asks for the new values.
    void updateFood(int id);

    // Programmatic version - used by the website API.
    bool updateFood(int id, string name, string category, float price,
                    bool available,
                    string emoji, string description, string color);

    void deleteFood(int id);

    Food* findFood(int id);
    Food* getHead();

    void saveToFile();
    void loadFromFile();
};

#endif
