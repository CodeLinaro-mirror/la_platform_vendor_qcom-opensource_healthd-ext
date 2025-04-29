/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#include <memory>
#include <string_view>

#include <android/binder_interface_utils.h>
#include <health-impl/Health.h>
#include <health/utils.h>

using ::aidl::android::hardware::health::BatteryHealth;
using ::aidl::android::hardware::health::BatteryStatus;
using ::aidl::android::hardware::health::HalHealthLoop;
using ::aidl::android::hardware::health::Health;
using ::aidl::android::hardware::health::HealthInfo;
using ::aidl::android::hardware::health::IHealth;
using ::android::hardware::health::InitHealthdConfig;
using ::ndk::ScopedAStatus;
using ::ndk::SharedRefBase;
using namespace std::literals;

namespace aidl::android::hardware::health {

class HealthImpl : public Health {
  public:
    // Use default constructor from IHealth
    using Health::Health;
    virtual ~HealthImpl() {}

    ScopedAStatus getChargeCounterUah(int32_t* out) override;
    ScopedAStatus getCurrentNowMicroamps(int32_t* out) override;
    ScopedAStatus getCurrentAverageMicroamps(int32_t* out) override;
    ScopedAStatus getCapacity(int32_t* out) override;
    ScopedAStatus getChargeStatus(BatteryStatus* out) override;
    ScopedAStatus getEnergyCounterNwh(int64_t* out) override;

  protected:
    void UpdateHealthInfo(HealthInfo* health_info) override;
};

ScopedAStatus HealthImpl::getChargeCounterUah(int32_t* out) {
    *out = 10000;  // 10 Ah
    return ScopedAStatus::ok();
}

ScopedAStatus HealthImpl::getCurrentNowMicroamps(int32_t* out) {
    *out = 900000;  // 0.9A
    return ScopedAStatus::ok();
}

ScopedAStatus HealthImpl::getCurrentAverageMicroamps(int32_t* out) {
    *out = 900000;  // 0.9A
    return ScopedAStatus::ok();
}

ScopedAStatus HealthImpl::getCapacity(int32_t* out) {
    *out = 100;
    return ScopedAStatus::ok();
}

ScopedAStatus HealthImpl::getChargeStatus(BatteryStatus* out) {
    *out = BatteryStatus::CHARGING;
    return ScopedAStatus::ok();
}

ScopedAStatus HealthImpl::getEnergyCounterNwh(int64_t* out) {
    *out = LONG_MAX;
    return ScopedAStatus::ok();
}

void HealthImpl::UpdateHealthInfo(HealthInfo* health_info) {
    health_info->chargerAcOnline = true;
    health_info->batteryStatus = BatteryStatus::CHARGING;
    health_info->batteryLevel = 100;
    health_info->batteryChargeTimeToFullNowSeconds = 0;
}
}  // namespace aidl::android::hardware::health

int main(int /* argc */, char** /* argv */) {
    // Automotive does not support offline-charging mode, hence do not handle
    // --charger option.
    using aidl::android::hardware::health::HealthImpl;

    auto config = std::make_unique<healthd_config>();
    qti_healthd_board_init(config.get());
    ::android::hardware::health::InitHealthdConfig(config.get());
    auto binder = ndk::SharedRefBase::make<Health>(gInstanceName, std::move(config));

    if (argc >= 2 && argv[1] == gChargerArg) {
#if !CHARGER_FORCE_NO_UI
        KLOG_INFO(LOG_TAG, "Starting charger mode with UI.");
        auto charger_callback = std::make_shared<aidl::android::hardware::health::ChargerCallbackImpl>(binder);
        return ChargerModeMain(binder, charger_callback);
#endif
        KLOG_INFO(LOG_TAG, "Starting charger mode without UI.");
    } else {
        KLOG_INFO(LOG_TAG, "Starting health HAL.");
    }

    auto hal_health_loop = std::make_shared<HalHealthLoop>(binder, binder);
    return hal_health_loop->StartLoop();
}
