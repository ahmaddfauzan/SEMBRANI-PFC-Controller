#include "ConfigManager.h"

ConfigManager::ConfigManager() {
    loadDefaults();
}

void ConfigManager::loadDefaults() {
    config.mode = SystemMode::PHASE_1;
    config.capacitorPerStepKvar = 5.0f;
    config.targetPF = 0.90f;
    config.page = 1;
}

void ConfigManager::begin() {
    loadDefaults();
}

PFCConfig ConfigManager::getConfig() const {
    return config;
}

void ConfigManager::setMode(SystemMode mode) {
    config.mode = mode;

    // Kembali ke halaman pertama saat mode berubah.
    config.page = 1;
}

void ConfigManager::setCapacitorPerStep(float kvar) {
    if (kvar < 0.1f) {
        kvar = 0.1f;
    }

    if (kvar > 100.0f) {
        kvar = 100.0f;
    }

    config.capacitorPerStepKvar = kvar;
}

void ConfigManager::setTargetPF(float pf) {
    if (pf < 0.85f) {
        pf = 0.85f;
    }

    if (pf > 0.99f) {
        pf = 0.99f;
    }

    config.targetPF = pf;
}

uint8_t ConfigManager::getPageCount() const {
    if (config.mode == SystemMode::PHASE_1) {
        return 1;
    }

    return 4;
}

void ConfigManager::setPage(uint8_t page) {
    const uint8_t pageCount = getPageCount();

    if (page < 1) {
        page = pageCount;
    }

    if (page > pageCount) {
        page = 1;
    }

    config.page = page;
}