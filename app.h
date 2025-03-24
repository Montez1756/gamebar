#ifndef APP_H
#define APP_H

#include <string>

class App{

    private:
        

    public:
        App();
        bool checkForUpate();
        void update();
        std::string getContent(const std::string &url);
};

#endif
