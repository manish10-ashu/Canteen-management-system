#include "../include/food.h"
#include "../include/util.h"
#include <iostream>
#include <fstream>

using namespace std;

Food::Food(int id, string name, string category, float price,
           bool available, string emoji, string description, string color)
{
    this->id = id;
    this->name = name;
    this->category = category;
    this->price = price;
    this->available = available;
    this->emoji = emoji;
    this->description = description;
    this->color = color;
    this->next = nullptr;
}

void Food::display() {
    cout << "ID: " << id << endl;
    cout << "Name: " << name << endl;
    cout << "Category: " << category << endl;
    cout << "Price: Rs. " << price << endl;
    cout << "Available: " << (available ? "Yes" : "No") << endl;
}

FoodList::FoodList() {
    head = nullptr;
}

void FoodList::addFood(int id, string name, string category, float price,
                       bool available, string emoji, string description,
                       string color) {

    Food* newFood = new Food(id, name, category, price,
                             available, emoji, description, color);

    if (head == nullptr) {
        head = newFood;
        saveToFile();
        return;
    }

    Food* temp = head;

    while (temp->next != nullptr) {
        temp = temp->next;
    }

    temp->next = newFood;
    saveToFile();
}

void FoodList::displayMenu() {

    if (head == nullptr) {
        cout << "Menu is empty." << endl;
        return;
    }

    Food* temp = head;

    cout << "\n========== CANTEEN MENU ==========" << endl;

    while (temp != nullptr) {
        temp->display();
        cout << "----------------------------------" << endl;
        temp = temp->next;
    }
}

void FoodList::searchFood(int id) {

    Food* temp = head;

    while (temp != nullptr) {

        if (temp->id == id) {
            cout << "\nFood Found:" << endl;
            temp->display();
            return;
        }

        temp = temp->next;
    }

    cout << "Food with ID " << id << " not found." << endl;
}

void FoodList::updateFood(int id) {

    Food* temp = head;

    while (temp != nullptr) {

        if (temp->id == id) {

            cout << "Enter new name: ";
            cin >> temp->name;

            cout << "Enter new category: ";
            cin >> temp->category;

            cout << "Enter new price: ";
            cin >> temp->price;

            saveToFile();

            cout << "Food updated successfully." << endl;
            return;
        }

        temp = temp->next;
    }

    cout << "Food with ID " << id << " not found." << endl;
}

bool FoodList::updateFood(int id, string name, string category, float price,
                          bool available, string emoji, string description,
                          string color) {

    Food* temp = head;

    while (temp != nullptr) {

        if (temp->id == id) {

            temp->name = name;
            temp->category = category;
            temp->price = price;
            temp->available = available;
            temp->emoji = emoji;
            temp->description = description;
            temp->color = color;

            saveToFile();
            return true;
        }

        temp = temp->next;
    }

    return false;
}

void FoodList::deleteFood(int id) {

    if (head == nullptr) {
        cout << "Menu is empty." << endl;
        return;
    }

    if (head->id == id) {
        Food* temp = head;
        head = head->next;
        delete temp;

        saveToFile();

        cout << "Food deleted successfully." << endl;
        return;
    }

    Food* temp = head;

    while (temp->next != nullptr && temp->next->id != id) {
        temp = temp->next;
    }

    if (temp->next == nullptr) {
        cout << "Food with ID " << id << " not found." << endl;
        return;
    }

    Food* deleteNode = temp->next;
    temp->next = deleteNode->next;

    delete deleteNode;

    saveToFile();

    cout << "Food deleted successfully." << endl;
}

Food* FoodList::findFood(int id) {

    Food* temp = head;

    while (temp != nullptr) {

        if (temp->id == id) {
            return temp;
        }

        temp = temp->next;
    }

    return nullptr;
}

Food* FoodList::getHead() {
    return head;
}

// File format (one food per line, "|" separated):
//   id|name|category|price|available|emoji|description|color
void FoodList::saveToFile()
{
    ofstream file("data/food.txt");

    if (!file)
    {
        cout << "Unable to open food file." << endl;
        return;
    }

    Food* temp = head;

    while (temp != nullptr)
    {
        file << temp->id << "|"
             << temp->name << "|"
             << temp->category << "|"
             << temp->price << "|"
             << (temp->available ? 1 : 0) << "|"
             << temp->emoji << "|"
             << temp->description << "|"
             << temp->color << endl;

        temp = temp->next;
    }

    file.close();
}

void FoodList::loadFromFile()
{
    ifstream file("data/food.txt");

    if (!file)
    {
        return;
    }

    string line;

    while (getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        vector<string> parts = splitLine(line, '|');

        if (parts.size() < 5)
        {
            continue;
        }

        int id = toInt(parts[0]);
        string name = parts[1];
        string category = parts[2];
        float price = toFloat(parts[3]);
        bool available = toInt(parts[4], 1) != 0;

        string emoji = parts.size() > 5 ? parts[5] : "";
        string description = parts.size() > 6 ? parts[6] : "";
        string color = parts.size() > 7 ? parts[7] : "";

        // Link directly (do NOT use addFood here - it would rewrite the
        // file while we are still reading it).
        Food* newFood = new Food(id, name, category, price,
                                 available, emoji, description, color);

        if (head == nullptr)
        {
            head = newFood;
        }
        else
        {
            Food* temp = head;

            while (temp->next != nullptr)
            {
                temp = temp->next;
            }

            temp->next = newFood;
        }
    }

    file.close();
}
