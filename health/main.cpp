/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/logging.h>
#include <android/binder_interface_utils.h>
#include <health-impl/ChargerUtils.h>
#include <health-impl/HalHealthLoop.h>
#include <health-impl/Health.h>
#include <health/utils.h>

using aidl::android::hardware::health::HalHealthLoop;
using aidl::android::hardware::health::Health;
using aidl::android::hardware::health::charger::ChargerCallback;
using aidl::android::hardware::health::charger::ChargerModeMain;

static constexpr const char* gInstanceName = "default";
static constexpr std::string_view gChargerArg{"--charger"};

int main(int argc, char** argv) {
    auto config = std::make_unique<healthd_config>();
    ::android::hardware::health::InitHealthdConfig(config.get());

    config->batteryStateOfHealthPath = "/sys/class/oplus_chg/battery/battery_ui_soh";
    config->batteryCycleCountPath = "/sys/class/oplus_chg/battery/battery_ui_cc";
    config->batteryFirstUsageDatePath = "/sys/class/oplus_chg/battery/battery_first_usage_date";
    config->batteryManufacturingDatePath = "/sys/class/oplus_chg/battery/battery_manu_date";

    auto binder = ndk::SharedRefBase::make<Health>(gInstanceName, std::move(config));

    if (argc >= 2 && argv[1] == gChargerArg) {
        return ChargerModeMain(binder, std::make_shared<ChargerCallback>(binder));
    }

    auto hal_health_loop = std::make_shared<HalHealthLoop>(binder, binder);
    return hal_health_loop->StartLoop();
}
