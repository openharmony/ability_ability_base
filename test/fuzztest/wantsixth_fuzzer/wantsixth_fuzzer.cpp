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

#include "wantsixth_fuzzer.h"

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
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

void DoGetTypeArrayParam()
{
    std::shared_ptr<Want> WantIn = std::make_shared<Want>();
    std::vector<bool> boolArrayValue = {true, false, true};
    WantIn->SetParam(std::string("bool_arraykey"), boolArrayValue);

    std::vector<byte> byteArrayValue = {'?', 'a', '\\'};
    WantIn->SetParam(std::string("byte_arraykey"), byteArrayValue);

    std::vector<zchar> charArrayValue = {U'e', U'l', U'l', U'o'};
    WantIn->SetParam(std::string("char_arraykey"), charArrayValue);

    std::vector<int> intArrayValue = {-10, 0, 10};
    WantIn->SetParam(std::string("int_arraykey"), intArrayValue);

    std::vector<double> doubleArrayValue = {-1000.1, 0.1, 1000.1};
    WantIn->SetParam(std::string("double_arraykey"), doubleArrayValue);

    std::vector<float> floatArrayValue = {-100.1, 0.1, 100.1};
    WantIn->SetParam(std::string("float_arraykey"), floatArrayValue);

    std::vector<long> longArrayValue = {-100, 0, 100};
    WantIn->SetParam(std::string("long_arraykey"), longArrayValue);

    std::vector<std::string> stringArrayValue = {"stringtest1", "string@test2", "string@!#test2"};
    WantIn->SetParam(std::string("string_arraykey"), stringArrayValue);

    std::vector<short> shortArrayValue = {-1, 0, 1};
    WantIn->SetParam(std::string("short_arraykey"), shortArrayValue);

    WantIn->GetBoolArrayParam(std::string("bool_arraykey"));
    WantIn->GetByteArrayParam(std::string("byte_arraykey"));
    WantIn->GetCharArrayParam(std::string("char_arraykey"));
    WantIn->GetIntArrayParam(std::string("int_arraykey"));
    WantIn->GetDoubleArrayParam(std::string("double_arraykey"));
    WantIn->GetFloatArrayParam(std::string("float_arraykey"));
    WantIn->GetLongArrayParam(std::string("long_arraykey"));
    WantIn->GetLongArrayParam(std::string("string_arraykey"));
    WantIn->GetShortArrayParam(std::string("short_arraykey"));
    WantIn->GetStringArrayParam(std::string("string_arraykey"));
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key(data, size);
    std::vector<float> floatVector;
    want->SetParam(key, floatVector);
    want->GetFloatArrayParam(key);
    std::vector<long> longVector;
    want->SetParam(key, longVector);
    want->GetLongArrayParam(key);
    short shortValue = 0;
    want->SetParam(key, shortValue);
    want->GetShortParam(key, shortValue);
    std::vector<short> shortVector;
    want->SetParam(key, shortVector);
    want->GetShortArrayParam(key);
    std::vector<std::string> stringVector;
    want->SetParam(key, stringVector);
    Want wantSource;
    Want wantDest(wantSource);
    wantDest = wantSource;
    want->GetBoolArrayParam(key);
    DoGetTypeArrayParam();
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
