#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <string>

using namespace std;

class Customer
{
public:
    int customerId;
    string name;
    string phone;

    Customer* next;

    Customer(int id, string n, string p);

    void displayCustomer();
    bool login(string enteredName, string enteredPhone);
};

class CustomerList
{
private:
    Customer* head;

public:
    CustomerList();

    void addCustomer(int id, string name, string phone);
    void displayCustomers();

    Customer* searchCustomer(string name);
    Customer* searchCustomerById(int id);
    Customer* loginCustomer(string name, string phone);
    
    void saveToFile();
    void loadFromFile();
};

#endif