/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "configuration_convertor.h"

#include "ability_base_log_wrapper.h"
#include "configuration.h"
namespace OHOS::AppExecFwk {
constexpr float DPI_BASE = 160.0;

Global::Resource::ColorMode ConvertColorMode(std::string colormode)
{
    auto resolution = Global::Resource::ColorMode::COLOR_MODE_NOT_SET;

    static const std::vector<std::pair<std::string, Global::Resource::ColorMode>> resolutions = {
        { "dark", Global::Resource::ColorMode::DARK },
        { "light", Global::Resource::ColorMode::LIGHT },
    };

    for (const auto& [tempColorMode, value] : resolutions) {
        if (tempColorMode == colormode) {
            resolution = value;
            break;
        }
    }

    return resolution;
}

Global::Resource::TimeFormat ConvertTimeFormat(std::string timeformat)
{
    auto resolution = Global::Resource::TimeFormat::HOUR_NOT_SET;

    static const std::vector<std::pair<std::string, Global::Resource::TimeFormat>> resolutions = {
        { "false", Global::Resource::TimeFormat::HOUR_12 },
        { "true", Global::Resource::TimeFormat::HOUR_24 },
    };

    for (const auto& [tempTimeFormat, value] : resolutions) {
        if (tempTimeFormat == timeformat) {
            resolution = value;
            break;
        }
    }

    return resolution;
}

Global::Resource::Direction ConvertDirection(int32_t height, int32_t width)
{
    return height >= width ? Global::Resource::Direction::DIRECTION_VERTICAL :
        Global::Resource::Direction::DIRECTION_HORIZONTAL;
}

Global::Resource::Direction ConvertDirection(std::string direction)
{
    auto resolution = Global::Resource::Direction::DIRECTION_NOT_SET;

    static const std::vector<std::pair<std::string, Global::Resource::Direction>> resolutions = {
        { "vertical", Global::Resource::Direction::DIRECTION_VERTICAL },
        { "horizontal", Global::Resource::Direction::DIRECTION_HORIZONTAL },
    };

    for (const auto& [tempDirection, value] : resolutions) {
        if (tempDirection == direction) {
            resolution = value;
            break;
        }
    }

    return resolution;
}

Global::Resource::ScreenDensity ConvertDensity(float density)
{
    static const std::vector<std::pair<float, Global::Resource::ScreenDensity>> resolutions = {
        { 0.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_NOT_SET },
        { 120.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_SDPI },
        { 160.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_MDPI },
        { 240.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_LDPI },
        { 320.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_XLDPI },
        { 480.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_XXLDPI },
        { 640.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_XXXLDPI },
    };

    float deviceDpi = density * DPI_BASE;
    auto resolution = Global::Resource::ScreenDensity::SCREEN_DENSITY_NOT_SET;
    for (const auto& [dpi, value] : resolutions) {
        resolution = value;
        if (deviceDpi <= dpi) {
            break;
        }
    }
    return resolution;
}

Global::Resource::ScreenDensity ConvertDensity(std::string density)
{
    auto resolution = Global::Resource::ScreenDensity::SCREEN_DENSITY_NOT_SET;

    static const std::vector<std::pair<std::string, Global::Resource::ScreenDensity>> resolutions = {
        { "sdpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_SDPI },
        { "mdpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_MDPI },
        { "ldpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_LDPI },
        { "xldpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_XLDPI },
        { "xxldpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_XXLDPI },
        { "xxxldpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_XXXLDPI },
    };

    for (const auto& [tempdensity, value] : resolutions) {
        if (tempdensity == density) {
            resolution = value;
            break;
        }
    }

    return resolution;
}

int32_t ConvertDisplayId(std::string displayId)
{
    if (displayId == ConfigurationInner::EMPTY_STRING) {
        return -1;
    }

    int32_t id = -1;
    try {
        id = std::stoi(displayId);
    } catch (...) {
        ABILITYBASE_LOGE("failed to convert to int: %{public}s", displayId.c_str());
    }
    return id;
}

Global::Resource::InputDevice ConvertHasPointerDevice(std::string hasPointerDevice)
{
    return hasPointerDevice == "true" ? Global::Resource::InputDevice::INPUTDEVICE_POINTINGDEVICE :
        Global::Resource::InputDevice::INPUTDEVICE_NOT_SET;
}

Global::Resource::DeviceType ConvertDeviceType(std::string deviceType)
{
    static const std::unordered_map<std::string, Global::Resource::DeviceType> deviceTypes = {
        {"default", Global::Resource::DeviceType::DEVICE_PHONE},
        {"phone", Global::Resource::DeviceType::DEVICE_PHONE},
        {"tablet", Global::Resource::DeviceType::DEVICE_TABLET},
        {"car", Global::Resource::DeviceType::DEVICE_CAR},
        {"tv", Global::Resource::DeviceType::DEVICE_TV},
        {"watch", Global::Resource::DeviceType::DEVICE_WEARABLE},
        {"2in1", Global::Resource::DeviceType::DEVICE_TWOINONE},
        {"wearable", Global::Resource::DeviceType::DEVICE_WEARABLE}
    };

    if (deviceTypes.find(deviceType) != deviceTypes.end()) {
        return deviceTypes.at(deviceType);
    }

    return Global::Resource::DeviceType::DEVICE_PHONE;
}

std::string GetColorModeStr(int32_t colormode)
{
    std::string ret("no_color_mode");

    switch (colormode) {
        case Global::Resource::ColorMode::DARK:
            ret = ConfigurationInner::COLOR_MODE_DARK;
            break;
        case Global::Resource::ColorMode::LIGHT:
            ret = ConfigurationInner::COLOR_MODE_LIGHT;
            break;
        case Global::Resource::ColorMode::COLOR_MODE_NOT_SET:
            ret = ConfigurationInner::COLOR_MODE_AUTO;
            break;
        default:
            break;
    }

    return ret;
}

std::string GetDirectionStr(Global::Resource::Direction direction)
{
    std::string ret("no_direction");

    switch (direction) {
        case Global::Resource::Direction::DIRECTION_VERTICAL:
            ret = ConfigurationInner::DIRECTION_VERTICAL;
            break;
        case Global::Resource::Direction::DIRECTION_HORIZONTAL:
            ret = ConfigurationInner::DIRECTION_HORIZONTAL;
            break;
        default:
            break;
    }

    return ret;
}

std::string GetDirectionStr(int32_t height, int32_t width)
{
    return GetDirectionStr(ConvertDirection(height, width));
}

std::string GetDirectionStr(int32_t orientation)
{
    std::string ret("no_direction");

    static const std::vector<std::pair<int32_t, std::string>> resolutions = {
        { 0, ConfigurationInner::DIRECTION_VERTICAL },
        { 1, ConfigurationInner::DIRECTION_HORIZONTAL },
        { 2, ConfigurationInner::DIRECTION_VERTICAL },
        { 3, ConfigurationInner::DIRECTION_HORIZONTAL },
    };

    for (const auto& [tempOrientation, value] : resolutions) {
        if (tempOrientation == orientation) {
            ret = value;
            break;
        }
    }
    return ret;
}

std::string GetDensityStr(Global::Resource::ScreenDensity density)
{
    std::string ret("no_screen_density");

    static const std::vector<std::pair<Global::Resource::ScreenDensity, std::string>> resolutions = {
        { Global::Resource::ScreenDensity::SCREEN_DENSITY_SDPI, "sdpi" },
        { Global::Resource::ScreenDensity::SCREEN_DENSITY_MDPI, "mdpi" },
        { Global::Resource::ScreenDensity::SCREEN_DENSITY_LDPI, "ldpi" },
        { Global::Resource::ScreenDensity::SCREEN_DENSITY_XLDPI, "xldpi" },
        { Global::Resource::ScreenDensity::SCREEN_DENSITY_XXLDPI, "xxldpi" },
        { Global::Resource::ScreenDensity::SCREEN_DENSITY_XXXLDPI, "xxxldpi" },
    };

    for (const auto& [dpi, value] : resolutions) {
        if (dpi == density) {
            return value;
        }
    }

    return ret;
}

std::string GetDensityStr(float density)
{
    return GetDensityStr(ConvertDensity(density));
}

Global::Resource::ColorMode DarkMode_ConvertEts2Native(const int32_t index)
{
    return static_cast<Global::Resource::ColorMode>(index - 1);
}
int32_t DarkMode_ConvertNative2Ets(const Global::Resource::ColorMode nativeValue)
{
    return nativeValue + 1;
}

// enum Direction {
//     DIRECTION_NOT_SET = -1,
//     DIRECTION_VERTICAL = 0,
//     DIRECTION_HORIZONTAL = 1
//   }
Global::Resource::Direction Direction_ConvertEts2Native(const int32_t index)
{
    return static_cast<Global::Resource::Direction>(index - 1);
}
int32_t Direction_ConvertNative2Ets(const Global::Resource::Direction nativeValue)
{
    return nativeValue + 1;
}

// enum ScreenDensity {
//     SCREEN_DENSITY_NOT_SET = 0,
//     SCREEN_DENSITY_SDPI = 120,
//     SCREEN_DENSITY_MDPI = 160,
//     SCREEN_DENSITY_LDPI = 240,
//     SCREEN_DENSITY_XLDPI = 320,
//     SCREEN_DENSITY_XXLDPI = 480,
//     SCREEN_DENSITY_XXXLDPI = 640
//   }
static constexpr std::array<int, 7> ScreenDensityArray = {0, 120, 160, 240, 320, 480, 640};
Global::Resource::ScreenDensity ScreenDensity_ConvertEts2Native(const int32_t index)
{
    if (index < 0 || index >= ScreenDensityArray.size()) {
        return Global::Resource::ScreenDensity::SCREEN_DENSITY_NOT_SET;
    }
    return static_cast<Global::Resource::ScreenDensity>(ScreenDensityArray[index]);
}
int32_t ScreenDensity_ConvertNative2Ets(const Global::Resource::ScreenDensity nativeValue)
{
    for (int32_t index = 0; index < static_cast<int32_t>(ScreenDensityArray.size()); index++) {
        if (nativeValue == ScreenDensityArray[index]) {
            return index;
        }
    }
    return 0;
}
} // namespace OHOS::AppExecFwk