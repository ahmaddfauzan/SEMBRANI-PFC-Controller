#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>

enum class SystemMode : uint8_t {
    PHASE_1 = 0,
    PHASE_3 = 1
};

struct PFCConfig {
    SystemMode mode;
    float capacitorPerStepKvar;
    float targetPF;
    uint8_t page;
};

class ConfigManager {
public:
    ConfigManager();

    void begin();

    PFCConfig getConfig() const;

    void setMode(SystemMode mode);
    void setCapacitorPerStep(float kvar);
    void setTargetPF(float pf);
    void setPage(uint8_t page);

    uint8_t getPageCount() const;

private:
    PFCConfig config;

    void loadDefaults();
};

#endif