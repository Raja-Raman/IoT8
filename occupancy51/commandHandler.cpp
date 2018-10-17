// commandHandler.cpp

#include "commandHandler.h"
/**
    NOTE: we are avoiding unnecessary STATUS_CMD_ACK messages because it my interfere with
    chaining of multiple commands from the gateway. TODO: revisit this.
**/

extern void send_on_demand_data();
extern void send_response(long command, int response);
extern void reset_data_interval(long new_value);

#ifdef VERBOSE_MODE 
// this string array is only used for serial debugging
const char* cmd_names[] = { // maintain the same order as in common.h !
    "CMD_HELLO",                //0
    "CMD_GET_DATA",             //1
    "CMD_SET_THRESHOLD",        //2   
    "CMD_SET_DATA_INTERVAL",    //3  
    "CMD_TARGET_PROD_URL",      //4
    "CMD_TARGET_TEST_URL",      //5
 
    "CMD_SET_DEVICE_ID",        //6
    "CMD_SET_GROUP_ID",         //7
    "CMD_SET_FW_SERVER_URL",    //8
    "CMD_UPDATE_FIRMWARE",      //9
    "CMD_WRITE_FLASH_CONFIG",   //10
    "CMD_RESTART",              //11

    "CMDX_PARSING_ERROR",       //12
    "CMDX_REGULAR_DATA",        //13
    "CMDX_EVENT_DATA"           //14
}; 
#endif

CommandHandler::CommandHandler() {
}
 
void CommandHandler::init (Config* configptr) {
    this->pC = configptr;
} 

void CommandHandler::sendResponse(int response) {
    send_response (command_cache.command, response);
}

void CommandHandler::handleCommand (const CommandData& cmd) {
    if (cmd.command == CMDX_PARSING_ERROR) {
        sendParsingError();
        return;
    }
    
    /*************
    // if the packet is not addressed to me, ignore it.  
    // (this will be relevant when listening for wild card MQTT topics)
    if (!checkRecipientAddress(cmd))
        return;
    ****************/
    
    // make a backup copy first
    copyCommand (cmd);
    // at this point, the copy has all the data, cmd parameter is safe to be released
    
    #ifdef VERBOSE_MODE
        printCommand();
    #endif

    switch(command_cache.command) {  
        case CMD_HELLO:
            sendHandshake();
            break;      
        case CMD_GET_DATA:
            send_on_demand_data();
            break; 
        case CMD_SET_THRESHOLD:
            setThreshold();
            break;  
        case CMD_SET_DATA_INTERVAL:
            setDataInterval();
            break;    
        case CMD_TARGET_PROD_URL:
            targetProductionUrl();
            break;              
        case CMD_TARGET_TEST_URL:
            targetTestUrl();
            break;               
        case CMD_SET_DEVICE_ID:  
            setDeviceID ();
            break;            
        case CMD_SET_GROUP_ID:  
            setGroupID ();
            break;               
        case CMD_SET_FW_SERVER_URL:
            setFirmwareServerUrl();
            break;
        case CMD_UPDATE_FIRMWARE:
            updateFirmware();
            break;      
        case CMD_WRITE_FLASH_CONFIG: // to save it permanently, you must call this 
             writeFlashConfig();
            break; 
        case CMD_RESTART:
            restart();
            break;           
        default:
            sendParsingError();
            break;
    }
}

//  Makes a local copy of the command object before another mqtt command arrives
// TODO: keep a small queue of command objects, at least 2.
void CommandHandler::copyCommand (const CommandData& cmd) {
    command_cache.device_id = cmd.device_id;
    command_cache.group_id = cmd.group_id;
    command_cache.command = cmd.command;
    command_cache.long_param = cmd.long_param;
    if (strlen(cmd.string_param) > 0) {
        strncpy (command_cache.string_param, cmd.string_param, MAX_STRING_LENGTH-1);
        command_cache.string_param[MAX_STRING_LENGTH-1] = '\0';   
    }                
}

/*************
    For a command to be obeyed, the group id must be the device's group id and the
    device id should match with the device's id. -OR- 
    one of both of them can also be the wild card broadcast value (zero).
    // Note: It is relevant only for MQTT with wild card topics.
**************/
bool CommandHandler::checkRecipientAddress (const CommandData& cmd) {
    if (cmd.device_id != pC->device_id && cmd.device_id != BROADCAST_DEVICE_ID) 
        return (false);
    if (cmd.group_id != pC->group_id && cmd.group_id != BROADCAST_GROUP_ID) 
        return (false);
    return (true); 
}

void CommandHandler::sendParsingError() {
    SERIAL_PRINT ("* ERROR: Command "); 
    SERIAL_PRINT(command_cache.command);     
    SERIAL_PRINTLN(" is not found *");
    sendResponse (CMDX_PARSING_ERROR); 
}            
            
void CommandHandler::sendHandshake() {
    SERIAL_PRINTLN("Replying to hello..");
    sendResponse (STATUS_CMD_ACK);    
}

// temporarily set device_id for the rest of the session
// NOTE: once you have set a new device id, the payload should have the new device id 
// (or the universal id 0).Otherwise the command will be ignored.
// Tip: First set the group_id to 0 and then change the device_id.
void CommandHandler::setDeviceID()
{
    pC->device_id = command_cache.device_id;
    SERIAL_PRINT("Till this device is restarted, temporary device id is: ");
    SERIAL_PRINTLN(pC->device_id);
    sendResponse (STATUS_CMD_SUCCESS); // with new id 
}

// temporarily set group_id for the rest of the session
// Tip: First set the device_id to the new value and then change the group_id.
void CommandHandler::setGroupID()
{
    pC->group_id = command_cache.group_id;
    SERIAL_PRINT("Till this device is restarted, temporary group id is: ");
    SERIAL_PRINTLN(pC->group_id);
    sendResponse (STATUS_CMD_SUCCESS); // with new id 
}

// temporarily set threhsold for the rest of the session
// Threshold value is in the long_param of command
void CommandHandler::setThreshold(){
    pC->threshold = command_cache.long_param;
    SERIAL_PRINT("Threshold temporarily set to: ");
    SERIAL_PRINTLN(pC->threshold);
    sendResponse (STATUS_CMD_SUCCESS);  
}

// Permanently save the device config to flash.
// Before calling this, you must set deviceID, groupID, and optionally, 
// the threshold into the config object pC.
void CommandHandler::writeFlashConfig() {
    FlashHelper F;
    F.init(pC);
    F.begin();
    bool result = F.writeFlash();
    F.end();
    if (result) {
        SERIAL_PRINTLN ("Device configuration stored in Flash.");
        sendResponse (STATUS_CMD_SUCCESS); 
    } else {
        SERIAL_PRINTLN ("*** Failed to write device configuration to Flash ***");
        sendResponse (STATUS_CMD_FAILED); 
    }
}
   
bool CommandHandler::verifyURL (const char* new_url) {
    if (strlen(new_url) >= MAX_STRING_LENGTH) {
      SERIAL_PRINTLN ("* ERROR: URL string is too long *");
      return (false);
    }
    if (!String(new_url).startsWith("http:")) {
      SERIAL_PRINTLN ("* ERROR: malformed URL *");
      return (false);     
    }
    return (true);  // TODO: check with a list of preconfigured urls, domain check etc 
}

// temporarily set device config for the current session
void CommandHandler::setFirmwareServerUrl () {
    // The new firmware URL is in command_cache.string_param  
    if (verifyURL(command_cache.string_param)) {
        strncpy (pC->firmware_server_url, command_cache.string_param, MAX_STRING_LENGTH);
        pC->firmware_server_url[MAX_STRING_LENGTH-1] = '\0';  // if overflows, unterminated ! 
    } else {
        SERIAL_PRINTLN ("verifyURL failed");
        sendResponse (STATUS_CMD_FAILED);   
        return;
    }
    SERIAL_PRINT("Firmware server URL temporarily set to: ");
    SERIAL_PRINTLN(pC->firmware_server_url);    
    sendResponse (STATUS_CMD_SUCCESS);    
}

void CommandHandler::updateFirmware() {  
    sendResponse (STATUS_CMD_ACK);
    OtaHelper O;
    O.init(pC);
    O.check_and_update(); // this will restart the device
}
  
void CommandHandler::restart () {
    sendResponse (STATUS_CMD_ACK);
    SERIAL_PRINTLN ("\n\n!!! Occupancy sensor is about to restart !!!\n");
    delay(1000);
    ESP.restart();
}
  
void CommandHandler::setDataInterval(){
    reset_data_interval(command_cache.long_param);
    sendResponse (STATUS_CMD_SUCCESS);  
}  

void CommandHandler::targetProductionUrl() {
    pC->targetProdUrl();
    sendResponse (STATUS_CMD_SUCCESS);      
}

void CommandHandler::targetTestUrl(){
    pC->targetTestUrl();
    sendResponse (STATUS_CMD_SUCCESS);      
}
  
#ifdef VERBOSE_MODE  
void CommandHandler::printCommand() {   // print from cache
    SERIAL_PRINT("device_id: ");
    SERIAL_PRINTLN(command_cache.device_id);
    SERIAL_PRINT("group_id: ");
    SERIAL_PRINTLN(command_cache.group_id);
    SERIAL_PRINT("command: (");
    SERIAL_PRINT(command_cache.command);
    SERIAL_PRINT(") ");
    SERIAL_PRINTLN(cmd_names[command_cache.command]);
    SERIAL_PRINT("long_param: ");
    SERIAL_PRINTLN(command_cache.long_param);
    SERIAL_PRINT("string_param: ");
    SERIAL_PRINTLN(command_cache.string_param);
    SERIAL_PRINTLN("-------------------------");   
}
#endif


