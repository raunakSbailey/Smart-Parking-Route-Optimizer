#include "crow.h"
#include "MainCode.cpp"

using namespace std;

bool loginUser(
    const string& user,
    const string& pass,
    string& role
){
    ifstream file("./UserFiles/users.txt");

    if(!file.is_open()) return false;

    string u,p,r;

    while(file >> u >> p >> r){
        if(u == user && p == pass){
            role = r;
            file.close();
            return true;
        }
    }

    file.close();
    return false;
}

bool registerUser(
    const string& user,
    const string& pass
){
    ifstream check("./UserFiles/users.txt");

    string u,p,r;

    while(check >> u >> p >> r){
        if(u == user){
            check.close();
            return false;
        }
    }

    check.close();

    ofstream file("./UserFiles/users.txt", ios::app);
    file << user << " " << pass << " user\n";
    file.close();

    return true;
}

int main(){

    crow::SimpleApp app;

    CROW_ROUTE(app,"/")([]{ 

        ifstream file("index.html");

        string html(
            (istreambuf_iterator<char>(file)),
            istreambuf_iterator<char>()
        );

        return crow::response(html);
    });

    CROW_ROUTE(app,"/login")
    ([](const crow::request& req){

        crow::json::wvalue res;

        auto userParam = req.url_params.get("user");
        auto passParam = req.url_params.get("pass");

        string user = userParam ? string(userParam) : "";
        string pass = passParam ? string(passParam) : "";
        string role = "user";

        bool ok = loginUser(user, pass, role);

        res["success"] = ok;
        res["role"]    = role;

        return crow::response(res);
    });

    CROW_ROUTE(app,"/register")
    ([](const crow::request& req){

        crow::json::wvalue res;

        auto userParam = req.url_params.get("user");
        auto passParam = req.url_params.get("pass");

        string user = userParam ? string(userParam) : "";
        string pass = passParam ? string(passParam) : "";

        bool ok = registerUser(user, pass);

        res["success"] = ok;
        if(!ok) res["error"] = "User already exists";

        return crow::response(res);
    });

    CROW_ROUTE(app,"/nodes")
    ([]{

        crow::json::wvalue res;
        int i = 0;

        for(auto &p : sys.nodes){

            auto n = p.second;

            if(n.ghost) continue;

            res[i]["id"]       = n.id;
            res[i]["name"]     = n.name;
            res[i]["capacity"] = n.capacity;
            res[i]["x"]        = n.x;
            res[i]["y"]        = n.y;

            // Also send current available slots for hour 0 (or current hour)
            // so frontend can show live availability
            int curHour = 10; // default preview hour
            int available = sys.slots.count(n.id) && sys.slots[n.id].count(curHour)
                            ? sys.slots[n.id][curHour] : 0;

            res[i]["available"] = available;

            i++;
        }

        return crow::response(res);
    });

    CROW_ROUTE(app,"/route")
    ([](const crow::request& req){

        crow::json::wvalue res;

        auto s  = req.url_params.get("start");
        auto e  = req.url_params.get("end");
        auto sh = req.url_params.get("startH");
        auto eh = req.url_params.get("endH");

        int startH = sh ? stoi(sh) : 10;
        int endH   = eh ? stoi(eh) : 12;

        // ---- BUG FIX: validate hour range ----
        if(startH < 0 || endH > 24 || startH >= endH){
            res["error"] = "Invalid hours. startH must be < endH, both in [0,24]";
            return crow::response(res);
        }

        int src  = sys.getId(s ? s : "");
        int dest = sys.getId(e ? e : "");

        if(src == -1 || dest == -1){
            res["error"] = "Invalid node names";
            return crow::response(res);
        }

        // Run Dijkstra on REAL nodes only
        auto realPath = sys.dijkstra(src, dest, sys.road);

        if(realPath.empty()){
            res["error"] = "No route found between these locations";
            return crow::response(res);
        }

        // Expand to visual route (adds ghost nodes for smooth display)
        auto visualPath = sys.buildVisualRoute(realPath);

        int  parkingNode = -1;
        bool fallback    = false;

        // Check destination first
        if(sys.availableAtTime(dest, startH, endH)){
            parkingNode = dest;
        }
        else{
            fallback = true;

            // ---- BUG FIX: search realPath backwards, not visualPath ----
            parkingNode = sys.fallbackParking(realPath, startH, endH);
        }

        if(parkingNode == -1){
            res["error"] = "No parking available near your destination";
            return crow::response(res);
        }

        // Reserve the slot
        sys.reserveSlot(parkingNode, startH, endH);

        // Build visual path up to parking node (if fallback, trim path)
        vector<int> displayPath = visualPath;

        if(fallback && parkingNode != dest){
            // Rebuild visual path only up to parkingNode
            vector<int> trimmedReal;
            for(int id : realPath){
                trimmedReal.push_back(id);
                if(id == parkingNode) break;
            }
            displayPath = sys.buildVisualRoute(trimmedReal);
        }

        int i = 0;
        for(int id : displayPath){
            auto n = sys.nodes[id];
            res["path"][i]["x"]     = n.x;
            res["path"][i]["y"]     = n.y;
            res["path"][i]["ghost"] = n.ghost;
            i++;
        }

        // Transport segment: parking node -> original destination
        if(fallback){
            res["transport"][0]["x"] = sys.nodes[parkingNode].x;
            res["transport"][0]["y"] = sys.nodes[parkingNode].y;
            res["transport"][1]["x"] = sys.nodes[dest].x;
            res["transport"][1]["y"] = sys.nodes[dest].y;
        }

        res["fallback"]     = fallback;
        res["parkingNode"]  = sys.nodes[parkingNode].name;

        return crow::response(res);
    });

    // ---- BUG FIX: /update now calls updateCapacity() which preserves reservations ----
    CROW_ROUTE(app,"/update")
    ([](const crow::request& req){

        crow::json::wvalue res;

        auto idParam  = req.url_params.get("id");
        auto capParam = req.url_params.get("cap");

        if(!idParam || !capParam){
            res["success"] = false;
            res["error"]   = "Missing id or cap parameter";
            return crow::response(res);
        }

        int id  = stoi(idParam);
        int cap = stoi(capParam);

        if(cap < 0){
            res["success"] = false;
            res["error"]   = "Capacity cannot be negative";
            return crow::response(res);
        }

        if(sys.nodes.find(id) == sys.nodes.end()){
            res["success"] = false;
            res["error"]   = "Node not found";
            return crow::response(res);
        }

        if(sys.nodes[id].ghost){
            res["success"] = false;
            res["error"]   = "Cannot update ghost node";
            return crow::response(res);
        }

        // This now correctly updates slots without wiping reservations
        sys.updateCapacity(id, cap);

        res["success"] = true;

        return crow::response(res);
    });

    app.port(18080)
       .multithreaded()
       .run();
}