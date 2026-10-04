#include "../include/Customer.h"
#include <iostream>
#include <fstream>

using namespace std;

Customer::Customer(int id, string n, string p)
{
    customerId = id;
    name = n;
    phone = p;
    next = nullptr;
}

void Customer::displayCustomer()
{
    cout << "Customer ID: " << customerId << endl;
    cout << "Name: " << name << endl;
    cout << "Phone: " << phone << endl;
}

CustomerList::CustomerList()
{
    head = nullptr;
}

void CustomerList::addCustomer(int id, string name, string phone)
{
    Customer* newCustomer = new Customer(id, name, phone);

    if (head == nullptr)
    {
        head = newCustomer;
         saveToFile();
        return;
    }

    Customer* temp = head;

    while (temp->next != nullptr)
    {
        temp = temp->next;
    }

    temp->next = newCustomer;
     saveToFile();
}

void CustomerList::displayCustomers()
{
    if (head == nullptr)
    {
        cout << "No customers registered." << endl;
        return;
    }

    Customer* temp = head;

    cout << "\n========== CUSTOMERS ==========" << endl;

    while (temp != nullptr)
    {
        temp->displayCustomer();
        cout << "------------------------------------" << endl;

        temp = temp->next;
    }
}

bool Customer::login(string enteredName, string enteredPhone)
{
    if (name == enteredName && phone == enteredPhone)
    {
        return true;
    }

    return false;
}

Customer* CustomerList::searchCustomer(string name)
{
    Customer* temp = head;

    while (temp != nullptr)
    {
        if (temp->name == name)
        {
            return temp;
        }

        temp = temp->next;
    }

    return nullptr;
}
 Customer* CustomerList::searchCustomerById(int id)
{
    Customer* temp = head;

    while (temp != nullptr)
    {
        if (temp->customerId == id)
        {
            return temp;
        }

        temp = temp->next;
    }

    return nullptr;
}

Customer* CustomerList::loginCustomer(string name, string phone)
{
    Customer* temp = head;

    while (temp != nullptr)
    {
        if (temp->name == name && temp->phone == phone)
        {
            return temp;
        }

        temp = temp->next;
    }

    return nullptr;
}

void CustomerList::saveToFile()
{
    ofstream file("data/customers.txt");

    if (!file)
    {
        cout << "Unable to open customer file." << endl;
        return;
    }

    Customer* temp = head;

    while (temp != nullptr)
    {
        file << temp->customerId << " "
             << temp->name << " "
             << temp->phone << endl;

        temp = temp->next;
    }

    file.close();
}

void CustomerList::loadFromFile()
{
    ifstream file("data/customers.txt");

    if (!file)
    {
        return;
    }

    int id;
    string name;
    string phone;

    while (file >> id >> name >> phone)
    {
        Customer* newCustomer =
            new Customer(id, name, phone);

        if (head == nullptr)
        {
            head = newCustomer;
        }
        else
        {
            Customer* temp = head;

            while (temp->next != nullptr)
            {
                temp = temp->next;
            }

            temp->next = newCustomer;
        }
    }

    file.close();
}