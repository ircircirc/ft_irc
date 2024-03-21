#include "ConfigManager.hpp"

int main(int argc, char **argv)
{
    ConfigManager conf(argc, argv);
    while (1)
        conf.routine();
    return 0;
}