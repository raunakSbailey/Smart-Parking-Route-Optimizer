#include "crow.h"
#include "MainCode.cpp"

using namespace std;

bool loginUser(
    const string& user,
    const string& pass,
    string& role
){

    ifstream file("./UserFiles/users.txt");

    if(!file.is_open())
        return false;

    string u,p,r;

    while(file >> u >> p >> r){

        if(
            u == user &&
            p == pass
        ){

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

    ifstream check(
        "./UserFiles/users.txt"
    );

    string u,p,r;

    while(check >> u >> p >> r){

        if(u == user){

            check.close();
            return false;
        }
    }

    check.close();

    ofstream file(
        "./UserFiles/users.txt",
        ios::app
    );

    file
    << user
    << " "
    << pass
    << " user\n";

    file.close();

    return true;
}

int main(){

    crow::SimpleApp app;

    CROW_ROUTE(app,"/")
    ([]{

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

        auto userParam =
        req.url_params.get("user");

        auto passParam =
        req.url_params.get("pass");

        string user =
        userParam ? string(userParam) : "";

        string pass =
        passParam ? string(passParam) : "";

        string role = "user";

        bool ok =
        loginUser(
            user,
            pass,
            role
        );

        res["success"] = ok;
        res["role"] = role;

        return crow::response(res);
    });

    CROW_ROUTE(app,"/register")
    ([](const crow::request& req){

        crow::json::wvalue res;

        auto userParam =
        req.url_params.get("user");

        auto passParam =
        req.url_params.get("pass");

        string user =
        userParam ? string(userParam) : "";

        string pass =
        passParam ? string(passParam) : "";

        bool ok =
        registerUser(
            user,
            pass
        );

        res["success"] = ok;

        if(!ok)
            res["error"] =
            "User already exists";

        return crow::response(res);
    });

    CROW_ROUTE(app,"/nodes")
    ([]{

        crow::json::wvalue res;

        int i=0;

        for(auto &p : sys.nodes){

            auto n = p.second;

            if(n.ghost)
                continue;

            res[i]["id"] = n.id;
            res[i]["name"] = n.name;
            res[i]["capacity"] = n.capacity;
            res[i]["x"] = n.x;
            res[i]["y"] = n.y;

            i++;
        }

        return crow::response(res);
    });

    CROW_ROUTE(app,"/route")
    ([](const crow::request& req){

        crow::json::wvalue res;

        auto s =
        req.url_params.get("start");

        auto e =
        req.url_params.get("end");

        auto sh =
        req.url_params.get("startH");

        auto eh =
        req.url_params.get("endH");

        int startH =
        sh ? stoi(sh) : 10;

        int endH =
        eh ? stoi(eh) : 12;

        int src =
        sys.getId(s ? s : "");

        int dest =
        sys.getId(e ? e : "");

        if(src==-1 || dest==-1){

            res["error"] =
            "Invalid nodes";

            return crow::response(res);
        }

        auto realPath =
        sys.dijkstra(
            src,
            dest,
            sys.road
        );

        auto path =
        sys.buildVisualRoute(realPath);

        if(path.empty()){

            res["error"] =
            "No route";

            return crow::response(res);
        }

        int parkingNode = -1;

        bool fallback = false;

        // DESTINATION AVAILABLE
        if(
            sys.availableAtTime(
                dest,
                startH,
                endH
            )
        ){

            parkingNode = dest;
        }

        // BACKTRACK
        else{

            fallback = true;

            for(
                int i=path.size()-1;
                i>=0;
                i--
            ){

                int node = path[i];

                if(
                    sys.nodes[node]
                    .ghost
                )
                    continue;

                if(
                    sys.availableAtTime(
                        node,
                        startH,
                        endH
                    )
                ){

                    parkingNode = node;
                    break;
                }
            }
        }

        if(parkingNode==-1){

            res["error"] =
            "No parking available";

            return crow::response(res);
        }

        // RESERVE
        sys.reserveSlot(
            parkingNode,
            startH,
            endH
        );

        int i=0;

        for(int id : path){

            auto n =
            sys.nodes[id];

            res["path"][i]["x"] =
            n.x;

            res["path"][i]["y"] =
            n.y;

            res["path"][i]["ghost"] =
            n.ghost;

            i++;
        }

        // TRANSPORT
        if(fallback){

            res["transport"][0]["x"] =
            sys.nodes[parkingNode].x;

            res["transport"][0]["y"] =
            sys.nodes[parkingNode].y;

            res["transport"][1]["x"] =
            sys.nodes[dest].x;

            res["transport"][1]["y"] =
            sys.nodes[dest].y;
        }

        res["fallback"] =
        fallback;

        res["parkingNode"] =
        sys.nodes[parkingNode]
        .name;

        return crow::response(res);
    });

    CROW_ROUTE(app,"/update")
    ([](const crow::request& req){

        crow::json::wvalue res;

        auto idParam =
        req.url_params.get("id");

        auto capParam =
        req.url_params.get("cap");

        if(
            !idParam ||
            !capParam
        ){

            res["success"] = false;

            return crow::response(res);
        }

        int id =
        stoi(idParam);

        int cap =
        stoi(capParam);

        if(
            sys.nodes.find(id)
            ==
            sys.nodes.end()
        ){

            res["success"] = false;

            return crow::response(res);
        }

        sys.nodes[id]
        .capacity = cap;

        sys.saveNodes();

        res["success"] = true;

        return crow::response(res);
    });

    app.port(18080)
    .multithreaded()
    .run();
}