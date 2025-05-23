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

#include "arraywrapperthird_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#define private public
#include "array_wrapper.h"
#undef private
#include "securec.h"
#include "string_wrapper.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
constexpr char LEFT_BRACE_STRING = '{';
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    long longSize = 0;
    InterfaceID id;
    std::shared_ptr<Array> array = std::make_shared<Array>(longSize, id);
    std::string values(data, size);
    array->ParseDouble(values, longSize);
    array->ParseChar(values, longSize);
    array->ParseArray(values, longSize);
    array->ParseWantParams(values, longSize);
    IArray* arrayptr = nullptr;
    std::function<sptr<IInterface>(std::string)> func;
    array->ParseElement(arrayptr, func, values, longSize);
    std::string errorString(data, longSize);
    array->Parse(errorString);
    errorString.insert(errorString.begin(), String::SIGNATURE);
    array->Parse(errorString);
    errorString.insert(errorString.begin(), LEFT_BRACE_STRING);
    array->Parse(errorString);
    long lengthSize = (long)(size);
    array->ParseElement(arrayptr, func, values, lengthSize);
    std::shared_ptr<Array> otherArray = std::make_shared<Array>(longSize, id);
    sptr<IInterface> stringValue = String::Box(values);
    for (size_t i = 0; i < longSize; i++) {
        otherArray->Set(i, stringValue);
    }
    array->IsStringArray(otherArray.get());
    std::function<void(IInterface*)> function;
    array->ForEach(otherArray.get(), function);
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

    char* ch = (char *)malloc(size + 1);
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
