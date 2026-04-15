#include <iostream>
#include <string>

#include "Core/console.h"

struct AppState
{
    bool running;
};

int main(int argc, char* argv[])
{
    AppState state;
    state.running = true;
    
    console::enableAnsi();
    // console::enableRawInput(); // For later - better navigation etc

    while (state.running)
    {
        std::string inputLine;
        std::getline(std::cin, inputLine);
        std::cout << "Echo: " + inputLine;
    }
    return 0;
}
