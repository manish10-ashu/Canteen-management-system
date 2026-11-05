// ---------------------------------------------------------------------------
//  Canteen core - command line bridge used by the Node/Express backend.
//
//  Usage:  canteen-core <command>
//
//  Input  : simple "key=value" lines on stdin. The whole dataset is streamed
//           in this way ("food=", "customer=", "order=", "orderitem=" lines,
//           plus the command parameters) so the stateless core can rebuild its
//           linked lists / queue before running the command.
//  Output : a single JSON object on stdout: { "result": <command output>,
//           "state": { food, customers, orders } }. The backend persists the
//           "state" so nothing is lost between the one-process-per-request runs.
//
//  All the real work (linked lists, FIFO order queue) is done by the existing
//  classes - this file only adapts them to a stateless, per-request process.
// ---------------------------------------------------------------------------

#include "../include/food.h"
#include "../include/order.h"
#include "../include/customer.h"
#include "../include/admin.h"
#include "../include/cart.h"
#include "../include/util.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>

using namespace std;

typedef vector<pair<string, string> > Params;

// ==================== INPUT HELPERS ====================

static Params readParams()
{
    Params params;
    string line;

    while (getline(cin, line))
    {
        if (line.empty())
        {
            continue;
        }

        size_t pos = line.find('=');

        if (pos == string::npos)
        {
            continue;
        }

        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);

        if (!value.empty() && value[value.size() - 1] == '\r')
        {
            value.erase(value.size() - 1);
        }

        params.push_back(make_pair(key, value));
    }

    return params;
}

static string getParam(const Params& params, const string& key,
                       const string& fallback = "")
{
    for (size_t i = 0; i < params.size(); i++)
    {
        if (params[i].first == key)
        {
            return params[i].second;
        }
    }

    return fallback;
}

static vector<string> getParamList(const Params& params, const string& key)
{
    vector<string> values;

    for (size_t i = 0; i < params.size(); i++)
    {
        if (params[i].first == key)
        {
            values.push_back(params[i].second);
        }
    }

    return values;
}

static bool hasParam(const Params& params, const string& key)
{
    for (size_t i = 0; i < params.size(); i++)
    {
        if (params[i].first == key)
        {
            return true;
        }
    }

    return false;
}

// ==================== JSON HELPERS ====================

static string escapeJson(const string& value)
{
    string out;

    for (size_t i = 0; i < value.size(); i++)
    {
        char c = value[i];

        if (c == '"')
        {
            out += "\\\"";
        }
        else if (c == '\\')
        {
            out += "\\\\";
        }
        else if (c == '\n')
        {
            out += "\\n";
        }
        else if (c == '\r')
        {
            out += "\\r";
        }
        else if (c == '\t')
        {
            out += "\\t";
        }
        else
        {
            out += c;
        }
    }

    return out;
}

static string jsonString(const string& value)
{
    return "\"" + escapeJson(value) + "\"";
}

static string numberToString(float value)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%g", value);
    return string(buffer);
}

static string foodToJson(Food* food)
{
    string json = "{";

    json += "\"id\":" + to_string(food->id) + ",";
    json += "\"name\":" + jsonString(food->name) + ",";
    json += "\"category\":" + jsonString(food->category) + ",";
    json += "\"price\":" + numberToString(food->price) + ",";
    json += "\"available\":";
    json += (food->available ? "true" : "false");
    json += ",";
    json += "\"emoji\":" + jsonString(food->emoji) + ",";
    json += "\"description\":" + jsonString(food->description) + ",";
    json += "\"color\":" + jsonString(food->color);

    json += "}";

    return json;
}

static string orderToJson(Order* order)
{
    string json = "{";

    json += "\"id\":" + to_string(order->orderId) + ",";
    json += "\"customerId\":" + to_string(order->customerId) + ",";
    json += "\"customerName\":" + jsonString(order->customerName) + ",";
    json += "\"total\":" + numberToString(order->totalAmount) + ",";
    json += "\"status\":" + jsonString(order->status) + ",";
    json += "\"items\":[";

    OrderItem* item = order->items;
    bool first = true;

    while (item != nullptr)
    {
        if (!first)
        {
            json += ",";
        }

        first = false;

        json += "{\"id\":" + to_string(item->foodId)
              + ",\"name\":" + jsonString(item->foodName)
              + ",\"price\":" + numberToString(item->price)
              + ",\"quantity\":" + to_string(item->quantity) + "}";

        item = item->next;
    }

    json += "]}";

    return json;
}

static string okJson(const string& message)
{
    return "{\"success\":true,\"message\":" + jsonString(message) + "}";
}

static string errorJson(const string& message)
{
    return "{\"success\":false,\"message\":" + jsonString(message) + "}";
}

// ==================== CORE STATE ====================
// The core holds no data of its own. Every request receives the full dataset
// on stdin and returns the updated dataset on stdout.

struct CoreState
{
    FoodList menu;
    OrderQueue orders;
    CustomerList customers;
};

static CoreState gState;

// ==================== COMMAND HANDLERS ====================

static Food* findFoodById(FoodList& menu, int id)
{
    return menu.findFood(id);
}

static Order* findOrderBySlot(Order* head, int orderId)
{
    while (head != nullptr)
    {
        if (head->orderId == orderId)
        {
            return head;
        }

        head = head->next;
    }

    return nullptr;
}

static Order* findOrder(OrderQueue& orders, int orderId)
{
    Order* found = findOrderBySlot(orders.getPendingHead(), orderId);

    if (found != nullptr)
    {
        return found;
    }

    return findOrderBySlot(orders.getHistoryHead(), orderId);
}

// ==================== STATE LOADING ====================

// Each of these rebuilds one linked list / queue from the "|"-separated rows
// the backend streams on stdin (originally loaded from MongoDB).

static void loadFoodState(const Params& params)
{
    vector<string> rows = getParamList(params, "food");

    for (size_t i = 0; i < rows.size(); i++)
    {
        vector<string> parts = splitLine(rows[i], '|');

        if (parts.size() < 5)
        {
            continue;
        }

        gState.menu.addFood(
            toInt(parts[0]),
            parts[1],
            parts[2],
            toFloat(parts[3]),
            toInt(parts[4], 1) != 0,
            parts.size() > 5 ? parts[5] : "",
            parts.size() > 6 ? parts[6] : "",
            parts.size() > 7 ? parts[7] : ""
        );
    }
}

static void loadCustomerState(const Params& params)
{
    vector<string> rows = getParamList(params, "customer");

    for (size_t i = 0; i < rows.size(); i++)
    {
        vector<string> parts = splitLine(rows[i], '|');

        if (parts.size() < 3)
        {
            continue;
        }

        gState.customers.addCustomer(toInt(parts[0]), parts[1], parts[2]);
    }
}

static void loadOrderState(const Params& params)
{
    vector<string> headers = getParamList(params, "order");

    for (size_t i = 0; i < headers.size(); i++)
    {
        vector<string> parts = splitLine(headers[i], '|');

        if (parts.size() < 4)
        {
            continue;
        }

        gState.orders.addOrderFromState(
            toInt(parts[0]), toInt(parts[1]), parts[2], parts[3]);
    }

    vector<string> items = getParamList(params, "orderitem");

    for (size_t i = 0; i < items.size(); i++)
    {
        vector<string> parts = splitLine(items[i], '|');

        if (parts.size() < 5)
        {
            continue;
        }

        Order* order = findOrder(gState.orders, toInt(parts[0]));

        if (order == nullptr)
        {
            continue;
        }

        order->addItem(
            toInt(parts[1]), parts[2], toFloat(parts[3]), toInt(parts[4]));
    }

    // Recompute the totals from the loaded items so the core stays the single
    // source of truth for an order's amount.
    Order* order = gState.orders.getPendingHead();

    while (order != nullptr)
    {
        order->calculateTotal();
        order = order->next;
    }

    order = gState.orders.getHistoryHead();

    while (order != nullptr)
    {
        order->calculateTotal();
        order = order->next;
    }
}

static void loadState(const Params& params)
{
    loadFoodState(params);
    loadCustomerState(params);
    loadOrderState(params);
}

static string handleCommand(const string& command, const Params& params)
{
    FoodList& menu = gState.menu;
    OrderQueue& orders = gState.orders;
    CustomerList& customers = gState.customers;
    Admin admin("admin", "1234");

    // ---------------- FOOD ----------------

    if (command == "list-food")
    {
        string json = "[";
        Food* food = menu.getHead();
        bool first = true;

        while (food != nullptr)
        {
            if (!first)
            {
                json += ",";
            }

            first = false;
            json += foodToJson(food);

            food = food->next;
        }

        json += "]";

        return json;
    }

    if (command == "add-food")
    {
        int id = toInt(getParam(params, "id"));

        if (menu.findFood(id) != nullptr)
        {
            return errorJson("A food item with this ID already exists.");
        }

        menu.addFood(
            id,
            getParam(params, "name"),
            getParam(params, "category"),
            toFloat(getParam(params, "price")),
            toInt(getParam(params, "available"), 1) != 0,
            getParam(params, "emoji"),
            getParam(params, "description"),
            getParam(params, "color")
        );

        return "{\"success\":true,\"message\":\"Food item added.\","
               "\"food\":" + foodToJson(findFoodById(menu, id)) + "}";
    }

    if (command == "update-food")
    {
        int id = toInt(getParam(params, "id"));

        Food* existing = menu.findFood(id);

        if (existing == nullptr)
        {
            return errorJson("Food item not found.");
        }

        // Only overwrite the fields that were actually supplied.
        string name = hasParam(params, "name") ?
                      getParam(params, "name") : existing->name;
        string category = hasParam(params, "category") ?
                          getParam(params, "category") : existing->category;
        float price = hasParam(params, "price") ?
                      toFloat(getParam(params, "price")) : existing->price;
        bool available = hasParam(params, "available") ?
                         (toInt(getParam(params, "available"), 1) != 0) :
                         existing->available;
        string emoji = hasParam(params, "emoji") ?
                       getParam(params, "emoji") : existing->emoji;
        string description = hasParam(params, "description") ?
                             getParam(params, "description") : existing->description;
        string color = hasParam(params, "color") ?
                       getParam(params, "color") : existing->color;

        menu.updateFood(id, name, category, price, available,
                        emoji, description, color);

        return "{\"success\":true,\"message\":\"Food item updated.\","
               "\"food\":" + foodToJson(findFoodById(menu, id)) + "}";
    }

    if (command == "delete-food")
    {
        int id = toInt(getParam(params, "id"));

        if (menu.findFood(id) == nullptr)
        {
            return errorJson("Food item not found.");
        }

        menu.deleteFood(id);

        return okJson("Food item removed.");
    }

    // ---------------- ORDERS ----------------

    if (command == "place-order")
    {
        int customerId = toInt(getParam(params, "customerId"));
        string customerName = getParam(params, "customerName");

        vector<string> items = getParamList(params, "item");

        if (items.empty())
        {
            return errorJson("Your cart is empty.");
        }

        Cart cart;

        for (size_t i = 0; i < items.size(); i++)
        {
            size_t sep = items[i].find(':');

            if (sep == string::npos)
            {
                continue;
            }

            int foodId = toInt(items[i].substr(0, sep));
            int quantity = toInt(items[i].substr(sep + 1));

            cart.addItem(menu, foodId, quantity);
        }

        if (cart.isEmpty())
        {
            return errorJson("No valid available items were found in the cart.");
        }

        int orderId = orders.getNextOrderId();

        // This is the same queue/linked-list logic the console app uses.
        orders.placeOrderConfirmed(orderId, cart, customerId, customerName);

        Order* created = findOrder(orders, orderId);

        return "{\"success\":true,\"message\":\"Order placed successfully.\","
               "\"order\":" + orderToJson(created) + "}";
    }

    if (command == "list-orders")
    {
        string json = "{\"success\":true,\"orders\":[";
        bool first = true;

        Order* order = orders.getPendingHead();

        for (int pass = 0; pass < 2; pass++)
        {
            while (order != nullptr)
            {
                if (!first)
                {
                    json += ",";
                }

                first = false;
                json += orderToJson(order);

                order = order->next;
            }

            order = orders.getHistoryHead();
        }

        json += "]}";

        return json;
    }

    if (command == "update-order-status")
    {
        int orderId = toInt(getParam(params, "orderId"));
        string status = getParam(params, "status");

        if (status != "Pending" && status != "Preparing" &&
            status != "Delivered")
        {
            return errorJson("Invalid order status.");
        }

        if (findOrder(orders, orderId) == nullptr)
        {
            return errorJson("Order not found.");
        }

        orders.updateOrderStatus(orderId, status);

        return "{\"success\":true,\"message\":\"Order status updated.\","
               "\"order\":" + orderToJson(findOrder(orders, orderId)) + "}";
    }

    if (command == "cancel-order")
    {
        int orderId = toInt(getParam(params, "orderId"));

        if (findOrderBySlot(orders.getPendingHead(), orderId) == nullptr)
        {
            return errorJson("Order not found in pending orders.");
        }

        orders.cancelOrder(orderId);

        return okJson("Order cancelled.");
    }

    // ---------------- CUSTOMERS ----------------

    if (command == "customer-login")
    {
        string name = getParam(params, "name");
        string phone = getParam(params, "phone");

        if (name.empty() || phone.empty())
        {
            return errorJson("Name and phone number are required.");
        }

        Customer* customer = customers.loginCustomer(name, phone);

        if (customer == nullptr)
        {
            // New customer - register them with the next available ID.
            int id = customers.getNextCustomerId();
            customers.addCustomer(id, name, phone);
            customer = customers.loginCustomer(name, phone);
        }

        string json = "{\"success\":true,\"customer\":{";
        json += "\"id\":" + to_string(customer->customerId) + ",";
        json += "\"name\":" + jsonString(customer->name) + ",";
        json += "\"phone\":" + jsonString(customer->phone);
        json += "}}";

        return json;
    }

    if (command == "customer-orders")
    {
        int customerId = toInt(getParam(params, "customerId"));

        string json = "{\"success\":true,\"pending\":[";
        bool first = true;

        Order* order = orders.getPendingHead();

        while (order != nullptr)
        {
            if (order->customerId == customerId)
            {
                if (!first)
                {
                    json += ",";
                }

                first = false;
                json += orderToJson(order);
            }

            order = order->next;
        }

        json += "],\"history\":[";
        first = true;
        order = orders.getHistoryHead();

        while (order != nullptr)
        {
            if (order->customerId == customerId)
            {
                if (!first)
                {
                    json += ",";
                }

                first = false;
                json += orderToJson(order);
            }

            order = order->next;
        }

        json += "]}";

        return json;
    }

    // ---------------- ADMIN ----------------

    if (command == "admin-login")
    {
        bool valid = admin.login(
            getParam(params, "username"),
            getParam(params, "password")
        );

        if (!valid)
        {
            return errorJson("Invalid username or password.");
        }

        return okJson("Login successful.");
    }

    return errorJson("Unknown command.");
}

// ==================== STATE SERIALIZATION ====================

static string stateFoodJson()
{
    string json = "[";
    Food* food = gState.menu.getHead();
    bool first = true;

    while (food != nullptr)
    {
        if (!first)
        {
            json += ",";
        }

        first = false;
        json += foodToJson(food);
        food = food->next;
    }

    json += "]";

    return json;
}

static string stateCustomerJson()
{
    string json = "[";
    Customer* customer = gState.customers.getHead();
    bool first = true;

    while (customer != nullptr)
    {
        if (!first)
        {
            json += ",";
        }

        first = false;

        json += "{\"id\":" + to_string(customer->customerId)
              + ",\"name\":" + jsonString(customer->name)
              + ",\"phone\":" + jsonString(customer->phone) + "}";

        customer = customer->next;
    }

    json += "]";

    return json;
}

static string stateOrderJson()
{
    string json = "[";
    Order* order = gState.orders.getPendingHead();
    bool first = true;

    for (int pass = 0; pass < 2; pass++)
    {
        while (order != nullptr)
        {
            if (!first)
            {
                json += ",";
            }

            first = false;
            json += orderToJson(order);
            order = order->next;
        }

        order = gState.orders.getHistoryHead();
    }

    json += "]";

    return json;
}

static string buildStateJson()
{
    return "{\"food\":" + stateFoodJson()
         + ",\"customers\":" + stateCustomerJson()
         + ",\"orders\":" + stateOrderJson() + "}";
}

// ==================== MAIN ====================

int main(int argc, char** argv)
{
    string command = argc > 1 ? argv[1] : "";

    // Always read stdin: the backend streams the full dataset (plus the
    // command parameters) as "key=value" lines, then closes the pipe.
    Params params = readParams();

    // The core classes print friendly messages with cout. Send that chatter to
    // stderr so stdout always carries valid JSON and nothing else.
    streambuf* realCout = cout.rdbuf();
    cout.rdbuf(cerr.rdbuf());

    loadState(params);

    string result = handleCommand(command, params);
    string state = buildStateJson();

    cout.rdbuf(realCout);
    cout << "{\"result\":" << result
         << ",\"state\":" << state << "}" << endl;

    return 0;
}
