/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "wantparamsfd_fuzzer.h"

#include <cstdint>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>

#define private public
#include "array_wrapper.h"
#include "int_wrapper.h"
#include "parcel.h"
#include "string_wrapper.h"
#include "want_params.h"
#include "want_params_wrapper.h"
#undef private
#include "want_fd_state.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace {
constexpr size_t U32_AT_SIZE = 4;
constexpr int BITS_PER_BYTE = 8;
constexpr size_t NON_STRICT_EXTRA_LEN = 8;
constexpr int NON_NEGATIVE_FD_MASK = 0x7fffffff;
constexpr int DEPTH_BASE = 90;
constexpr int DEPTH_SPAN = 12;
constexpr int BRANCH_VALID_ROUND_TRIP = 0;
constexpr int BRANCH_INJECTED_STRICT_MARKER = 1;
constexpr int BRANCH_NON_STRICT_COMPAT = 2;
constexpr int BRANCH_TRUNCATED_PARCEL = 3;
constexpr int BRANCH_DEPTH_BOUNDARY = 4;
constexpr int BRANCH_ARRAY_ELEMENT_MARKER = 5;
constexpr int BRANCH_DUPLICATE_KEY_OVERWRITE = 6;
constexpr int BRANCH_GETFD_PROBES = 7;
constexpr int BRANCH_COUNT = BRANCH_GETFD_PROBES + 1;
constexpr uint8_t STRICT_BIT_MASK = 0x01;
constexpr const char *DEV_NULL = "/dev/null";

uint32_t GetU32Data(const char *ptr)
{
    // convert fuzz input data to an integer (big-endian byte order)
    uint32_t result = 0;
    for (size_t i = 0; i < U32_AT_SIZE; ++i) {
        result = (result << BITS_PER_BYTE) | static_cast<uint8_t>(ptr[i]);
    }
    return result;
}

void RecycleFds(WantParams *wp, WantParams *parsed)
{
    if (wp != nullptr) {
        wp->CloseAllFdWithStatus(FdTraversalMode::RECURSIVE);
    }
    if (parsed != nullptr) {
        parsed->CloseAllFdWithStatus(FdTraversalMode::RECURSIVE);
        delete parsed;
    }
}

void FuzzValidRoundTrip(const char *data, size_t size)
{
    int fd = open(DEV_NULL, O_RDONLY);
    if (fd < 0) {
        return;
    }
    WantParams wp;
    wp.SetParam("name", String::Box(std::string(data, size)));
    if (wp.SetFd("img", fd, FdOwnership::LEGACY_EXPLICIT) != FdSetStatus::SUCCESS) {
        close(fd);
        return;
    }
    Parcel parcel;
    if (wp.Marshalling(parcel) && parcel.RewindRead(0)) {
        WantParams *parsed = WantParams::Unmarshalling(parcel);
        RecycleFds(nullptr, parsed);
    }
    RecycleFds(&wp, nullptr);
}

void FuzzInjectedStrictMarker(const char *data, size_t size)
{
    WantParams marker;
    marker.SetParam("type", String::Box("FD"));
    int injected = static_cast<int>(GetU32Data(data) & NON_NEGATIVE_FD_MASK);
    marker.SetParam("value", Integer::Box(injected));
    WantParams wp;
    wp.SetParam("fd", WantParamWrapper::Box(std::move(marker)));
    Parcel parcel;
    if (wp.Marshalling(parcel) && parcel.RewindRead(0)) {
        WantParams *parsed = WantParams::Unmarshalling(parcel);
        RecycleFds(nullptr, parsed);
    }
    RecycleFds(&wp, nullptr);
}

void FuzzNonStrictMarkerCompat(const char *data, size_t size)
{
    WantParams marker;
    marker.SetParam("type", String::Box("FD"));
    marker.SetParam("value", Integer::Box(static_cast<int>(GetU32Data(data) & NON_NEGATIVE_FD_MASK)));
    marker.SetParam("extra", String::Box(std::string(data, size % NON_STRICT_EXTRA_LEN)));
    WantParams wp;
    wp.SetParam("fd", WantParamWrapper::Box(std::move(marker)));
    Parcel parcel;
    if (wp.Marshalling(parcel) && parcel.RewindRead(0)) {
        WantParams *parsed = WantParams::Unmarshalling(parcel);
        RecycleFds(nullptr, parsed);
    }
    RecycleFds(&wp, nullptr);
}

void FuzzTruncatedParcel(const char *data, size_t size)
{
    int fd = open(DEV_NULL, O_RDONLY);
    if (fd < 0) {
        return;
    }
    WantParams wp;
    wp.SetParam("name", String::Box("v"));
    if (wp.SetFd("img", fd, FdOwnership::LEGACY_EXPLICIT) != FdSetStatus::SUCCESS) {
        close(fd);
        return;
    }
    Parcel parcel;
    if (wp.Marshalling(parcel)) {
        auto *raw = reinterpret_cast<const uint8_t *>(parcel.GetData());
        size_t full = parcel.GetDataSize();
        size_t truncated = full;
        if (size >= U32_AT_SIZE) {
            truncated = GetU32Data(data) % (full + 1);
        }
        Parcel cut;
        if (raw != nullptr && truncated > 0 && cut.WriteBuffer(raw, truncated)) {
            WantParams *parsed = WantParams::Unmarshalling(cut);
            RecycleFds(nullptr, parsed);
        }
    }
    RecycleFds(&wp, nullptr);
}

void FuzzDepthBoundary(const char *data, size_t size)
{
    int depth = DEPTH_BASE + static_cast<int>(size % DEPTH_SPAN); // 90..101
    WantParams current;
    current.SetParam("leaf", String::Box("v"));
    for (int i = 0; i < depth; ++i) {
        WantParams outer;
        outer.SetParam("n", WantParamWrapper::Box(std::move(current)));
        current = std::move(outer);
    }
    Parcel parcel;
    if (current.Marshalling(parcel) && parcel.RewindRead(0)) {
        WantParams *parsed = WantParams::Unmarshalling(parcel);
        RecycleFds(nullptr, parsed);
    }
    RecycleFds(&current, nullptr);
}

void FuzzArrayElementMarker(const char *data, size_t size)
{
    bool strict = (data[0] & STRICT_BIT_MASK) != 0;
    WantParams elem;
    elem.SetParam("type", String::Box("FD"));
    if (strict) {
        elem.SetParam("value", Integer::Box(static_cast<int>(GetU32Data(data) & NON_NEGATIVE_FD_MASK)));
    }
    sptr<IWantParams> boxed = WantParamWrapper::Box(std::move(elem));
    if (boxed == nullptr) {
        return;
    }
    sptr<IArray> arr = new (std::nothrow) Array(1, g_IID_IWantParams);
    if (arr == nullptr) {
        return;
    }
    arr->Set(0, boxed.GetRefPtr());
    WantParams wp;
    wp.SetParam("arr", arr.GetRefPtr());
    Parcel parcel;
    if (wp.Marshalling(parcel) && parcel.RewindRead(0)) {
        WantParams *parsed = WantParams::Unmarshalling(parcel);
        RecycleFds(nullptr, parsed);
    }
    RecycleFds(&wp, nullptr);
}

void FuzzDuplicateKeyOverwrite(const char *data, size_t size)
{
    int fd = open(DEV_NULL, O_RDONLY);
    if (fd < 0) {
        return;
    }
    WantParams wp;
    if (wp.SetFd("img", fd, FdOwnership::LEGACY_EXPLICIT) != FdSetStatus::SUCCESS) {
        close(fd);
        return;
    }
    (void)wp.SetFd("img", static_cast<int>(GetU32Data(data) & NON_NEGATIVE_FD_MASK), FdOwnership::LEGACY_EXPLICIT);
    Parcel parcel;
    if (wp.Marshalling(parcel) && parcel.RewindRead(0)) {
        WantParams *parsed = WantParams::Unmarshalling(parcel);
        RecycleFds(nullptr, parsed);
    }
    RecycleFds(&wp, nullptr);
}
void FuzzGetFdProbes(const char *data, size_t size)
{
    WantParams marker;
    marker.SetParam("type", String::Box("FD"));
    marker.SetParam("value", Integer::Box(static_cast<int>(GetU32Data(data) & NON_NEGATIVE_FD_MASK)));
    WantParams wp;
    wp.SetParam("fd", WantParamWrapper::Box(std::move(marker)));
    int got = -1;
    (void)wp.GetFd("fd", got);

    WantParams plain;
    plain.SetParam("note", String::Box(std::string(data, size)));
    int plainGot = -1;
    (void)plain.GetFd("note", plainGot);

    int fd = open(DEV_NULL, O_RDONLY);
    if (fd < 0) {
        return;
    }
    WantParams managed;
    if (managed.SetFd("img", fd, FdOwnership::LEGACY_EXPLICIT) != FdSetStatus::SUCCESS) {
        close(fd);
        return;
    }
    close(fd); // external raw close before probing
    int probed = -1;
    (void)managed.GetFd("img", probed);
    RecycleFds(&managed, nullptr);
}
} // namespace
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }
    const char *ch = reinterpret_cast<const char *>(data);
    switch (ch[0] % OHOS::BRANCH_COUNT) {
        case OHOS::BRANCH_VALID_ROUND_TRIP:
            OHOS::FuzzValidRoundTrip(ch, size);
            break;
        case OHOS::BRANCH_INJECTED_STRICT_MARKER:
            OHOS::FuzzInjectedStrictMarker(ch, size);
            break;
        case OHOS::BRANCH_NON_STRICT_COMPAT:
            OHOS::FuzzNonStrictMarkerCompat(ch, size);
            break;
        case OHOS::BRANCH_TRUNCATED_PARCEL:
            OHOS::FuzzTruncatedParcel(ch, size);
            break;
        case OHOS::BRANCH_DEPTH_BOUNDARY:
            OHOS::FuzzDepthBoundary(ch, size);
            break;
        case OHOS::BRANCH_ARRAY_ELEMENT_MARKER:
            OHOS::FuzzArrayElementMarker(ch, size);
            break;
        case OHOS::BRANCH_GETFD_PROBES:
            OHOS::FuzzGetFdProbes(ch, size);
            break;
        case OHOS::BRANCH_DUPLICATE_KEY_OVERWRITE:
        default:
            OHOS::FuzzDuplicateKeyOverwrite(ch, size);
            break;
    }
    return 0;
}
