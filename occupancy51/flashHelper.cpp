// flashHelper.cpp

#include "flashHelper.h" 

void FlashHelper::init(Config* configptr) {   
    this->pC = configptr;
}

void FlashHelper::begin() {
    SERIAL_PRINT("Beginning EEPROM with ");
    SERIAL_PRINT(TOTAL_ALLOC_SIZE);
    SERIAL_PRINTLN(" bytes of memory..");    
    EEPROM.begin(TOTAL_ALLOC_SIZE);  // request memory allocation for all 3 copies
}

void FlashHelper::commit() {
    EEPROM.commit(); 
    SERIAL_PRINTLN ("EEPROM writes committed.");
}

void FlashHelper::end() {
    EEPROM.end(); 
    SERIAL_PRINTLN ("EEPROM released.");
}

/* 
   Take the data from pC and store in flash:
   Write 3 copies of the data structure for safety
*/
bool FlashHelper::writeFlash () {    
    StorageBlock block;
    block.magic_number = MAGIC_NUMBER;  // finger print of the data block
    block.device_id = pC->device_id;
    block.group_id =  pC->group_id;
    block.threshold = pC->threshold;     

    int struct_size = sizeof(block);
    SERIAL_PRINT ("Size of StorageBlock structure: ");
    SERIAL_PRINT(struct_size);     
    SERIAL_PRINTLN(" bytes");
    if (struct_size > (BLOCK_ADDRESS2 - BLOCK_ADDRESS1) || 
        struct_size > (BLOCK_ADDRESS3 - BLOCK_ADDRESS2) || 
        struct_size > (TOTAL_ALLOC_SIZE - BLOCK_ADDRESS3)) {
        SERIAL_PRINTLN("StorageBlock is too big to fit in the allocated memory. Aborting...");
        return false;
    }
    
    SERIAL_PRINTLN("Writing device id...");
    begin();  
    EEPROM.put (BLOCK_ADDRESS1, block);
    EEPROM.put (BLOCK_ADDRESS2, block);
    EEPROM.put (BLOCK_ADDRESS3, block);
    commit();  // do not end, as you will need to read it back later  
    
    delay(250);
    SERIAL_PRINTLN ("\nFinished writing.");
    return true;
}  

/* 
   Read all 3 blocks and compare them. If OK, return one copy of device identifier 
   Return value of the function : true if OK, false if error occured
   The device id, group id and threshold are stored in the config object pC.
*/

bool FlashHelper::readFlash () {
    DeviceIdentifier ident1, ident2, ident3;  
    if (readBlock(BLOCK_ADDRESS1, ident1)) {  // true if the block is corrupted
        SERIAL_PRINTLN ("Block 1 is corrupted");      
        return false;
    }
    if (readBlock(BLOCK_ADDRESS2, ident2)) { // true if the block is corrupted
        SERIAL_PRINTLN ("Block 2 is corrupted");      
        return false;  
    }
    if (readBlock(BLOCK_ADDRESS3, ident3)) { // true if the block is corrupted
        SERIAL_PRINTLN ("Block 3 is corrupted");
        return false;
    }
    if (ident1.device_id != ident2.device_id) {
        SERIAL_PRINTLN ("Device_id in copies 1&2 do not match !");
        return false;
    }
    if (ident1.device_id != ident3.device_id) {
        SERIAL_PRINTLN ("Device_id in copies 1&3 do not match !");
        return false;
    }    
    if (ident1.group_id != ident2.group_id) {
        SERIAL_PRINTLN ("Group_id in copies 1&2 do not match !");
        return false;
    }
    if (ident1.group_id != ident3.group_id) {
        SERIAL_PRINTLN ("Group_id in copies 1&3 do not match !");
        return false;
    }
    // TODO: compare the 3 threshold values also
    pC->device_id = ident1.device_id;
    pC->group_id = ident1.group_id;    
    pC->threshold = temp_thrshold;
    return true;
}

// Returns true if the block is OK, and false if it is corrupted 
bool FlashHelper::readBlock (int block_addr, DeviceIdentifier& ident) {
    StorageBlock block;  
    EEPROM.get (block_addr, block);
    #ifdef VERBOSE_MODE    
        SERIAL_PRINT("address :");
        SERIAL_PRINTLN(block_addr);    
        SERIAL_PRINT("magic_number: ");
        SERIAL_PRINTLNF(block.magic_number, HEX);   
        SERIAL_PRINT("device_id: ");
        SERIAL_PRINTLN(block.device_id);   
        SERIAL_PRINT("group_id: ");
        SERIAL_PRINTLN(block.group_id);   
        SERIAL_PRINT("threshold: ");
        SERIAL_PRINTLN(block.threshold);     
        SERIAL_PRINTLN("-------------------------");   
    #endif    
    ident.is_corrupted = false;    
    if (block.magic_number != MAGIC_NUMBER) {
        ident.is_corrupted = true;
        SERIAL_PRINTLN("FLASH ERROR: Magic number is corrupted !");
        return (false);
    }
    ident.device_id = block.device_id;
    ident.group_id  = block.group_id;  
    temp_thrshold = block.threshold; // stored in a local variable, pending validation
    return(true);
}

bool FlashHelper::testMemory() {
    SERIAL_PRINTLN("--------------------------");
    SERIAL_PRINTLN ("Performing memory self-test...");
    //SERIAL_PRINT ("\nsize of long: ");  // 4 bytes
    //SERIAL_PRINTLN (sizeof(long));
    SERIAL_PRINT ("size of StorageBlock: ");
    SERIAL_PRINTLN (sizeof(StorageBlock)); 
      
    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();  // set in the IDE
    FlashMode_t ideMode = ESP.getFlashChipMode();

    SERIAL_PRINTF("Flash chip id:   %08X\n", ESP.getFlashChipId());
    SERIAL_PRINTF("Flash real size: %u\n", realSize);
    SERIAL_PRINTF("Flash IDE  size: %u\n", ideSize);
    SERIAL_PRINTF("Flash IDE speed: %u\n", ESP.getFlashChipSpeed());
    SERIAL_PRINTF("Flash IDE mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? 
        "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

    boolean result = 1;
    if(ideSize != realSize) {
        SERIAL_PRINTLN("Flash Chip configuration in the IDE is wrong!\n");
        result = 0;
    } else {
        SERIAL_PRINTLN("Flash Chip configuration is OK.");
        result = 1;
    }
    SERIAL_PRINTLN("--------------------------");
    return (result);
}



 
