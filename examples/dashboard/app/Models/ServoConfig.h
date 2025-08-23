#ifndef SERVO_CONFIG_MODEL_H
#define SERVO_CONFIG_MODEL_H

#include "../../lib/MVCFramework/src/Database/Model.h"
#include "../../lib/Servo/Servo.h"
#include <Arduino.h>
#include <ArduinoJson.h>

class ServoConfigModel : public Model {
public:
    // Constructor
    ServoConfigModel() : Model("servo_configs") {}
    
    // Create a new servo config instance
    ServoConfigModel(uint8_t pin, const String& name, int minPulseWidth, int maxPulseWidth, 
                     int minAngle, int maxAngle) : Model("servo_configs") {
        setPin(pin);
        setName(name);
        setMinPulseWidth(minPulseWidth);
        setMaxPulseWidth(maxPulseWidth);
        setMinAngle(minAngle);
        setMaxAngle(maxAngle);
    }
    
    // Create from ServoConfig struct
    ServoConfigModel(const ServoConfig& config) : Model("servo_configs") {
        setPin(config.pin);
        setName(config.name);
        setMinPulseWidth(config.minPulseWidth);
        setMaxPulseWidth(config.maxPulseWidth);
        setMinAngle(config.minAngle);
        setMaxAngle(config.maxAngle);
    }
    
    // Getters
    uint8_t getPin() const { return get("pin").toInt(); }
    String getName() const { return get("name"); }
    int getMinPulseWidth() const { return get("min_pulse_width").toInt(); }
    int getMaxPulseWidth() const { return get("max_pulse_width").toInt(); }
    int getMinAngle() const { return get("min_angle").toInt(); }
    int getMaxAngle() const { return get("max_angle").toInt(); }
    
    // Setters
    void setPin(uint8_t pin) { set("pin", String(pin)); }
    void setName(const String& name) { set("name", name); }
    void setMinPulseWidth(int width) { set("min_pulse_width", String(width)); }
    void setMaxPulseWidth(int width) { set("max_pulse_width", String(width)); }
    void setMinAngle(int angle) { set("min_angle", String(angle)); }
    void setMaxAngle(int angle) { set("max_angle", String(angle)); }
    
    // Convert to ServoConfig struct
    ServoConfig toServoConfig() const {
        ServoConfig config;
        config.pin = getPin();
        config.name = getName();
        config.minPulseWidth = getMinPulseWidth();
        config.maxPulseWidth = getMaxPulseWidth();
        config.minAngle = getMinAngle();
        config.maxAngle = getMaxAngle();
        return config;
    }
    
    // Convert to JSON
    JsonDocument toJson() const {
        JsonDocument json;
        json["pin"] = getPin();
        json["name"] = getName();
        json["min_pulse_width"] = getMinPulseWidth();
        json["max_pulse_width"] = getMaxPulseWidth();
        json["min_angle"] = getMinAngle();
        json["max_angle"] = getMaxAngle();
        return json;
    }
    
    // Static methods to work with servo configurations
    
    // Initialize servo configs table
    static bool initTable() {
        CsvDatabase* db = Model::getDatabase();
        if (!db) return false;
        
        // Create table if it doesn't exist
        if (!db->tableExists("servo_configs")) {
            std::vector<String> columns = {
                "pin", "name", "min_pulse_width", "max_pulse_width", 
                "min_angle", "max_angle"
            };
            return db->createTable("servo_configs", columns);
        }
        
        return true;
    }
    
    // Save a servo configuration
    static bool saveConfig(const ServoConfig& config) {
        ServoConfigModel* existing = findByPin(config.pin);
        bool result = false;
        
        if (existing) {
            // Update existing
            existing->setName(config.name);
            existing->setMinPulseWidth(config.minPulseWidth);
            existing->setMaxPulseWidth(config.maxPulseWidth);
            existing->setMinAngle(config.minAngle);
            existing->setMaxAngle(config.maxAngle);
            result = existing->save();
            delete existing;
        } else {
            // Create new
            ServoConfigModel newConfig(config);
            result = newConfig.save();
        }
        
        return result;
    }
    
    // Find a servo config by pin
    static ServoConfigModel* findByPin(uint8_t pin) {
        CsvDatabase* db = Model::getDatabase();
        if (!db) return nullptr;
        
        // Make sure table exists
        if (!db->tableExists("servo_configs")) {
            if (!initTable()) {
                return nullptr;
            }
        }
        
        // Find by pin
        std::map<String, String> conditions;
        conditions["pin"] = String(pin);
        
        std::vector<std::map<String, String>> results = db->select("servo_configs", conditions);
        
        if (results.size() > 0) {
            ServoConfigModel* config = new ServoConfigModel();
            config->fill(results[0]);
            return config;
        }
        
        return nullptr;
    }
    
    // Find a servo config by name
    static ServoConfigModel* findByName(const String& name) {
        CsvDatabase* db = Model::getDatabase();
        if (!db) return nullptr;
        
        // Make sure table exists
        if (!db->tableExists("servo_configs")) {
            if (!initTable()) {
                return nullptr;
            }
        }
        
        // Find by name
        std::map<String, String> conditions;
        conditions["name"] = name;
        
        std::vector<std::map<String, String>> results = db->select("servo_configs", conditions);
        
        if (results.size() > 0) {
            ServoConfigModel* config = new ServoConfigModel();
            config->fill(results[0]);
            return config;
        }
        
        return nullptr;
    }
    
    // Get all servo configurations
    static std::vector<ServoConfigModel*> getAll() {
        std::vector<ServoConfigModel*> configs;
        CsvDatabase* db = Model::getDatabase();
        if (!db) return configs;
        
        // Make sure table exists
        if (!db->tableExists("servo_configs")) {
            if (!initTable()) {
                return configs;
            }
        }
        
        std::vector<std::map<String, String>> results = db->select("servo_configs");
        
        for (const auto& row : results) {
            ServoConfigModel* config = new ServoConfigModel();
            config->fill(row);
            configs.push_back(config);
        }
        
        return configs;
    }
    
    // Delete a servo config by pin
    static bool deleteByPin(uint8_t pin) {
        ServoConfigModel* config = findByPin(pin);
        if (config) {
            bool result = config->delete_();
            delete config;
            return result;
        }
        return false;
    }
    
    // Delete a servo config by name
    static bool deleteByName(const String& name) {
        ServoConfigModel* config = findByName(name);
        if (config) {
            bool result = config->delete_();
            delete config;
            return result;
        }
        return false;
    }
    
    // Load all saved configurations into ServoManager
    static bool loadAllToManager(ServoManager& manager) {
        std::vector<ServoConfigModel*> configs = getAll();
        bool allLoaded = true;
        
        for (ServoConfigModel* config : configs) {
            ServoConfig servoConfig = config->toServoConfig();
            if (!manager.addServo(servoConfig)) {
                allLoaded = false;
            }
            delete config;
        }
        
        return allLoaded;
    }
    
    // Save all current ServoManager configurations to database
    static bool saveAllFromManager(ServoManager& manager) {
        std::vector<ServoStatus> statusList = manager.getAllServoStatus();
        bool allSaved = true;
        
        for (const ServoStatus& status : statusList) {
            // Get the config from manager (we need to access the private configs map)
            // For now, we'll create a basic config - this might need enhancement
            ServoConfig config;
            config.pin = status.pin;
            config.name = status.name;
            config.minPulseWidth = 500;  // Default values - could be enhanced
            config.maxPulseWidth = 2500;
            config.minAngle = 0;
            config.maxAngle = 180;
            
            if (!saveConfig(config)) {
                allSaved = false;
            }
        }
        
        return allSaved;
    }
};

#endif // SERVO_CONFIG_MODEL_H
