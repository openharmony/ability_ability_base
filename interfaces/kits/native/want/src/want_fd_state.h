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

#ifndef OHOS_ABILITY_BASE_WANT_FD_STATE_H
#define OHOS_ABILITY_BASE_WANT_FD_STATE_H

#include <cstdint>
#include <memory>

namespace OHOS {
namespace AAFwk {

enum class FdOwnership : uint8_t {
    TAKE_TAGGED = 0,
    LEGACY_EXPLICIT = 1,
};

enum class FdInvalidReason : uint8_t {
    NONE = 0,
    FORCE_CLOSED,
    CLOSE_FAILED,
    RELEASED,
};

enum class FdCloseResult : uint8_t {
    SUCCESS = 0,
    FAILED = 1,
};

enum class FdGetStatus : uint8_t {
    SUCCESS,
    NOT_FOUND,
    CLOSED,
    INVALID_MARKER,
};

enum class FdSetStatus : uint8_t {
    SUCCESS,
    INVALID_FD,
    INTERNAL_ERROR,
};

enum class FdCloseStatus : uint8_t {
    SUCCESS,
    NO_FDS,
    PARTIAL,
};

enum class FdRemoveStatus : uint8_t {
    SUCCESS,
    NO_FDS,
};

class WantFdState final {
public:
    static std::shared_ptr<WantFdState> AdoptTagged(int fd);
    static std::shared_ptr<WantFdState> LegacyExplicit(int fd);
    static bool IsFdAlive(int fd) noexcept;

    class PrivateCtorToken {
        PrivateCtorToken() = default;
        friend class WantFdState;
    };

    explicit WantFdState(int fd, FdOwnership ownership, PrivateCtorToken);

    ~WantFdState();
    WantFdState(const WantFdState &) = delete;
    WantFdState &operator=(const WantFdState &) = delete;

    int GetFdValue() const noexcept;

    FdInvalidReason GetInvalidReason() const noexcept;

    int DuplicateFd() const;
    FdCloseResult CloseOnce() noexcept;
    int ReleaseOwned() noexcept;
    bool IsOwned() const noexcept;
    FdOwnership GetOwnership() const noexcept;
    bool UpgradeToOwned() noexcept;

private:
    uint64_t FdsanTag() const noexcept;

    void AcquireFdTag() noexcept;

    void ReleaseFdTag() noexcept;

    static int CloseWithTag(int fd, uint64_t tag) noexcept;

    int fd_ = -1;
    FdOwnership ownership_ = FdOwnership::LEGACY_EXPLICIT;
    FdInvalidReason invalidReason_ = FdInvalidReason::NONE;
    uint64_t fdsanTag_ = 0;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_BASE_WANT_FD_STATE_H
