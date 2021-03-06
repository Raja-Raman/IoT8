// commandHandler.h

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "common.h"
#include "config.h"
#include "jayson.h"
#include "otaHelper.h"
#include "flashHelper.h"

class CommandHandler {
public:
    CommandHandler();
    void init (Config* configptr);
    void handleCommand (const CommandData& cmd);   
private:
    Config *pC;
    CommandData command_cache;  // local copy of the incoming command packet
    void copyCommand (const CommandData& cmd) ;   
    void sendResponse(int response);
    
    void sendHandshake(); 
    void setDataInterval();
    void targetProductionUrl();
    void targetTestUrl();
    void setDeviceID();
    void setGroupID();
    void setThreshold();    
    void setFirmwareServerUrl(); 
    void updateFirmware(); 
    void writeFlashConfig();
    void restart ();
    
    void sendParsingError (); 
    void printCommand();
    bool verifyURL(const char* new_url);
    bool checkRecipientAddress (const CommandData& cmd); 
};

#endif 
