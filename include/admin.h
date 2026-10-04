#ifndef ADMIN_H
#define ADMIN_H

#include <string>

using namespace std;

class Admin
{
private:
    string username;
    string password;

public:
    Admin(string username, string password);

    bool login(string enteredUsername, string enteredPassword);
};

#endif