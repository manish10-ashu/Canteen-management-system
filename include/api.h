
#ifndef API_H
#define API_H

#include "food.h"
#include "cart.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>
#include <regex>
#include <sstream>

using namespace std;

// ================= JSON HELPERS =================

string jsonEscape(const string& value)
{
    string result;

    for (char c : value)
    {
        if (c == '\\')
            result += "\\\\";
        else if (c == '"')
            result += "\\\"";
        else if (c == '\n')
            result += "\\n";
        else if (c == '\r')
            result += "\\r";
        else if (c == '\t')
            result += "\\t";
        else
            result += c;
    }

    return result;
}

// ================= HTTP REQUEST =================
string apiRequest(
    const string& method,
    const string& endpoint,
    const string& body = "",
    const string& token = ""
)
{
    if (!body.empty())
    {
        ofstream file("canteen_request.json");

        if (!file)
            return "";

        file << body;
        file.close();
    }

    string command = "curl.exe -s -X " + method;

    command += " -H \"Content-Type: application/json\"";

    if (!token.empty())
        command += " -H \"Authorization: Bearer " + token + "\"";

    if (!body.empty())
        command += " --data-binary @canteen_request.json";

    command += " http://localhost:5000" + endpoint;

    command += " > canteen_response.json 2>&1";

int result = system(command.c_str());

ifstream responseFile("canteen_response.json");

if (!responseFile)
    return "";

string response;
string line;

while (getline(responseFile, line))
    response += line;

responseFile.close();

remove("canteen_response.json");
if (result != 0 && response.empty())
    return "";

remove("canteen_request.json");

return response;
}

string apiGet(const string& endpoint)
{
    return apiRequest("GET", endpoint);
}

// ================= RESPONSE HELPERS =================

bool apiSuccess(const string& response)
{
    regex pattern(R"("success"\s*:\s*true)");

    return regex_search(response, pattern);
}

string apiMessage(const string& response)
{
    regex pattern(R"REGEX("message"\s*:\s*"((?:\\.|[^"\\])*)")REGEX");

    smatch match;

    if (regex_search(response, match, pattern))
        return match[1].str();

    return "Backend request failed.";
}

string getStringField(
    const string& object,
    const string& field
)
{
    regex pattern(
        "\"" + field +
        R"REGEX("\s*:\s*"((?:\\.|[^"\\])*)")REGEX"
    );

    smatch match;

    if (regex_search(object, match, pattern))
        return match[1].str();

    return "";
}

string getNumberField(
    const string& object,
    const string& field
)
{
    regex pattern(
        "\"" + field +
        R"("\s*:\s*(-?[0-9]+(?:\.[0-9]+)?))"
    );

    smatch match;

    if (regex_search(object, match, pattern))
        return match[1].str();

    return "0";
}

int apiReadIntField(
    const string& object,
    const string& field
)
{
    return stoi(getNumberField(object, field));
}

bool getBoolField(
    const string& object,
    const string& field
)
{
    regex pattern(
        "\"" + field +
        R"("\s*:\s*(true|false))"
    );

    smatch match;

    if (regex_search(object, match, pattern))
        return match[1] == "true";

    return false;
}

// Extract a nested JSON object.
string apiGetObject(
    const string& response,
    const string& key
)
{
    size_t pos = response.find("\"" + key + "\"");

    if (pos == string::npos)
        return "";

    pos = response.find('{', pos);

    if (pos == string::npos)
        return "";

    int depth = 0;
    bool inString = false;
    bool escaped = false;

    for (size_t i = pos; i < response.length(); i++)
    {
        char c = response[i];

        if (inString)
        {
            if (escaped)
                escaped = false;
            else if (c == '\\')
                escaped = true;
            else if (c == '"')
                inString = false;

            continue;
        }

        if (c == '"')
            inString = true;
        else if (c == '{')
            depth++;
        else if (c == '}')
        {
            depth--;

            if (depth == 0)
                return response.substr(pos, i - pos + 1);
        }
    }

    return "";
}

// Extract a JSON array.
string apiReadArray(
    const string& response,
    const string& key
)
{
    size_t pos = response.find("\"" + key + "\"");

    if (pos == string::npos)
        return "";

    pos = response.find('[', pos);

    if (pos == string::npos)
        return "";

    int depth = 0;
    bool inString = false;
    bool escaped = false;

    for (size_t i = pos; i < response.length(); i++)
    {
        char c = response[i];

        if (inString)
        {
            if (escaped)
                escaped = false;
            else if (c == '\\')
                escaped = true;
            else if (c == '"')
                inString = false;

            continue;
        }

        if (c == '"')
            inString = true;
        else if (c == '[')
            depth++;
        else if (c == ']')
        {
            depth--;

            if (depth == 0)
                return response.substr(pos + 1, i - pos - 1);
        }
    }

    return "";
}

// ================= ADMIN LOGIN =================

bool apiAdminLogin(
    const string& username,
    const string& password,
    string& token
)
{
    string body =
        "{\"username\":\"" + jsonEscape(username) +
        "\",\"password\":\"" + jsonEscape(password) + "\"}";

    string response = apiRequest(
        "POST",
        "/api/admin/login",
        body
    );

    if (!apiSuccess(response))
        return false;

    token = getStringField(response, "token");

    return !token.empty();
}

// ================= FOOD OPERATIONS =================

// Load food from MongoDB into the linked list.
bool loadFoodFromApi(FoodList& menu)
{
    string response = apiGet("/api/food");

    if (response.empty())
        return false;

    regex objectPattern(R"(\{[^{}]*\})");

    auto begin = sregex_iterator(
        response.begin(),
        response.end(),
        objectPattern
    );

    auto end = sregex_iterator();

    bool loaded = false;

    for (auto it = begin; it != end; ++it)
    {
        string object = it->str();

        int id = apiReadIntField(object, "id");

        string name = getStringField(object, "name");
        string category = getStringField(object, "category");

        float price = stof(getNumberField(object, "price"));

        bool available = getBoolField(object, "available");

        string emoji = getStringField(object, "emoji");
        string description = getStringField(object, "description");
        string color = getStringField(object, "color");

        if (id <= 0 || name.empty())
            continue;

        if (menu.findFood(id))
        {
            menu.updateFood(
                id, name, category, price,
                available, emoji, description, color
            );
        }
        else
        {
            menu.addFood(
                id, name, category, price,
                available, emoji, description, color
            );
        }

        loaded = true;
    }

    return loaded;
}

// Add food.
bool apiAddFood(
    int id,
    string name,
    string category,
    float price,
    const string& token
)
{
    ostringstream priceText;
    priceText << price;

    string body =
        "{\"id\":" + to_string(id) +
        ",\"name\":\"" + jsonEscape(name) +
        "\",\"category\":\"" + jsonEscape(category) +
        "\",\"price\":" + priceText.str() +
        ",\"available\":true}";

    string response = apiRequest(
        "POST", "/api/food", body, token
    );

    if (!apiSuccess(response))
        cout << apiMessage(response) << endl;

    return apiSuccess(response);
}

// Update food.
bool apiUpdateFood(
    int id,
    string name,
    string category,
    float price,
    const string& token
)
{
    ostringstream priceText;
    priceText << price;

    string body =
        "{\"name\":\"" + jsonEscape(name) +
        "\",\"category\":\"" + jsonEscape(category) +
        "\",\"price\":" + priceText.str() + "}";

    string response = apiRequest(
        "PUT",
        "/api/food/" + to_string(id),
        body,
        token
    );

    if (!apiSuccess(response))
        cout << apiMessage(response) << endl;

    return apiSuccess(response);
}

// Delete food.
bool apiDeleteFood(
    int id,
    const string& token
)
{
    string response = apiRequest(
        "DELETE",
        "/api/food/" + to_string(id),
        "",
        token
    );

    if (!apiSuccess(response))
        cout << apiMessage(response) << endl;

    return apiSuccess(response);
}

// ================= CUSTOMER OPERATIONS =================

// Register or login customer.
// Backend automatically creates a new customer if needed.
string apiCustomerLogin(
    const string& name,
    const string& phone
)
{
    string body =
        "{\"name\":\"" + jsonEscape(name) +
        "\",\"phone\":\"" + jsonEscape(phone) + "\"}";

    return apiRequest(
        "POST",
        "/api/customers/login",
        body
    );
}

int apiGetCustomerId(const string& response)
{
    string customer = apiGetObject(response, "customer");

    return apiReadIntField(customer, "id");
}

string apiGetCustomerToken(const string& response)
{
    return getStringField(response, "token");
}

// Admin: get all customers.
string apiGetCustomers(const string& token)
{
    return apiRequest(
        "GET",
        "/api/customers",
        "",
        token
    );
}

// ================= ORDER OPERATIONS =================

// Place an order using the existing Cart linked list.
string apiPlaceOrder(
    Cart& cart,
    const string& name,
    const string& phone,
    const string& token
)
{
    string items = "[";

    CartItem* temp = cart.getHead();

    bool first = true;

    while (temp != nullptr)
    {
        if (!first)
            items += ",";

        first = false;

        items +=
            "{\"id\":" + to_string(temp->foodId) +
            ",\"quantity\":" + to_string(temp->quantity) + "}";

        temp = temp->next;
    }

    items += "]";

    string body =
        "{\"customerName\":\"" + jsonEscape(name) +
        "\",\"phone\":\"" + jsonEscape(phone) +
        "\",\"items\":" + items + "}";

    return apiRequest(
        "POST",
        "/api/orders",
        body,
        token
    );
}

// Get all orders for admin.
string apiGetAdminOrders(const string& token)
{
    return apiRequest(
        "GET",
        "/api/orders",
        "",
        token
    );
}

// Get customer orders.
string apiGetCustomerOrders(
    int customerId,
    const string& token
)
{
    return apiRequest(
        "GET",
        "/api/customers/" + to_string(customerId) + "/orders",
        "",
        token
    );
}

// Change order status.
string apiUpdateOrderStatus(
    int orderId,
    const string& status,
    const string& token
)
{
    string body =
        "{\"status\":\"" + jsonEscape(status) + "\"}";

    return apiRequest(
        "PATCH",
        "/api/orders/" + to_string(orderId) + "/status",
        body,
        token
    );
}

// Admin cancels an order.
string apiCancelAdminOrder(
    int orderId,
    const string& token
)
{
    return apiRequest(
        "DELETE",
        "/api/orders/" + to_string(orderId),
        "",
        token
    );
}

// Customer cancels an order.
// Requires the matching backend route to be added.
string apiCancelCustomerOrder(
    int customerId,
    int orderId,
    const string& token
)
{
    return apiRequest(
        "DELETE",
        "/api/customers/" + to_string(customerId) +
        "/orders/" + to_string(orderId),
        "",
        token
    );
}

// ================= DISPLAY DATABASE RECORDS =================

void apiDisplayCustomers(const string& response)
{
    string data = apiReadArray(response, "customers");

    if (data.empty())
    {
        cout << "\nNo customers found." << endl;
        return;
    }

    regex objectPattern(R"(\{[^{}]*\})");

    auto begin = sregex_iterator(
        data.begin(), data.end(), objectPattern
    );

    auto end = sregex_iterator();

    cout << "\n========== REGISTERED CUSTOMERS ==========" << endl;

    for (auto it = begin; it != end; ++it)
    {
        string object = it->str();

        cout << "\nCustomer ID: "
             << apiReadIntField(object, "id") << endl;

        cout << "Name: "
             << getStringField(object, "name") << endl;

        cout << "Phone: "
             << getStringField(object, "phone") << endl;

        cout << "-----------------------------------" << endl;
    }
}

void apiDisplayOrders(const string& response, const string& arrayName)
{
    string data = apiReadArray(response, arrayName);

    if (data.empty())
    {
        cout << "\nNo orders found." << endl;
        return;
    }

    cout << "\n========== ORDERS ==========" << endl;

    int depth = 0;
    bool inString = false;
    bool escaped = false;
    size_t start = 0;

    for (size_t i = 0; i < data.length(); i++)
    {
        char c = data[i];

        if (inString)
        {
            if (escaped)
                escaped = false;
            else if (c == '\\')
                escaped = true;
            else if (c == '"')
                inString = false;

            continue;
        }

        if (c == '"')
            inString = true;

        else if (c == '{')
        {
            if (depth == 0)
                start = i;

            depth++;
        }
        else if (c == '}')
        {
            depth--;

            if (depth == 0)
            {
                string object = data.substr(start, i - start + 1);

                cout << "\nOrder ID: "
                     << apiReadIntField(object, "id") << endl;

                cout << "Customer ID: "
                     << apiReadIntField(object, "customerId") << endl;

                cout << "Customer: "
                     << getStringField(object, "customerName") << endl;

                cout << "Status: "
                     << getStringField(object, "status") << endl;

                cout << "Total: Rs. "
                     << getNumberField(object, "total") << endl;

                cout << "-----------------------------------" << endl;
            }
        }
    }
}
#endif
