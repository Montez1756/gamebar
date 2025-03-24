#include "app.h"
#include <vector>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>
#include "httpClient.h"

using json = nlohmann::json;

App::App(){

    if(checkForUpate())
    {
        update();
    }
}

bool App::checkForUpate()
{
    std::ifstream localJson("version.json");
    json j = json::parse(localJson);
    std::string localVersion = j["version"];
    HttpClient client;
    std::string remoteVersion = client.getBlocking(QUrl("https://example.com/version.txt")).toStdString();
    
    std::vector<int> local, remote;
    std::stringstream lstream(localVersion), rstream(remoteVersion);
    std::string part;

    while (std::getline(lstream, part, '.')) local.push_back(std::stoi(part));
    while (std::getline(rstream, part, '.')) remote.push_back(std::stoi(part));

    for (size_t i = 0; i < std::min(local.size(), remote.size()); ++i) {
        if (remote[i] > local[i]) return true;
        if (remote[i] < local[i]) return false;
    }
    return remote.size() > local.size();
}
