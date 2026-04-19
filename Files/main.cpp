#include <bits/stdc++.h>

using namespace std;

const int MAX_NODES = 20;
const int INF = numeric_limits<int>::max();

class User {
    private:
        string currentUserName;

    public:
        User () {
            currentUserName = "admin";
        }

        bool userLogin (string userName, string userPassword) {
            if (userName == "admin" && userPassword == "123") return true;

            vector <string> userNames; 
            vector <string> userPasswords;

            ifstream names (".\\UserFiles\\userNames.txt");
            ifstream passwords (".\\UserFiles\\userPasswords.txt");

            string name, password;

            for (int i = 0; getline (names, name); ++i) {
                userNames.push_back (name);
            }

            while (getline (passwords, password)) {
                userPasswords.push_back (password);
            }

            for (int i = 0; i < userNames.size (); ++i) {
                if (userNames [i] == userName && userPasswords [i] == userPassword) {
                    currentUserName = userName;
                    return true;
                }
            }

            currentUserName = "Unidentified";

            names.close ();
            passwords.close ();

            return false;
        }

        bool userRegister (string userName, string userPassword) {
            ofstream userNames (".\\UserFiles\\userNames.txt", ios::app);
            ofstream userPasswords (".\\UserFiles\\userPasswords.txt", ios::app);

            userNames << userName << endl;
            userPasswords << userPassword << endl;

            currentUserName = userName;

            userNames.close ();
            userPasswords.close ();

            return true;
        }

        string getUserName () {return currentUserName;}
};


struct CampusBlock {
    int id;
    string name;
    int parkingCapacity;
    int occupiedSlots;
};


class CampusNavigator {
private:
    vector<pair<int,int>> adjList[MAX_NODES];
    map<int, CampusBlock> blocks;
    int numBlocks = 0;

public:
    CampusNavigator() {
        for (int i = 0; i < MAX_NODES; i++) {
            adjList[i].clear();
        }
    }

    void loadCampusData(string cityFile, string adjFile) {
        ifstream cFile(cityFile);
        int id, cap;
        string name;
        while (cFile >> id >> name >> cap) {
            blocks[id] = {id, name, cap, 0};
            numBlocks++;
        }

        ifstream aFile(adjFile);
        string line;
        int row = 0;
        while (getline(aFile, line)) {
            stringstream ss(line);
            string val;
            int col = 0;
            while (getline(ss, val, ',')) {
                int weight = stoi (val);

                while (weight != 0) {
                    adjList [row].push_back ({col, weight});
                }

                col++;
            }
            row++;
        }
    }

    int getBlockId(string name) {
        for (auto const& [id, block] : blocks) {
            if (block.name == name) return id;
        }
        return -1;
    }

    void findCampusRoute(string start, string end) {
        int src = getBlockId(start);
        int dest = getBlockId(end);

        if (src == -1 || dest == -1) {
            cout << "Block name not recognized.\n";
            return;
        }

        vector<int> dist(MAX_NODES, INF);
        vector<int> parent(MAX_NODES, -1);
        vector<bool> visited(MAX_NODES, false);

        // 🔥 Min Heap (distance, node)
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;

        dist[src] = 0;
        pq.push({0, src});

        while (!pq.empty()) {
            auto [currDist, u] = pq.top();
            pq.pop();

            if (visited[u]) continue;
            visited[u] = true;

            // 🚀 Traverse adjacency list
            for (auto &[v, weight] : adjList[u]) {
                if (currDist + weight < dist[v]) {
                    dist[v] = currDist + weight;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }

        if (dist[dest] == INF) {
            cout << "No walking/driving path found between these blocks.\n";
        } else {
            cout << "\n[Optimal Campus Route]: ";
            printPath(parent, dest);
            cout << "\nEstimated Time Delay: " << dist[dest] << " units\n";
            checkParking(dest);
        }
    }

    void printPath(vector<int>& parent, int j) {
        if (parent[j] == -1) {
            cout << blocks[j].name;
            return;
        }
        printPath(parent, parent[j]);
        cout << " -> " << blocks[j].name;
    }

    void checkParking(int blockId) {
        CampusBlock &b = blocks[blockId];
        cout << "Parking at " << b.name << ": ";
        if (b.occupiedSlots < b.parkingCapacity) {
            cout << (b.parkingCapacity - b.occupiedSlots) << " slots available.\n";
        } else {
            cout << "FULL. Please use the nearest overflow lot.\n";
        }
    }

    void updateOccupancy(string name, int change) {
        int id = getBlockId(name);
        if (id != -1) {
            blocks[id].occupiedSlots = max(0, min(blocks[id].parkingCapacity, blocks[id].occupiedSlots + change));
            cout << "Updated " << name << " occupancy.\n";
        }
    }

    bool initiateParking (int slotID, string userName, int fromHour, int fromMin, int toHour, int toMin) {
        if (slotID < 0) return false;

        CampusBlock& tempBlock = blocks [slotID];

        if (tempBlock.occupiedSlots < tempBlock.parkingCapacity) {
            tempBlock.occupiedSlots++;
            return true;
        } else {
            return false;
        }
    }
};

User user;
CampusNavigator uniMap;
bool isAdmin;

void startUp () {
    cout << "1. Login\n2. Register\n: ";
    int choice;
    cin >> choice;

    if (choice == 1) {
        cout << "Please enter your Username and Password (uN pW): ";
        string userName, userPassword;
        cin >> userName >> userPassword;

        if (!user.userLogin (userName, userPassword)) {
            cout << "Please enter correct details." << endl;
            exit (0);
        }

        cout << "Welcome! " << userName << endl;
    } else {
        cout << "Please enter your Username and Password (uN pW): ";
        string userName, userPassword;
        cin >> userName >> userPassword;

        if (!user.userRegister (userName, userPassword)) {
            cout << "Please enter correct details." << endl;
            exit (0);
        }

        cout << "Welcome! " << userName << endl;
    }
}

void loadMaps () {
    uniMap.loadCampusData("nodes_list.txt", "adjacency_list.txt");

    isAdmin = (user.getUserName() == "admin");
}

void initialApplicationStartUp () {
    int choice;

    while (true) {
        cout << "\n--- University Travel Assistant ---\n";
        cout << "1. Find Route & Check Parking\n";
        if (isAdmin) {
            cout << "2. Update Block Parking (Admin)\n";
            cout << "3. Modify Campus Graph Data (Admin)\n";
        }
        cout << "4. Exit\nChoice: ";
        cin >> choice;

        if (choice == 4) break;

        string b1, b2;
        if (choice == 1) {
            cout << "Current Location: "; cin.ignore(); getline (cin, b1);
            cout << "Destination Block: "; getline (cin, b2);
            cout << "Route from: " << b1 << endl << "To: " << b2 << endl;
            uniMap.findCampusRoute(b1, b2);

            cout << "Do you want to book? (1/0) ";
            cin >> choice;

            if (choice) {
                int blockID = uniMap.getBlockId (b2);
                int fromHour, fromMin, toHour, toMin;

                cout << "Please enter start time (Hour Min): ";
                cin >> fromHour >> fromMin;

                cout << "Please enter end time (Hour Min): ";
                cin >> toHour >> toMin;

                if (uniMap.initiateParking (blockID, user.getUserName(), fromHour, fromMin, toHour, toMin)) {
                    cout << "Parking is Booked!" << endl;
                } else {
                    cout << "An Error has occured (404)" << endl;
                }
            }
        } else if (choice == 2) {
            if (!isAdmin) {
                cout << "Access Denied. Admin privileges required.\n";
                continue;
            }
            int change;
            cout << "Block Name: "; cin >> b1;
            cout << "Change (e.g., 1 for entry, -1 for exit): "; cin >> change;
            uniMap.updateOccupancy(b1, change);
        } else if (choice == 3) {
            if (!isAdmin) {
                cout << "Access Denied. Admin privileges required.\n";
                continue;
            }
            
            int adminChoice;
            cout << "\n--- Graph Modification ---\n";
            cout << "1. Enter New Graph Data\n";
            cout << "2. Update Existing Graph\n";
            cout << "Choice: ";
            cin >> adminChoice;

            if (adminChoice == 1) {
                int n;
                cout << "Enter number of blocks: ";
                cin >> n;

                ofstream f1("nodes_list.txt");
                vector<string> names(n);

                for (int i = 0; i < n; i++) {
                    string bName;
                    int cap;
                    cout << "Name for block " << i << ": ";
                    cin >> bName;
                    cout << "Capacity: ";
                    cin >> cap;
                    
                    names[i] = bName;
                    f1 << i << " " << bName << " " << cap << endl;
                }
                f1.close();

                ofstream f2("adjacency_matrix.txt");
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        int w = 0;
                        if (i != j) {
                            cout << "Weight " << names[i] << " to " << names[j] << ": ";
                            cin >> w;
                        }
                        f2 << w << (j == n - 1 ? "" : ",");
                    }
                    f2 << endl;
                }
                f2.close();

                cout << "Done. Files saved. Reloading campus map...\n";
                uniMap = CampusNavigator(); 
                uniMap.loadCampusData("nodes_list.txt", "adjacency_matrix.txt");
            } else if (adminChoice == 2) {
                cout << "Feature 'Update Graph' coming soon...\n";
            } else {
                cout << "Invalid choice.\n";
            }
        } else {
            cout << "Invalid choice.\n";
        }
    }
}


int main() {
    
    startUp ();
    cout << "Startup Worked" << endl;
    loadMaps ();
    cout << "Loading Maps Worked" << endl;
    initialApplicationStartUp ();
    
    return 0;
}