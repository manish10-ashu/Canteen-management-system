#include "../include/customer.h"
#include <iostream>

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
        return;
    }

    Customer* temp = head;

    while (temp->next != nullptr)
    {
        temp = temp->next;
    }

    temp->next = newCustomer;
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

Customer* CustomerList::getHead()
{
    return head;
}

int CustomerList::getNextCustomerId()
{
    int maxId = 0;

    Customer* temp = head;

    while (temp != nullptr)
    {
        if (temp->customerId > maxId)
        {
            maxId = temp->customerId;
        }

        temp = temp->next;
    }

    return maxId + 1;
}