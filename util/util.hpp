#ifndef UTIL_HPP
# define UTIL_HPP

#include <vector>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <sys/event.h>

std::vector<std::string> split(const std::string &str, const std::string &del);
void handleError(std::string errMsg);
void makeNonBlock(int fd);
void printEventTypes(const std::vector<struct kevent> &change_list);

#endif