/*
    Backup and restore tool for Aerotech Automation1 iSMC.
    Downloads MCD files to a chosen location from a connected controller.
    Uploads MCD files to a connected controller from a chosen location.
*/

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <numeric>
#include <exception>
#include <iomanip>

#include "Automation1.h"
#include "backuprestore.h"
#include "BackupRestoreConfig.h"

#include "readline/readline.h"
#include "readline/history.h"


// Loops forever accepting user input and checking it against a command list.
int main(int argc, char* argv[])
{
    std::map<std::string,std::string,case_insensitive_map::comp> cmd_list;
    populate_cmd_list(&cmd_list);
    std::string strInput;
    char delim[] = " ";
    Automation1Controller controller = nullptr;

    // report version
    std::cout << argv[0] << " Version " << BackupRestore_VERSION_MAJOR << "." << BackupRestore_VERSION_MINOR << std::endl;
    while(true)
    {
        std::cout << "\nEnter a command (or \"exit\" to exit or \"help\" for a list of commands)" << std::endl;
        strInput = readline(">>> ");
        if(strInput != "")
        {    
            add_history(strInput.c_str());
            //Split input into tokens. Tokens will contain the command as well as any args.
            std::vector<std::string> args = parse_cmd(strInput,delim);
            
            try
            {
                    execute_command(args, cmd_list, &controller);
            }
            catch(const std::exception& e)
            {
                    std::cerr << e.what() << '\n';
            }
        }
        else
        {
                std::cout << "ERROR: Please enter a command!" << std::endl;
        }
    }
     
    return 0;
}//END OF main
/*############################################################################################################################*/

///@brief Takes a string as input and returns a vector with the string split into tokens according to delimiter list.
///@param[In] str The string to split.
///@param[In] delim Delimiter list to split on. Splits on whitespace by default.
///@return Returns a vector<string> containing the split string.
std::vector<std::string> parse_cmd(std::string str, char delim[])
{       
    std::vector<std::string> ret;                   //return vector of tokens

    int n = str.length();                           //Length of char array
    char charInput[n + 1];                          //declaring character array
            
    strcpy(charInput, str.c_str());                 //copying the contents of the
    char* pch = strtok(charInput, delim);           //string to char array

    while(pch != NULL)                              //Split string and add to vector until end is reached.
    {
            ret.push_back(pch);
            pch = strtok(NULL, delim);
    }
    return ret;
}

// Main workhorse of the program. Determines which command to execute based on user input.
bool execute_command(std::vector<std::string> args, std::map<std::string,std::string,case_insensitive_map::comp> cmd_list, Automation1Controller *controller)
{
    std::string keyword = args[0];
    bool ret = false;
    //Scans through the keys in cmd_list for a ->case-insensitive<- match.
    //Return of find is an iterator pointing to the element.

    if (cmd_list.find(keyword) != cmd_list.end()) 
    {       
        //Found the keyword in cmd_list. Determine which.
        if(strcasecmp(keyword.c_str(), "getapi") == 0)
        {
            getApi();
            return true;
        }
        else if(strcasecmp(keyword.c_str(), "connect") == 0)
        {
            return connect(args, controller);
        }
        else if(strcasecmp(keyword.c_str(), "disconnect") == 0)
        {
            return disconnect(*controller);
        }
        else if(strcasecmp(keyword.c_str(), "exit") == 0)
        {
            if(areWeConnected(*controller))
            {  
                std::cout << "Disconnecting as we exit." << std::endl;
                disconnect(*controller);
            }
            std::cout << "Exiting..." << std::endl;
            exit(EXIT_SUCCESS);
        }
        else if(strcasecmp(keyword.c_str(), "help") == 0)
        {
            send_help(cmd_list);
            return true;
        }
        else if(strcasecmp(keyword.c_str(), "backup") == 0)
        {
            if(!areWeConnected(*controller))
            {
                std::cout << "ERROR: Not connected to a controller." << std::endl;
                return false;
            }
            return backup(args, controller);
        }
        else if(strcasecmp(keyword.c_str(), "restore") == 0)
        {
            if(!areWeConnected(*controller))
            {
                std::cout << "ERROR: Not connected to a controller." << std::endl;
                return false;
            }
            return restore(args, controller);
        }
/*      else if(strcasecmp(keyword.c_str(), "") == 0)
        {
            
        }
*/   
    }
    else //We don't have a command keyword so assume user entered it incorrectly and needs help.
    {
        std::cout << "ERROR: Command not found." << std::endl;
        send_help(cmd_list);
        return false;
    }
    
    //Thing was not done.
    return false;
}

// Sets up the list of commands to be used.
void populate_cmd_list(std::map<std::string,std::string, case_insensitive_map::comp>* cmd_list)
{//The commands listed here as keys must contain the same characters as those used for command comparisons later. A mismatch will prevent the cmd being properly run.
        *cmd_list =      { 
                                {"GetApi","Usage: getapi -> Prints the current version of the Aerotech C API."},
                                {"Connect", "Usage: connect <params> -> Connects to the controller."},
                                {"Disconnect","Usage: disconnect -> Disconnects from the controller."},
                                {"Exit","Usage: exit -> Exit this program."},
                                {"Help","Usage: help -> Prints this help."},
                                {"Backup","Usage: backup <PATH> -> Downloads the MCD configuration file for the currently connected controller to the location specified by <PATH>."},
                                {"Restore","Usage: restore <PATH> -> Uploads the selected MCD configuration file specified by <PATH> to the currently connected controller."}
                                //{"","Usage: "},
                        };
}

// Prints the command list to console.
void send_help(std::map<std::string,std::string, case_insensitive_map::comp> cmd_list)
{
        std::cout << std::setw(20) << std::right << "\nAvailable Commands\n" << std::endl;
        for (auto const& x : cmd_list)
        {
            std::cout << std::setw(20) << std::right << x.first << " : " << std::left << x.second <<  std::endl;
        }
        std::cout << std::endl;
}

/// @brief Gets the current version of the Aerotech C API in the format MAJOR.MINOR.PATCH
void getApi()
{
    int apiMaj, apiMin, apiPat;
    Automation1_GetApiVersion(&apiMaj, &apiMin, &apiPat);
    std::cout << "C API Version: " << apiMaj << "." << apiMin << "." << apiPat << std::endl;
}

// Connects to a controller at the specified IP address.
bool connect(std::vector<std::string> args, Automation1Controller* controller)
{
    if(areWeConnected(*controller))
    {
        std::cout << "ERROR: Already connected to a controller." << std::endl;
        return false;
    }
    const char *host = args[1].c_str();
    bool ret;
    ret = Automation1_ConnectWithHost(host, controller);
    if(ret)
    {
        std::cout << "Connected to controller." << std::endl;      
    }
    else
    {
        logError("Unable to connect to controller.");
    }
    return ret;
}

// Disconnects from the currently connected controller.
bool disconnect(Automation1Controller controller)
{
    if(!areWeConnected(controller))
    {
        std::cout << "ERROR: Not connected to a controller." << std::endl;
        return false;
    }
    bool ret;
    ret = Automation1_Disconnect(controller);
    if(ret)
    {
        std::cout << "Disconnected from controller." << std::endl;
        controller = nullptr;
    }
    else
    {
        logError("Unable to disconnect from controller.");
    }
    return ret;
}

// Checks if there is an active connection to a controller.
bool areWeConnected(Automation1Controller controller)
{
    if(controller != nullptr)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Asks user a yes/no question and retrieves the answer.
bool getUserChoice(std::string requestToUser)
{
    std::string strInput;
    while(true)
    {
        std::cout << requestToUser << " (y/n)" << std::endl;
        strInput = readline(">>> ");
        if(strInput == "y")
        {
            return true;
        }
        else if(strInput == "n")
        {
            return false;
        }
        else
        {
            std::cout << "Please respond with either y or n." << std::endl;
        }
    }

}

// Downloads the connected controller's MCD configuration file.
bool backup(std::vector<std::string> args, Automation1Controller* controller)
{
    bool ret;
    bool incFiles = false;
    bool incConfig = false;
    std::string fnMCDBackupLocation = args[1];                                    // Path to download MCD file to.

    // Ask user if they want to include files/config in backup.
    incFiles = getUserChoice("Include all files in the backup?");
    if(incFiles) std::cout << "Including all files in backup.\n" << std::endl;
    else std::cout << "Not including files in backup.\n" << std::endl;

    incConfig = getUserChoice("Include all configuration settings in the backup?");
    if(incConfig) std::cout << "Including all configuration settings in backup.\n" << std::endl;
    else std::cout << "Not including configuration settings in backup.\n" << std::endl;

    ret = Automation1_Controller_DownloadMcdToFile(*controller, fnMCDBackupLocation.c_str(), incFiles, incConfig);
    if(!ret)
    {
        logError(std::string("Failed to download MCD to: "+fnMCDBackupLocation));
    }
    else
    {
        std::cout << "Downloading MCD backup to: " << fnMCDBackupLocation << std::endl;
    }
    return ret;
}

// TODO: Put restore API call in here.
// Uploads the selected MCD configuration file to the currently connected controller.
bool restore(std::vector<std::string> args, Automation1Controller* controller)
{
    bool ret;
    bool incFiles = false;
    bool incConfig = false;
    bool eraseController = false;
    std::string fnMCDRestoreLocation = args[1];                                    // Path from which to restore MCD file.

    // Ask user if they want to include files/config in backup.
    incFiles = getUserChoice("Include all files in the upload?");
    if(incFiles) std::cout << "Including all files in upload.\n" << std::endl;
    else std::cout << "Not including files in upload.\n" << std::endl;

    incConfig = getUserChoice("Include all configuration settings in the upload?");
    if(incConfig) std::cout << "Including all configuration settings in upload.\n" << std::endl;
    else std::cout << "Not including configuration settings in upload.\n" << std::endl;

    eraseController = getUserChoice("Erase controller before uploading MCD?");
    if(eraseController) std::cout << "Erasing controller before upload.\n" << std::endl;
    else std::cout << "Not erasing controller before upload.\n" << std::endl;

    ret = Automation1_Controller_UploadMcdToController(*controller, fnMCDRestoreLocation.c_str(), incFiles, incConfig, eraseController);
    if(!ret)
    {
        logError(std::string("Failed to upload MCD from: "+fnMCDRestoreLocation));
    }
    else
    {
        std::cout << "Uploading MCD from: " << fnMCDRestoreLocation << std::endl;
    }
    return ret;
}

// A wrapper function to retrieve any controller errors and print to console.
void logError(std::string programErrorMessage)
{
    std::cout << programErrorMessage << std::endl;
    int size = 2048;
    char controllerErrorMessage[size];
    Automation1_GetLastErrorMessage(controllerErrorMessage, size);
    std::cout << "ERRMSG: " << std::string(controllerErrorMessage) << std::endl;
}