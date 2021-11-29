#pragma once

#include <cstdlib>
#include <vector>
#include <string>
#include <map>
#include <strings.h>

// Makes map keys case insensitive - Used for cmd_list
struct case_insensitive_map {
    struct comp {
        bool operator() (const std::string& lhs, const std::string& rhs) const {
            return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
        }
    };
};



std::vector<std::string> parse_cmd(std::string str, char delim[]);
bool execute_command(std::vector<std::string> args, std::map<std::string,std::string,case_insensitive_map::comp> cmd_list, Automation1Controller *controller);
void populate_cmd_list(std::map<std::string,std::string, case_insensitive_map::comp>* cmd_list);
void send_help(std::map<std::string,std::string, case_insensitive_map::comp> cmd_list);
void getApi(void);
bool connect(std::vector<std::string> args, Automation1Controller* controller);
bool disconnect(Automation1Controller controller);
bool areWeConnected(Automation1Controller controller);
bool backup(std::vector<std::string> args, Automation1Controller* controller);
bool restore(std::vector<std::string> args, Automation1Controller* controller);
void logError(std::string programErrorMessage);

