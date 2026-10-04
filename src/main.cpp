#include "../include/Food.h"
#include "../include/Order.h"
#include "../include/Customer.h"
#include "../include/Admin.h"
#include "../include/Cart.h"
#include <iostream>

using namespace std;

int main()
{
    Admin admin("admin", "1234");

    FoodList menu;
    OrderQueue orders;
    CustomerList customers;

    menu.loadFromFile();
    customers.loadFromFile();
    orders.loadHistoryFromFile();

    int mainChoice;

    do
    {
        cout << "\n========== CANTEEN MANAGEMENT SYSTEM ==========" << endl;
        cout << "1. Admin Login" << endl;
        cout << "2. Customer" << endl;
        cout << "3. Exit" << endl;

        cout << "\nEnter your choice: ";
        cin >> mainChoice;

        // ================= ADMIN =================

        if (mainChoice == 1)
        {
            string username;
            string password;

            cout << "\n========== ADMIN LOGIN ==========" << endl;

            cout << "Enter username: ";
            cin >> username;

            cout << "Enter password: ";
            cin >> password;

            if (admin.login(username, password))
            {
                cout << "\nLogin successful." << endl;
                cout << "Welcome, Admin!" << endl;

                int adminChoice;

                do
                {
                    cout << "\n========== ADMIN MENU ==========" << endl;
                    cout << "1. Display Food Menu" << endl;
                    cout << "2. Search Food" << endl;
                    cout << "3. Add Food" << endl;
                    cout << "4. Update Food" << endl;
                    cout << "5. Delete Food" << endl;
                    cout << "6. View Pending Orders" << endl;
                    cout << "7. Process Order" << endl;
                    cout << "8. Cancel Order" << endl;
                    cout << "9. View Order History" << endl;
                    cout << "10. View Customers" << endl;
                    cout << "11. Logout" << endl;

                    cout << "\nEnter your choice: ";
                    cin >> adminChoice;

                    if (adminChoice == 1)
                    {
                        menu.displayMenu();
                    }

                    else if (adminChoice == 2)
                    {
                        int id;

                        cout << "Enter food ID: ";
                        cin >> id;

                        menu.searchFood(id);
                    }

                    else if (adminChoice == 3)
                    {
                        int id;
                        string name;
                        string category;
                        float price;

                        cout << "Enter food ID: ";
                        cin >> id;

                        cout << "Enter food name: ";
                        cin >> name;

                        cout << "Enter category: ";
                        cin >> category;

                        cout << "Enter price: ";
                        cin >> price;

                        menu.addFood(id, name, category, price);

                        cout << "Food added successfully." << endl;
                    }

                    else if (adminChoice == 4)
                    {
                        int id;

                        cout << "Enter food ID to update: ";
                        cin >> id;

                        menu.updateFood(id);
                    }

                    else if (adminChoice == 5)
                    {
                        int id;

                        cout << "Enter food ID to delete: ";
                        cin >> id;

                        menu.deleteFood(id);
                    }

                    else if (adminChoice == 6)
                    {
                        orders.displayOrders();
                    }

                    else if (adminChoice == 7)
                    {
                        orders.processOrder();
                    }

                    else if (adminChoice == 8)
                    {
                        int orderId;

                        cout << "Enter order ID to cancel: ";
                        cin >> orderId;

                        orders.cancelOrder(orderId);
                    }

                    else if (adminChoice == 9)
                    {
                        orders.displayHistory();
                    }

                    else if (adminChoice == 10)
                    {
                        customers.displayCustomers();
                    }

                    else if (adminChoice == 11)
                    {
                        cout << "Logging out..." << endl;
                    }

                    else
                    {
                        cout << "Invalid choice." << endl;
                    }

                } while (adminChoice != 11);
            }

            else
            {
                cout << "\nInvalid username or password." << endl;
                cout << "Access denied." << endl;
            }
        }

        // ================= CUSTOMER =================

        else if (mainChoice == 2)
        {
            int customerOption;

            do
            {
                cout << "\n========== CUSTOMER ==========" << endl;
                cout << "1. Register" << endl;
                cout << "2. Login" << endl;
                cout << "3. Back to Main Menu" << endl;

                cout << "\nEnter your choice: ";
                cin >> customerOption;

                // -------- REGISTER --------

                if (customerOption == 1)
                {
                    int id;
                    string name;
                    string phone;

                    cout << "\n========== REGISTER CUSTOMER ==========" << endl;

                    cout << "Enter customer ID: ";
                    cin >> id;

                    cout << "Enter customer name: ";
                    cin >> name;

                    cout << "Enter phone number: ";
                    cin >> phone;

                    customers.addCustomer(id, name, phone);

                    cout << "Customer registered successfully." << endl;
                }

                // -------- LOGIN --------

                else if (customerOption == 2)
                {
                    string enteredName;
                    string enteredPhone;

                    cout << "\n========== CUSTOMER LOGIN ==========" << endl;

                    cout << "Enter customer name: ";
                    cin >> enteredName;

                    cout << "Enter phone number: ";
                    cin >> enteredPhone;

                    Customer *customer =
                        customers.loginCustomer(
                            enteredName,
                            enteredPhone);

                    if (customer == nullptr)
                    {
                        cout << "Invalid name or phone number." << endl;
                    }

                    else
                    {
                        cout << "\nLogin successful!" << endl;
                        cout << "Welcome, "
                             << customer->name << "!" << endl;

                        int customerChoice;

                        do
                        {
                            cout << "\n========== CUSTOMER MENU ==========" << endl;
                            cout << "1. View Food Menu" << endl;
                            cout << "2. Search Food" << endl;
                            cout << "3. Place Order" << endl;
                            cout << "4. View My Pending Orders" << endl;
                            cout << "5. Cancel My Order" << endl;
                            cout << "6. View My Order History" << endl;
                            cout << "7. Logout" << endl;

                            cout << "\nEnter your choice: ";
                            cin >> customerChoice;

                            // View food menu
                            if (customerChoice == 1)
                            {
                                menu.displayMenu();
                            }

                            // Search food
                            else if (customerChoice == 2)
                            {
                                int id;

                                cout << "Enter food ID: ";
                                cin >> id;

                                menu.searchFood(id);
                            }

                            // Place order
                            else if (customerChoice == 3)
                            {
                                int orderId;
                                int foodId;
                                int quantity;
                                int cartChoice;

                                Cart cart;

                                cout << "\n========== PLACE ORDER ==========" << endl;

                                cout << "Enter order ID: ";
                                cin >> orderId;

                                do
                                {
                                    cout << "\n========== ORDER MENU ==========" << endl;
                                    cout << "1. View Food Menu" << endl;
                                    cout << "2. Add Food" << endl;
                                    cout << "3. View Cart" << endl;
                                    cout << "4. Remove Food" << endl;
                                    cout << "5. Change Quantity" << endl;
                                    cout << "6. Checkout" << endl;
                                    cout << "7. Cancel Order" << endl;

                                    cout << "\nEnter your choice: ";
                                    cin >> cartChoice;

                                    if (cartChoice == 1)
                                    {
                                        menu.displayMenu();
                                    }

                                    else if (cartChoice == 2)
                                    {
                                        cout << "\nEnter food ID: ";
                                        cin >> foodId;

                                        cout << "Enter quantity: ";
                                        cin >> quantity;

                                        cart.addItem(menu, foodId, quantity);
                                    }

                                    else if (cartChoice == 3)
                                    {
                                        cart.displayCart();
                                    }

                                    else if (cartChoice == 4)
                                    {
                                        if (cart.isEmpty())
                                        {
                                            cout << "\nCart is empty." << endl;
                                        }
                                        else
                                        {
                                            cart.displayCart();

                                            cout << "\nEnter food ID to remove: ";
                                            cin >> foodId;

                                            cart.removeItem(foodId);
                                        }
                                    }

                                    else if (cartChoice == 5)
                                    {
                                        if (cart.isEmpty())
                                        {
                                            cout << "\nCart is empty." << endl;
                                        }
                                        else
                                        {
                                            cart.displayCart();

                                            cout << "\nEnter food ID: ";
                                            cin >> foodId;

                                            cout << "Enter new quantity: ";
                                            cin >> quantity;

                                            cart.updateQuantity(foodId, quantity);
                                        }
                                    }

                                    else if (cartChoice == 6)
                                    {
                                        if (cart.isEmpty())
                                        {
                                            cout << "\nCart is empty. Add food first." << endl;
                                        }
                                        else
                                        {
                                            orders.placeOrder(
                                                orderId,
                                                cart,
                                                customer->customerId,
                                                customer->name);

                                            // Exit ordering menu after checkout
                                            cartChoice = 7;
                                        }
                                    }

                                    else if (cartChoice == 7)
                                    {
                                        cout << "\nOrder cancelled." << endl;
                                    }

                                    else
                                    {
                                        cout << "\nInvalid choice." << endl;
                                    }

                                } while (cartChoice != 7);
                            }

                            // View pending orders
                            else if (customerChoice == 4)
                            {
                                orders.displayCustomerOrders(customer->customerId);
                            }

                            // Cancel order
                            else if (customerChoice == 5)
                            {
                                int orderId;

                                cout << "Enter order ID to cancel: ";
                                cin >> orderId;

                                orders.cancelCustomerOrder(
                                    orderId, customer->customerId);
                            }

                            // View history
                            else if (customerChoice == 6)
                            {
                                orders.displayCustomerHistory(customer->customerId);
                            }

                            // Logout
                            else if (customerChoice == 7)
                            {
                                cout << "Logging out..." << endl;
                            }

                            else
                            {
                                cout << "Invalid choice." << endl;
                            }

                        } while (customerChoice != 7);
                    }
                }

                // -------- BACK --------

                else if (customerOption == 3)
                {
                    cout << "Returning to main menu..." << endl;
                }

                else
                {
                    cout << "Invalid choice." << endl;
                }

            } while (customerOption != 3);
        }

        // ================= EXIT =================

        else if (mainChoice == 3)
        {
            cout << "Exiting system..." << endl;
        }

        else
        {
            cout << "Invalid choice." << endl;
        }

    } while (mainChoice != 3);

    return 0;
}