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

#include "wantfourth_fuzzer.h"

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
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key(data, size);
    sptr<IRemoteObject> remoteObject;
    want->SetParam(key, remoteObject);
    want->GetRemoteObject(key);
    std::vector<bool> boolValue;
    want->SetParam(key, boolValue);
    want->GetBoolArrayParam(key);
    byte byteValue = '\0';
    want->SetParam(key, byteValue);
    want->GetByteParam(key, byteValue);
    std::vector<byte> byteVector;
    want->SetParam(key, byteVector);
    want->GetByteArrayParam(key);
    zchar charValue = U'\0';
    want->SetParam(key, charValue);
    want->GetCharParam(key, charValue);
    Parcel parcel;
    want->Marshalling(parcel);
    want->ReadFromParcel(parcel);
    auto wantPtr = want->Unmarshalling(parcel);
    if (wantPtr) {
        delete wantPtr;
        wantPtr = nullptr;
    }
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
