#include <bits/stdc++.h>
using namespace std;

class UserSystem {
private:
    string currentUser;
    string currentRole;

    const string FILE_NAME = ".\\UserFiles\\users.txt";

public:

    UserSystem() {
        currentUser = "guest";
        currentRole = "user";
    }

    // 🔐 LOGIN
    bool login(string username, string password) {

        ifstream file(FILE_NAME);

        if (!file.is_open()) return false;

        string u, p, r;

        while (file >> u >> p >> r) {
            if (u == username && p == password) {
                currentUser = u;
                currentRole = r;
                file.close();
                return true;
            }
        }

        file.close();
        currentUser = "guest";
        currentRole = "user";
        return false;
    }

    // 📝 REGISTER (ONLY USER, NOT ADMIN)
    bool registerUser(string username, string password) {

        // Check duplicate
        ifstream check(FILE_NAME);
        string u, p, r;

        while (check >> u >> p >> r) {
            if (u == username) {
                check.close();
                return false; // already exists
            }
        }
        check.close();

        ofstream file(FILE_NAME, ios::app);
        file << username << " " << password << " user" << endl;
        file.close();

        currentUser = username;
        currentRole = "user";

        return true;
    }

    string getUser() { return currentUser; }
    string getRole() { return currentRole; }

    bool isAdmin() { return currentRole == "admin"; }
};