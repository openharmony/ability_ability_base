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

#include <fcntl.h>
#include <unistd.h>

#include <gtest/gtest.h>

#define private public
#include "want_params.h"
#include "want_params_wrapper.h"
#undef private
#include "string_wrapper.h"
#include "int_wrapper.h"
#include "want_fd_state.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char *DEV_NULL = "/dev/null";
constexpr const char *FD_KEY = "test_fd";

int OpenFd()
{
    return open(DEV_NULL, O_RDONLY);
}
} // namespace

class WantFdStateTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WantFdStateTest::SetUpTestCase(void)
{}

void WantFdStateTest::TearDownTestCase(void)
{}

void WantFdStateTest::SetUp(void)
{}

void WantFdStateTest::TearDown(void)
{}

/**
 * @tc.number: WantFdState_SetFd_TakeTagged_001
 * @tc.name: SetFd TAKE_TAGGED
 * @tc.desc: TAKE_TAGGED adopts the fd; GetFd returns the same fd and CloseAllFd closes it.
 */
HWTEST_F(WantFdStateTest, WantFdState_SetFd_TakeTagged_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    WantParams wp;
    EXPECT_EQ(wp.SetFd(FD_KEY, fd, FdOwnership::TAKE_TAGGED), FdSetStatus::SUCCESS);
    int got = -1;
    EXPECT_EQ(wp.GetFd(FD_KEY, got), FdGetStatus::SUCCESS);
    EXPECT_EQ(got, fd);
    EXPECT_NE(fcntl(fd, F_GETFD), -1);
    wp.CloseAllFd();
    EXPECT_EQ(fcntl(fd, F_GETFD), -1);
}

/**
 * @tc.number: WantFdState_GetFd_Closed_001
 * @tc.name: GetFd CLOSED
 * @tc.desc: After closing an owned fd, GetFd returns CLOSED instead of the stale marker value.
 */
HWTEST_F(WantFdStateTest, WantFdState_GetFd_Closed_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    WantParams wp;
    ASSERT_EQ(wp.SetFd(FD_KEY, fd, FdOwnership::TAKE_TAGGED), FdSetStatus::SUCCESS);
    int got = -1;
    ASSERT_EQ(wp.GetFd(FD_KEY, got), FdGetStatus::SUCCESS);
    EXPECT_EQ(wp.CloseAllFdWithStatus(FdTraversalMode::CURRENT_LEVEL), FdCloseStatus::SUCCESS);
    EXPECT_EQ(wp.GetFd(FD_KEY, got), FdGetStatus::NOT_FOUND);
}

/**
 * @tc.number: WantFdState_ShareOwnership_001
 * @tc.name: Share via operator=
 * @tc.desc: operator= shares the state; RemoveAllFd on one copy keeps the fd valid for the other.
 */
HWTEST_F(WantFdStateTest, WantFdState_ShareOwnership_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    WantParams a;
    ASSERT_EQ(a.SetFd(FD_KEY, fd, FdOwnership::TAKE_TAGGED), FdSetStatus::SUCCESS);
    int aFd = -1;
    ASSERT_EQ(a.GetFd(FD_KEY, aFd), FdGetStatus::SUCCESS);
    WantParams b;
    b = a;
    a.RemoveAllFd();
    EXPECT_NE(fcntl(aFd, F_GETFD), -1);
    int bFd = -1;
    EXPECT_EQ(b.GetFd(FD_KEY, bFd), FdGetStatus::SUCCESS);
    EXPECT_EQ(bFd, aFd);
    b.CloseAllFd();
    EXPECT_EQ(fcntl(aFd, F_GETFD), -1);
}

/**
 * @tc.number: WantFdState_SetParam_SelfOverwrite_001
 * @tc.name: SetParam self-overwrite
 * @tc.desc: SetParam(key, GetParam(key)) self-overwrite preserves the managed state (F-04).
 */
HWTEST_F(WantFdStateTest, WantFdState_SetParam_SelfOverwrite_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    WantParams wp;
    ASSERT_EQ(wp.SetFd(FD_KEY, fd, FdOwnership::TAKE_TAGGED), FdSetStatus::SUCCESS);
    int got = -1;
    ASSERT_EQ(wp.GetFd(FD_KEY, got), FdGetStatus::SUCCESS);
    // Self-overwrite with the same FD marker must preserve the state.
    wp.SetParam(FD_KEY, wp.GetParam(FD_KEY));
    int gotAfter = -1;
    ASSERT_EQ(wp.GetFd(FD_KEY, gotAfter), FdGetStatus::SUCCESS);
    EXPECT_EQ(got, gotAfter);
    EXPECT_NE(fcntl(gotAfter, F_GETFD), -1);
    wp.CloseAllFd();
}

/**
 * @tc.number: WantFdState_SetParam_PlainOverwrite_001
 * @tc.name: SetParam plain overwrite
 * @tc.desc: SetParam(key, plain value) over an fd key leaves fds_ state residual; CloseAllFd closes it (F-04).
 */
HWTEST_F(WantFdStateTest, WantFdState_SetParam_PlainOverwrite_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    WantParams wp;
    ASSERT_EQ(wp.SetFd(FD_KEY, fd, FdOwnership::TAKE_TAGGED), FdSetStatus::SUCCESS);
    int got = -1;
    ASSERT_EQ(wp.GetFd(FD_KEY, got), FdGetStatus::SUCCESS);
    wp.SetParam(FD_KEY, String::Box("not-fd"));
    EXPECT_EQ(wp.GetFd(FD_KEY, got), FdGetStatus::SUCCESS);
    EXPECT_NE(fcntl(got, F_GETFD), -1);
    wp.CloseAllFd();
    EXPECT_EQ(fcntl(got, F_GETFD), -1);
}

/**
 * @tc.number: WantFdState_RecursiveClose_001
 * @tc.name: Recursive Close
 * @tc.desc: CloseAllFd(RECURSIVE) closes fd states in nested WantParams without touching params_.
 */
HWTEST_F(WantFdStateTest, WantFdState_RecursiveClose_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    int nestedFd = OpenFd();
    ASSERT_GE(nestedFd, 0);
    WantParams wp;
    ASSERT_EQ(wp.SetFd(FD_KEY, fd, FdOwnership::TAKE_TAGGED), FdSetStatus::SUCCESS);
    int topFd = -1;
    ASSERT_EQ(wp.GetFd(FD_KEY, topFd), FdGetStatus::SUCCESS);
    // Nested WantParams with its own managed fd (move-box preserves fds_ ownership).
    WantParams nested;
    ASSERT_EQ(nested.SetFd("nested_fd", nestedFd, FdOwnership::TAKE_TAGGED), FdSetStatus::SUCCESS);
    wp.SetParam("nested", WantParamWrapper::Box(std::move(nested)));

    EXPECT_EQ(wp.CloseAllFdWithStatus(FdTraversalMode::RECURSIVE), FdCloseStatus::SUCCESS);
    EXPECT_EQ(fcntl(fd, F_GETFD), -1);
    EXPECT_EQ(fcntl(nestedFd, F_GETFD), -1);
    EXPECT_EQ(wp.GetFd(FD_KEY, topFd), FdGetStatus::CLOSED);
}

/**
 * @tc.number: WantFdState_LegacyExplicit_Close_001
 * @tc.name: LEGACY_EXPLICIT close
 * @tc.desc: LEGACY_EXPLICIT is force-closed by CloseAllFd (legacy semantics), not by destructor.
 */
HWTEST_F(WantFdStateTest, WantFdState_LegacyExplicit_Close_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    WantParams wp;
    ASSERT_EQ(wp.SetFd(FD_KEY, fd, FdOwnership::LEGACY_EXPLICIT), FdSetStatus::SUCCESS);
    int got = -1;
    ASSERT_EQ(wp.GetFd(FD_KEY, got), FdGetStatus::SUCCESS);
    EXPECT_EQ(got, fd);
    EXPECT_EQ(wp.CloseAllFdWithStatus(FdTraversalMode::CURRENT_LEVEL), FdCloseStatus::SUCCESS);
    EXPECT_EQ(fcntl(fd, F_GETFD), -1);
}

/**
 * @tc.number: WantFdState_LegacyExplicit_Remove_001
 * @tc.name: LEGACY_EXPLICIT remove
 * @tc.desc: RemoveAllFd does not close a LEGACY_EXPLICIT fd (legacy leak semantics).
 */
HWTEST_F(WantFdStateTest, WantFdState_LegacyExplicit_Remove_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    WantParams wp;
    ASSERT_EQ(wp.SetFd(FD_KEY, fd, FdOwnership::LEGACY_EXPLICIT), FdSetStatus::SUCCESS);
    EXPECT_EQ(wp.RemoveAllFdWithStatus(), FdRemoveStatus::SUCCESS);
    EXPECT_NE(fcntl(fd, F_GETFD), -1);
    close(fd);
}

/**
 * @tc.number: WantFdState_AdoptFd_Upgrade_001
 * @tc.name: AdoptAllLegacyFd upgrades LEGACY_EXPLICIT
 * @tc.desc: AdoptAllLegacyFd turns LEGACY_EXPLICIT into RAII-owned; the fd closes on WantParams destruction.
 */
HWTEST_F(WantFdStateTest, WantFdState_AdoptFd_Upgrade_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    {
        WantParams wp;
        ASSERT_EQ(wp.SetFd(FD_KEY, fd, FdOwnership::LEGACY_EXPLICIT), FdSetStatus::SUCCESS);
        EXPECT_EQ(wp.AdoptAllLegacyFd(FdTraversalMode::CURRENT_LEVEL), FdAdoptStatus::SUCCESS);
    }
    EXPECT_EQ(fcntl(fd, F_GETFD), -1);
}

/**
 * @tc.number: WantFdState_AdoptFd_Idempotent_001
 * @tc.name: AdoptAllLegacyFd idempotent on owned
 * @tc.desc: AdoptAllLegacyFd on an already-owned fd keeps it owned; CloseAllFd still closes it.
 */
HWTEST_F(WantFdStateTest, WantFdState_AdoptFd_Idempotent_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    WantParams wp;
    ASSERT_EQ(wp.SetFd(FD_KEY, fd, FdOwnership::TAKE_TAGGED), FdSetStatus::SUCCESS);
    EXPECT_EQ(wp.AdoptAllLegacyFd(FdTraversalMode::CURRENT_LEVEL), FdAdoptStatus::SUCCESS);
    wp.CloseAllFd();
    EXPECT_EQ(fcntl(fd, F_GETFD), -1);
}

/**
 * @tc.number: WantFdState_AdoptAllLegacyFd_001
 * @tc.name: AdoptAllLegacyFd recursive
 * @tc.desc: AdoptAllLegacyFd(RECURSIVE) upgrades top-level and nested LEGACY_EXPLICIT fds to owned.
 */
HWTEST_F(WantFdStateTest, WantFdState_AdoptAllLegacyFd_001, Function | MediumTest | Level1)
{
    int fd1 = OpenFd();
    ASSERT_GE(fd1, 0);
    int fd2 = OpenFd();
    ASSERT_GE(fd2, 0);
    WantParams wp;
    ASSERT_EQ(wp.SetFd(FD_KEY, fd1, FdOwnership::LEGACY_EXPLICIT), FdSetStatus::SUCCESS);
    WantParams nested;
    ASSERT_EQ(nested.SetFd("nested_fd", fd2, FdOwnership::LEGACY_EXPLICIT), FdSetStatus::SUCCESS);
    wp.SetParam("nested", WantParamWrapper::Box(std::move(nested)));

    wp.AdoptAllLegacyFd(FdTraversalMode::RECURSIVE);
    EXPECT_EQ(wp.CloseAllFdWithStatus(FdTraversalMode::RECURSIVE), FdCloseStatus::SUCCESS);
    EXPECT_EQ(fcntl(fd1, F_GETFD), -1);
    EXPECT_EQ(fcntl(fd2, F_GETFD), -1);
}
/**
 * @tc.number: WantFdState_SetFd_DuplicateKey_Take_001
 * @tc.name: SetFd duplicate key TAKE
 * @tc.desc: Second SetFd on the same key closes the old fd exactly once and registers the new one (close-last).
 */
HWTEST_F(WantFdStateTest, WantFdState_SetFd_DuplicateKey_Take_001, Function | MediumTest | Level1)
{
    int fd1 = OpenFd();
    ASSERT_GE(fd1, 0);
    int fd2 = OpenFd();
    ASSERT_GE(fd2, 0);
    WantParams wp;
    ASSERT_EQ(wp.SetFd(FD_KEY, fd1, FdOwnership::TAKE_TAGGED), FdSetStatus::SUCCESS);
    ASSERT_EQ(wp.SetFd(FD_KEY, fd2, FdOwnership::TAKE_TAGGED), FdSetStatus::SUCCESS);
    EXPECT_EQ(fcntl(fd1, F_GETFD), -1);
    int got = -1;
    ASSERT_EQ(wp.GetFd(FD_KEY, got), FdGetStatus::SUCCESS);
    EXPECT_EQ(got, fd2);
    EXPECT_NE(fcntl(fd2, F_GETFD), -1);
    wp.CloseAllFd();
    EXPECT_EQ(fcntl(fd2, F_GETFD), -1);
}

/**
 * @tc.number: WantFdState_GetFd_ExternalClose_001
 * @tc.name: GetFd after external close
 * @tc.desc: An externally closed managed fd is reported CLOSED via liveness probe, not SUCCESS.
 */
HWTEST_F(WantFdStateTest, WantFdState_GetFd_ExternalClose_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    WantParams wp;
    ASSERT_EQ(wp.SetFd(FD_KEY, fd, FdOwnership::LEGACY_EXPLICIT), FdSetStatus::SUCCESS);
    close(fd);
    int got = -1;
    EXPECT_EQ(wp.GetFd(FD_KEY, got), FdGetStatus::CLOSED);
}

/**
 * @tc.number: WantFdState_WriteToParcelFD_RefuseDead_001
 * @tc.name: managed write refuses dead fd
 * @tc.desc: WriteToParcelFD writes a live managed fd and refuses one closed externally.
 */
HWTEST_F(WantFdStateTest, WantFdState_WriteToParcelFD_RefuseDead_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    WantParams wp;
    ASSERT_EQ(wp.SetFd(FD_KEY, fd, FdOwnership::LEGACY_EXPLICIT), FdSetStatus::SUCCESS);
    WantParams marker;
    MessageParcel alive;
    EXPECT_TRUE(wp.WriteToParcelFD(alive, FD_KEY, marker));
    close(fd);
    MessageParcel dead;
    EXPECT_FALSE(wp.WriteToParcelFD(dead, FD_KEY, marker));
}

/**
 * @tc.number: WantFdState_GetFd_BareMarker_001
 * @tc.name: GetFd legacy fallback live
 * @tc.desc: A bare FD marker registered via SetParam resolves to SUCCESS with the marker fd.
 */
HWTEST_F(WantFdStateTest, WantFdState_GetFd_BareMarker_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    WantParams marker;
    marker.SetParam(TYPE_PROPERTY, String::Box(FD));
    marker.SetParam(VALUE_PROPERTY, Integer::Box(fd));
    WantParams wp;
    wp.SetParam(FD_KEY, WantParamWrapper::Box(marker));
    int got = -1;
    EXPECT_EQ(wp.GetFd(FD_KEY, got), FdGetStatus::SUCCESS);
    EXPECT_EQ(got, fd);
    close(fd);
}

/**
 * @tc.number: WantFdState_GetFd_InvalidMarker_001
 * @tc.name: GetFd invalid marker
 * @tc.desc: A non-marker param at the key yields INVALID_MARKER from the legacy fallback.
 */
HWTEST_F(WantFdStateTest, WantFdState_GetFd_InvalidMarker_001, Function | MediumTest | Level1)
{
    WantParams wp;
    wp.SetParam(FD_KEY, String::Box("not-fd"));
    int got = -1;
    EXPECT_EQ(wp.GetFd(FD_KEY, got), FdGetStatus::INVALID_MARKER);
}

/**
 * @tc.number: WantFdState_GetFd_BareMarkerClosed_001
 * @tc.name: GetFd legacy fallback closed
 * @tc.desc: A bare marker whose fd was closed yields CLOSED via the liveness probe.
 */
HWTEST_F(WantFdStateTest, WantFdState_GetFd_BareMarkerClosed_001, Function | MediumTest | Level1)
{
    int fd = OpenFd();
    ASSERT_GE(fd, 0);
    close(fd);
    WantParams marker;
    marker.SetParam(TYPE_PROPERTY, String::Box(FD));
    marker.SetParam(VALUE_PROPERTY, Integer::Box(fd));
    WantParams wp;
    wp.SetParam(FD_KEY, WantParamWrapper::Box(marker));
    int got = -1;
    EXPECT_EQ(wp.GetFd(FD_KEY, got), FdGetStatus::CLOSED);
}
} // namespace AAFwk
} // namespace OHOS
