#include "../include/Admin.h"

using namespace std;

Admin::Admin(string username, string password)
{
    this->username = username;
    this->password = password;
}

bool Admin::login(string enteredUsername, string enteredPassword)
{
    if (enteredUsername == username &&
        enteredPassword == password)
    {
        return true;
    }

    return false;
}