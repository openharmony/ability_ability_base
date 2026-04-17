/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "wantthird_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#define private public
#include "want.h"
#undef private
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;

void TryParseWantFromString(const std::string &jsonInput)
{
    std::string json = jsonInput;
    Want *parsedWant = Want::FromString(json);
    if (parsedWant != nullptr) {
        delete parsedWant;
        parsedWant = nullptr;
    }
}
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

void DoReadFromJsonFirst()
{
    std::string noBundleName = "{"
        "\"deviceId\": \"teset_deviceId\""
    "}";

    std::string noAbilityName = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\""
    "}";

    std::string noUri = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\""
    "}";

    std::string noType = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\""
    "}";

    std::string noFlags = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\""
    "}";

    std::string noAction = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\","
        "\"flags\": \"teset_flags\""
    "}";
    TryParseWantFromString(noBundleName);
    TryParseWantFromString(noAbilityName);
    TryParseWantFromString(noUri);
    TryParseWantFromString(noType);
    TryParseWantFromString(noFlags);
    TryParseWantFromString(noAction);
}

void DoReadFromJsonSecond()
{
    std::string noParameters = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\","
        "\"flags\": \"teset_flags\","
        "\"action\": \"teset_action\","
        "\"entities\": \"teset_entities\""
    "}";
    std::string noEntities = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\","
        "\"flags\": \"teset_flags\","
        "\"action\": \"teset_action\","
        "\"parameters\": \"teset_parameters\""
    "}";
    std::string noStringDeviceId = "{"
        "\"deviceId\": {},"
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\","
        "\"flags\": \"teset_flags\","
        "\"action\": \"teset_action\","
        "\"parameters\": \"teset_parameters\","
        "\"entities\": \"teset_entities\""
    "}";
    std::string noStringBundleName = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": {},"
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\","
        "\"flags\": \"teset_flags\","
        "\"action\": \"teset_action\","
        "\"parameters\": \"teset_parameters\","
        "\"entities\": \"teset_entities\""
    "}";
    TryParseWantFromString(noParameters);
    TryParseWantFromString(noEntities);
    TryParseWantFromString(noStringDeviceId);
    TryParseWantFromString(noStringBundleName);
}

void DoReadFromJsonThird()
{
    std::string noStringAbilityName = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": {},"
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\","
        "\"flags\": \"teset_flags\","
        "\"action\": \"teset_action\","
        "\"parameters\": \"teset_parameters\","
        "\"entities\": \"teset_entities\""
    "}";

    std::string noStringUri = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": {},"
        "\"type\": \"teset_type\","
        "\"flags\": \"teset_flags\","
        "\"action\": \"teset_action\","
        "\"parameters\": \"teset_parameters\","
        "\"entities\": \"teset_entities\""
    "}";

    std::string noStringType = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": {},"
        "\"flags\": \"teset_flags\","
        "\"action\": \"teset_action\","
        "\"parameters\": \"teset_parameters\","
        "\"entities\": \"teset_entities\""
    "}";

    TryParseWantFromString(noStringAbilityName);
    TryParseWantFromString(noStringUri);
    TryParseWantFromString(noStringType);
}

void DoReadFromJsonFourth()
{
    std::string noStringFlags = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\","
        "\"flags\": {},"
        "\"action\": \"teset_action\","
        "\"parameters\": \"teset_parameters\","
        "\"entities\": \"teset_entities\""
    "}";

    std::string noStringAction = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\","
        "\"flags\": \"teset_flags\","
        "\"action\": {},"
        "\"parameters\": \"teset_parameters\","
        "\"entities\": \"teset_entities\""
    "}";

    std::string noStringParameters = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\","
        "\"flags\": \"teset_flags\","
        "\"action\": \"teset_action\","
        "\"parameters\": {},"
        "\"entities\": \"teset_entities\""
    "}";

    TryParseWantFromString(noStringFlags);
    TryParseWantFromString(noStringAction);
    TryParseWantFromString(noStringParameters);
}

void DoReadFromJsonFifth()
{
    std::string nullEntities = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\","
        "\"flags\": \"teset_flags\","
        "\"action\": \"teset_action\","
        "\"parameters\": \"teset_parameters\","
        "\"entities\": null"
    "}";

    std::string arraryEntitiesItemNotString = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\","
        "\"flags\": \"teset_flags\","
        "\"action\": \"teset_action\","
        "\"parameters\": \"teset_parameters\","
        "\"entities\": ["
            "{},"
        "]"
    "}";

    std::string stringJson = "{"
        "\"deviceId\": \"teset_deviceId\","
        "\"bundleName\": \"teset_bundleName\","
        "\"abilityName\": \"teset_abilityName\","
        "\"uri\": \"teset_uri\","
        "\"type\": \"teset_type\","
        "\"flags\": \"teset_flags\","
        "\"action\": \"teset_action\","
        "\"parameters\": \"teset_parameters\","
        "\"entities\": ["
            "\"entities\""
        "]"
    "}";

    TryParseWantFromString(nullEntities);
    TryParseWantFromString(arraryEntitiesItemNotString);
    TryParseWantFromString(stringJson);
}

void DoReadFromJson()
{
    DoReadFromJsonFirst();
    DoReadFromJsonSecond();
    DoReadFromJsonThird();
    DoReadFromJsonFourth();
    DoReadFromJsonFifth();
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    want->CountEntities();
    Want *wantptr = nullptr;
    want->GetScheme();
    AppExecFwk::ElementName elementName;
    wantptr = want->MakeMainAbility(elementName);
    if (wantptr) {
        delete wantptr;
        wantptr = nullptr;
    }
    Operation operation;
    want->SetOperation(operation);
    wantptr = want->CloneOperation();
    if (wantptr) {
        delete wantptr;
        wantptr = nullptr;
    }
    std::string key(data, size);
    want->HasParameter(key);
    std::string content(data, size);
    std::string prop(data, size);
    std::string value(data, size);
    want->ParseContent(content, prop, value);
    std::string str(data, size);
    want->Decode(str);
    want->Encode(str);
    int level = static_cast<int>(GetU32Data(data));
    want->DumpInfo(level);
    std::string jsonStr(data, size);
    size_t pos = jsonStr.find("\"jsonStr\"");
    if (pos != std::string::npos) {
        std::string jsonString = jsonStr.substr(pos + 8);
        TryParseWantFromString(jsonString);
    }
    DoReadFromJson();
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = reinterpret_cast<char *>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
