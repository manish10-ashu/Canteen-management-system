
#include "../include/food.h"
#include "../include/order.h"
#include "../include/customer.h"
#include "../include/admin.h"
#include "../include/cart.h"
#include "../include/api.h"

#include <iostream>
#include <string>

using namespace std;

// Read a complete line, including spaces.
void readLine(string &value)
{
    cin >> ws;
    getline(cin, value);
}

// Display an API error safely.
void showApiError(const string &response)
{
    string message = apiMessage(response);

    if (message.empty())
        cout << "Request failed. Check the backend connection.\n";
    else
        cout << message << endl;
}

// ================= ADMIN MENU =================

void adminMenu(FoodList &menu, const string &adminToken)
{
    int choice;

    do
    {
        cout << "\n========== ADMIN MENU ==========\n";
        cout << "1. Display Food Menu\n";
        cout << "2. Search Food\n";
        cout << "3. Add Food\n";
        cout << "4. Update Food\n";
        cout << "5. Delete Food\n";
        cout << "6. View All Orders\n";
        cout << "7. Process / Update Order\n";
        cout << "8. Cancel Order\n";
        cout << "9. View Customers\n";
        cout << "10. Logout\n";

        cout << "\nEnter your choice: ";
        cin >> choice;

        // Display menu
        if (choice == 1)
        {
            if (loadFoodFromApi(menu))
                menu.displayMenu();
            else
                cout << "Unable to load food menu.\n";
        }

        // Search food
        else if (choice == 2)
        {
            int id;

            cout << "Enter food ID: ";
            cin >> id;

            loadFoodFromApi(menu);
            menu.searchFood(id);
        }

        // Add food
        else if (choice == 3)
        {
            int id;
            string name, category;
            float price;

            cout << "Enter food ID: ";
            cin >> id;

            cout << "Enter food name: ";
            readLine(name);

            cout << "Enter category: ";
            readLine(category);

            cout << "Enter price: ";
            cin >> price;

            if (apiAddFood(id, name, category, price, adminToken))
            {
                cout << "\nFood added successfully!\n";
                loadFoodFromApi(menu);
            }
            else
            {
                cout << "\n";
            }
        }

        // Update food
        else if (choice == 4)
        {
            int id;
            string name, category;
            float price;

            cout << "Enter food ID to update: ";
            cin >> id;

            loadFoodFromApi(menu);

            Food *food = menu.findFood(id);

            if (food == nullptr)
            {
                cout << "Food not found.\n";
            }
            else
            {
                cout << "Enter new food name: ";
                readLine(name);

                cout << "Enter new category: ";
                readLine(category);

                cout << "Enter new price: ";
                cin >> price;

                if (apiUpdateFood(id, name, category, price, adminToken))
                {
                    menu.updateFood(
                        id,
                        name,
                        category,
                        price,
                        food->available,
                        food->emoji,
                        food->description,
                        food->color);

                    cout << "\nFood updated successfully!\n";
                }
            }
        }

        // Delete food
        else if (choice == 5)
        {
            int id;

            cout << "Enter food ID to delete: ";
            cin >> id;

            loadFoodFromApi(menu);

            if (menu.findFood(id) == nullptr)
            {
                cout << "Food not found.\n";
            }
            else if (apiDeleteFood(id, adminToken))
            {
                menu.deleteFood(id);
                cout << "Food deleted successfully!\n";
            }
        }

        // View all orders from MongoDB
        else if (choice == 6)
        {
            string response = apiGetAdminOrders(adminToken);

            if (apiSuccess(response))
                apiDisplayOrders(response, "orders");
            else
                showApiError(response);
        }

        // Process / update order
        else if (choice == 7)
        {
            int orderId;
            string status;

            cout << "Enter order ID: ";
            cin >> orderId;

            cout << "\nAvailable statuses:\n";
            cout << "1. Pending\n";
            cout << "2. Preparing\n";
            cout << "3. Delivered\n";

            int statusChoice;

            cout << "Choose status: ";
            cin >> statusChoice;

            if (statusChoice == 1)
                status = "Pending";
            else if (statusChoice == 2)
                status = "Preparing";
            else if (statusChoice == 3)
                status = "Delivered";
            else
            {
                cout << "Invalid status choice.\n";
                status.clear();
            }

            if (!status.empty())
            {
                string response =
                    apiUpdateOrderStatus(orderId, status, adminToken);

                if (apiSuccess(response))
                    cout << "Order status updated successfully!\n";
                else
                    showApiError(response);
            }
        }

        // Cancel order
        else if (choice == 8)
        {
            int orderId;

            cout << "Enter order ID to cancel: ";
            cin >> orderId;

            string response =
                apiCancelAdminOrder(orderId, adminToken);

            if (apiSuccess(response))
                cout << "Order cancelled successfully!\n";
            else
                showApiError(response);
        }

        // View customers from MongoDB
        else if (choice == 9)
        {
            string response = apiGetCustomers(adminToken);

            apiDisplayCustomers(response);
        }

        else if (choice == 10)
        {
            cout << "Logging out from admin account...\n";
        }

        else
        {
            cout << "Invalid choice.\n";
        }

    } while (choice != 10);
}

// ================= CUSTOMER ORDER MENU =================

void customerOrderMenu(
    FoodList &menu,
    const string &name,
    const string &phone,
    int customerId,
    const string &customerToken)
{
    Cart cart;
    int choice;

    do
    {
        cout << "\n========== ORDER MENU ==========\n";
        cout << "1. View Food Menu\n";
        cout << "2. Add Food\n";
        cout << "3. View Cart\n";
        cout << "4. Remove Food\n";
        cout << "5. Change Quantity\n";
        cout << "6. Checkout\n";
        cout << "7. Cancel Order\n";

        cout << "\nEnter your choice: ";
        cin >> choice;

        if (choice == 1)
        {
            loadFoodFromApi(menu);
            menu.displayMenu();
        }

        else if (choice == 2)
        {
            int foodId, quantity;

            cout << "Enter food ID: ";
            cin >> foodId;

            cout << "Enter quantity: ";
            cin >> quantity;

            if (quantity <= 0)
                cout << "Quantity must be greater than zero.\n";
            else
                cart.addItem(menu, foodId, quantity);
        }

        else if (choice == 3)
        {
            if (cart.isEmpty())
                cout << "Your cart is empty.\n";
            else
                cart.displayCart();
        }

        else if (choice == 4)
        {
            if (cart.isEmpty())
            {
                cout << "Your cart is empty.\n";
            }
            else
            {
                int foodId;

                cart.displayCart();

                cout << "Enter food ID to remove: ";
                cin >> foodId;

                cart.removeItem(foodId);
            }
        }

        else if (choice == 5)
        {
            if (cart.isEmpty())
            {
                cout << "Your cart is empty.\n";
            }
            else
            {
                int foodId, quantity;

                cart.displayCart();

                cout << "Enter food ID: ";
                cin >> foodId;

                cout << "Enter new quantity: ";
                cin >> quantity;

                if (quantity <= 0)
                    cout << "Quantity must be greater than zero.\n";
                else
                    cart.updateQuantity(foodId, quantity);
            }
        }

        // Checkout through MongoDB API
        else if (choice == 6)
        {
            if (cart.isEmpty())
            {
                cout << "Cart is empty. Add food first.\n";
            }
            else
            {
                cout << "\n========== ORDER BILL ==========\n";
                cart.displayCart();

                cout << "Total Amount: Rs. "
                     << cart.calculateTotal() << endl;

                char confirm;

                cout << "\nConfirm order? (Y/N): ";
                cin >> confirm;

                if (confirm == 'Y' || confirm == 'y')
                {
                    string response = apiPlaceOrder(
                        cart,
                        name,
                        phone,
                        customerToken);

                    if (apiSuccess(response))
                    {
                        cout << "\nOrder placed successfully!\n";
                        cart.clearCart();
                        choice = 7;
                    }
                    else
                    {
                        cout << "\nUnable to place order: ";
                        showApiError(response);
                    }
                }
                else
                {
                    cout << "Checkout cancelled. Your cart is unchanged.\n";
                }
            }
        }

        else if (choice == 7)
        {
            cout << "Leaving order menu...\n";
        }

        else
        {
            cout << "Invalid choice.\n";
        }

    } while (choice != 7);
}

// ================= CUSTOMER MENU =================

void customerMenu(
    FoodList &menu,
    const string &name,
    const string &phone,
    int customerId,
    const string &customerToken)
{
    int choice;

    do
    {
        cout << "\n========== CUSTOMER MENU ==========\n";
        cout << "1. View Food Menu\n";
        cout << "2. Search Food\n";
        cout << "3. Place Order\n";
        cout << "4. View My Pending Orders\n";
        cout << "5. Cancel My Order\n";
        cout << "6. View My Order History\n";
        cout << "7. Logout\n";

        cout << "\nEnter your choice: ";
        cin >> choice;

        if (choice == 1)
        {
            loadFoodFromApi(menu);
            menu.displayMenu();
        }

        else if (choice == 2)
        {
            int id;

            cout << "Enter food ID: ";
            cin >> id;

            loadFoodFromApi(menu);
            menu.searchFood(id);
        }

        else if (choice == 3)
        {
            customerOrderMenu(
                menu,
                name,
                phone,
                customerId,
                customerToken);
        }

        else if (choice == 4)
        {
            string response =
                apiGetCustomerOrders(customerId, customerToken);

            if (apiSuccess(response))
                apiDisplayOrders(response, "pending");
            else
                showApiError(response);
        }

        else if (choice == 5)
        {
            int orderId;

            cout << "Enter order ID to cancel: ";
            cin >> orderId;

            string response =
                apiCancelCustomerOrder(customerId, orderId, customerToken);

            if (apiSuccess(response))
                cout << "Order cancelled successfully.\n";
            else
                showApiError(response);
        }

        else if (choice == 6)
        {
            string response =
                apiGetCustomerOrders(customerId, customerToken);

            if (apiSuccess(response))
                apiDisplayOrders(response, "history");
            else
                showApiError(response);
        }

        else if (choice == 7)
        {
            cout << "Logging out...\n";
        }

        else
        {
            cout << "Invalid choice.\n";
        }

    } while (choice != 7);
}

// ================= MAIN FUNCTION =================

int main()
{
    FoodList menu;

    int mainChoice;

    cout << "\nConnecting to Canteen Backend...\n";

    if (loadFoodFromApi(menu))
        cout << "Food menu loaded successfully from MongoDB.\n";
    else
        cout << "Backend unavailable. Start the backend before using the system.\n";

    do
    {
        cout << "\n========== CANTEEN MANAGEMENT SYSTEM ==========\n";
        cout << "1. Admin Login\n";
        cout << "2. Customer\n";
        cout << "3. Exit\n";

        cout << "\nEnter your choice: ";
        cin >> mainChoice;

        // ================= ADMIN LOGIN =================

        if (mainChoice == 1)
        {
            string username, password;
            string adminToken;

            cout << "\n========== ADMIN LOGIN ==========\n";

            cout << "Enter username: ";
            cin >> username;

            cout << "Enter password: ";
            cin >> password;

            if (apiAdminLogin(username, password, adminToken))
            {
                cout << "\nLogin successful!\n";
                cout << "Welcome, Admin!\n";

                adminMenu(menu, adminToken);
            }
            else
            {
                cout << "\nInvalid username or password, or backend unavailable.\n";
                cout << "Access denied.\n";
            }
        }

        // ================= CUSTOMER LOGIN / REGISTER =================

        else if (mainChoice == 2)
        {
            int customerOption;

            do
            {
                cout << "\n========== CUSTOMER ==========\n";
                cout << "1. Register\n";
                cout << "2. Login\n";
                cout << "3. Back to Main Menu\n";

                cout << "\nEnter your choice: ";
                cin >> customerOption;

                if (customerOption == 1 || customerOption == 2)
                {
                    string name, phone;

                    cout << "Enter customer name: ";
                    readLine(name);

                    cout << "Enter phone number: ";
                    cin >> phone;

                    // Backend creates the customer if not already registered.
                    string response = apiCustomerLogin(name, phone);

                    if (apiSuccess(response))
                    {
                        int customerId = apiGetCustomerId(response);
                        string customerToken =
                            apiGetCustomerToken(response);

                        if (customerId <= 0 || customerToken.empty())
                        {
                            cout << "Invalid customer response from backend.\n";
                        }
                        else
                        {
                            if (customerOption == 1)
                                cout << "\nRegistration successful!\n";
                            else
                                cout << "\nLogin successful!\n";

                            cout << "Welcome, " << name << "!\n";
                            cout << "Customer ID: " << customerId << endl;

                            customerMenu(
                                menu,
                                name,
                                phone,
                                customerId,
                                customerToken);
                        }
                    }
                    else
                    {
                        cout << "\n";
                        showApiError(response);
                    }
                }

                else if (customerOption == 3)
                {
                    cout << "Returning to main menu...\n";
                }

                else
                {
                    cout << "Invalid choice.\n";
                }

            } while (customerOption != 3);
        }

        else if (mainChoice == 3)
        {
            cout << "\nThank you for using the Canteen Management System!\n";
        }

        else
        {
            cout << "Invalid choice.\n";
        }

    } while (mainChoice != 3);

    return 0;
}
