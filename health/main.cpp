/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <limits>
#include <string>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>
#include <android/binder_interface_utils.h>
#include <health-impl/ChargerUtils.h>
#include <health-impl/HalHealthLoop.h>
#include <health-impl/Health.h>
#include <health/utils.h>

using aidl::android::hardware::health::HalHealthLoop;
using aidl::android::hardware::health::Health;
using aidl::android::hardware::health::HealthInfo;
using aidl::android::hardware::health::charger::ChargerCallback;
using aidl::android::hardware::health::charger::ChargerModeMain;

static constexpr const char* gInstanceName = "default";
static constexpr std::string_view gChargerArg{"--charger"};

class OplusHealth : public Health {
  public:
    using Health::Health;

  protected:
    void UpdateHealthInfo(HealthInfo* info) override {
        Health::UpdateHealthInfo(info);
        // OPlus reports learned full-charge capacity in mAh, while HealthInfo uses uAh.
        std::string value;
        int32_t capacityMah;
        if (::android::base::ReadFileToString(
                    "/sys/class/oplus_chg/battery/battery_fcc", &value) &&
            ::android::base::ParseInt(::android::base::Trim(value), &capacityMah,
                                     int32_t{1}, std::numeric_limits<int32_t>::max() / 1000)) {
            info->batteryFullChargeUah = capacityMah * 1000;
        }
    }
};

int main(int argc, char** argv) {
    auto config = std::make_unique<healthd_config>();
    ::android::hardware::health::InitHealthdConfig(config.get());

    config->batteryStateOfHealthPath = "/sys/class/oplus_chg/battery/battery_ui_soh";
    config->batteryCycleCountPath = "/sys/class/power_supply/battery/cycle_count";
    config->batteryFirstUsageDatePath = "/sys/class/oplus_chg/battery/battery_first_usage_date";
    config->batteryManufacturingDatePath = "/sys/class/oplus_chg/battery/battery_manu_date";

    auto binder = ndk::SharedRefBase::make<OplusHealth>(gInstanceName, std::move(config));

    if (argc >= 2 && argv[1] == gChargerArg) {
        return ChargerModeMain(binder, std::make_shared<ChargerCallback>(binder));
    }

    auto hal_health_loop = std::make_shared<HalHealthLoop>(binder, binder);
    return hal_health_loop->StartLoop();
}
