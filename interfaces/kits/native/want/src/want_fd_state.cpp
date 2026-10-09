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

#include "want_fd_state.h"

#include "ability_base_log_wrapper.h"

#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

namespace OHOS {
namespace AAFwk {

std::shared_ptr<WantFdState> WantFdState::AdoptTagged(int fd)
{
    if (fd < 0) {
        return nullptr;
    }
    return std::make_shared<WantFdState>(fd, FdOwnership::TAKE_TAGGED, PrivateCtorToken {});
}

std::shared_ptr<WantFdState> WantFdState::LegacyExplicit(int fd)
{
    if (fd < 0) {
        return nullptr;
    }
    return std::make_shared<WantFdState>(fd, FdOwnership::LEGACY_EXPLICIT, PrivateCtorToken {});
}

bool WantFdState::IsFdAlive(int fd) noexcept
{
    return fd >= 0 && fcntl(fd, F_GETFD) >= 0;
}

int WantFdState::CloseWithTag(int fd, uint64_t tag) noexcept
{
    if (tag != 0) {
        return fdsan_close_with_tag(fd, tag);
    }
    return close(fd);
}

void WantFdState::AcquireFdTag() noexcept
{
    fdsanTag_ = FdsanTag();
    fdsan_exchange_owner_tag(fd_, 0, fdsanTag_);
}

void WantFdState::ReleaseFdTag() noexcept
{
    if (fdsanTag_ != 0) {
        fdsan_exchange_owner_tag(fd_, fdsanTag_, 0);
    }
    fdsanTag_ = 0;
}

WantFdState::WantFdState(int fd, FdOwnership ownership, PrivateCtorToken)
    : fd_(fd), ownership_(ownership)
{
    if (fd_ >= 0 && ownership_ == FdOwnership::TAKE_TAGGED) {
        AcquireFdTag();
    }
}

WantFdState::~WantFdState()
{
    if (fd_ >= 0 && ownership_ == FdOwnership::TAKE_TAGGED) {
        CloseWithTag(fd_, fdsanTag_);
    }
}

int WantFdState::DuplicateFd() const
{
    if (fd_ < 0) {
        return -1;
    }
    return dup(fd_);
}

FdCloseResult WantFdState::CloseOnce() noexcept
{
    if (fd_ < 0) {
        return FdCloseResult::SUCCESS;
    }
    int fd = fd_;
    uint64_t tag = fdsanTag_;
    fd_ = -1;
    fdsanTag_ = 0;
    int rc = CloseWithTag(fd, tag);
    invalidReason_ = (rc == 0) ? FdInvalidReason::FORCE_CLOSED : FdInvalidReason::CLOSE_FAILED;
    return (rc == 0) ? FdCloseResult::SUCCESS : FdCloseResult::FAILED;
}

int WantFdState::ReleaseOwned() noexcept
{
    if (fd_ < 0 || ownership_ != FdOwnership::TAKE_TAGGED) {
        return -1;
    }
    ReleaseFdTag();
    int fd = fd_;
    fd_ = -1;
    invalidReason_ = FdInvalidReason::RELEASED;
    return fd;
}

bool WantFdState::IsOwned() const noexcept
{
    return ownership_ == FdOwnership::TAKE_TAGGED;
}

bool WantFdState::UpgradeToOwned() noexcept
{
    if (ownership_ != FdOwnership::LEGACY_EXPLICIT || fd_ < 0) {
        return false;
    }
    if (!IsFdAlive(fd_)) {
        fd_ = -1;
        invalidReason_ = FdInvalidReason::FORCE_CLOSED;
        return false;
    }
    ownership_ = FdOwnership::TAKE_TAGGED;
    AcquireFdTag();
    return true;
}

int WantFdState::GetFdValue() const noexcept
{
    return fd_;
}

FdInvalidReason WantFdState::GetInvalidReason() const noexcept
{
    return invalidReason_;
}

FdOwnership WantFdState::GetOwnership() const noexcept
{
    return ownership_;
}

uint64_t WantFdState::FdsanTag() const noexcept
{
    return fdsan_create_owner_tag(FDSAN_OWNER_TYPE_UNIQUE_FD, ABILITYBASE_LOG_DOMAIN);
}

} // namespace AAFwk
} // namespace OHOS
