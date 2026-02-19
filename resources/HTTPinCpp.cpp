#include "httplib.h"
#include <iostream>

using namespace std;

int main () {
    /*httplib :: Client cli ("http://www.youtube.com");
    auto res = cli.Get ("/api");
    std::cout << res -> status << std::endl << res -> has_header << std::endl;*/

    cout << "Please enter text: ";
    string s;
    getline (cin, s);

    if (s.size()) {
        "{ \"contents\": [ { \"parts\": [ { \"text\": \"" + s + "\" } ] } ] }";
    } 

    


    /*{
        "contents" : [
            {
                "parts" : [
                    { "text" : s}
                ]
            }
        ]
    }*/

}