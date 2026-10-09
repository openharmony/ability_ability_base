/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_map>

#define private public
#include "configuration.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class BaseConfigurationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void BaseConfigurationTest::SetUpTestCase()
{
}

void BaseConfigurationTest::TearDownTestCase()
{
}

void BaseConfigurationTest::SetUp()
{
}

void BaseConfigurationTest::TearDown()
{
}

/*
 * Feature: Configuration
 * Function: FilterDuplicates
 * SubFunction: NA
 * FunctionPoints:FilterDuplicates Configuration
 * EnvConditions: NA
 * CaseDescription: FilterDuplicates Configuratio, call FilterDuplicates function.
 */
HWTEST_F(BaseConfigurationTest, Configuration_FilterDuplicates_001, TestSize.Level1)
{
    AppExecFwk::Configuration config;
    ASSERT_TRUE(config.configParameter_.empty());
    auto configuration = std::make_shared<AppExecFwk::Configuration>();
    configuration->FilterDuplicates(config);
    config.configParameter_.emplace(std::make_pair("key1", "value1"));
    configuration->configParameter_.emplace(std::make_pair("key1", "value1"));
    configuration->FilterDuplicates(config);
    EXPECT_EQ(configuration->configParameter_.size(), 0);
}

/* Copy both the display ID and its values, and replace old destination entries. */
HWTEST_F(BaseConfigurationTest, Configuration_CopyState_001, TestSize.Level1)
{
    const auto key = AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE;
    Configuration source;
    source.defaultDisplayId_ = 7;
    ASSERT_TRUE(source.AddItem(key, "en"));
    Configuration copy(source);
    Configuration assigned;
    ASSERT_TRUE(assigned.AddItem(0, key, "old"));
    assigned = source;
    EXPECT_EQ(copy.GetItem(key), "en");
    EXPECT_EQ(assigned.GetItem(key), "en");
    EXPECT_EQ(assigned.GetItem(0, key), "");
    EXPECT_EQ(assigned.GetItemSize(), 1);
    source.AddItem(key, "zh");
    EXPECT_EQ(copy.GetItem(key), "en");
    EXPECT_EQ(assigned.GetItem(key), "en");
}

/* Self comparison/merge/assignment are no-ops; self filtering preserves empty values. */
HWTEST_F(BaseConfigurationTest, Configuration_SelfOperations_001, TestSize.Level1)
{
    Configuration config;
    config.configParameter_ = {{"key", "value"}, {"empty", ""}};
    std::vector<std::string> diff {"stale"};
    config.CompareDifferent(diff, config);
    EXPECT_TRUE(diff.empty());
    config.Merge({"key", "empty", "missing"}, config);
    const Configuration &same = config;
    config = same;
    EXPECT_EQ(config.GetItemSize(), 2);
    config.FilterDuplicates(config);
    EXPECT_EQ(config.GetItemSize(), 1);
    EXPECT_EQ(config.configParameter_.count("empty"), 1);
}

/* Missing/empty source values must not clear the target or create new entries. */
HWTEST_F(BaseConfigurationTest, Configuration_CompareMerge_001, TestSize.Level1)
{
    Configuration source;
    source.configParameter_ = {{"same", "one"}, {"changed", "two"}, {"new", "three"}, {"empty", ""}};
    Configuration target;
    target.configParameter_ = {{"same", "one"}, {"changed", "old"}, {"empty", "keep"}};
    std::vector<std::string> diff;
    target.CompareDifferent(diff, source);
    std::sort(diff.begin(), diff.end());
    const std::vector<std::string> expected {"changed", "new"};
    EXPECT_EQ(diff, expected);
    EXPECT_EQ(target.GetItemSize(), 3);
    diff.push_back("empty");
    diff.push_back("missing");
    target.Merge(diff, source);
    EXPECT_EQ(target.GetValue("changed"), "two");
    EXPECT_EQ(target.GetValue("new"), "three");
    EXPECT_EQ(target.GetValue("empty"), "keep");
    EXPECT_EQ(target.GetItemSize(), 4);
}

/* Filtering uses nonempty key presence, not value equality. */
HWTEST_F(BaseConfigurationTest, Configuration_FilterDuplicates_002, TestSize.Level1)
{
    Configuration source;
    source.configParameter_ = {{"shared", "new"}, {"sourceEmpty", ""}, {"targetEmpty", "value"}};
    Configuration target;
    target.configParameter_ = {
        {"shared", "old"}, {"sourceEmpty", "keep"}, {"targetEmpty", ""}, {"unique", "keep"}};
    target.FilterDuplicates(source);
    EXPECT_EQ(target.configParameter_.count("shared"), 0);
    EXPECT_EQ(target.GetItemSize(), 3);
    EXPECT_EQ(target.GetValue("sourceEmpty"), "keep");
    EXPECT_EQ(target.configParameter_.count("targetEmpty"), 1);
    EXPECT_EQ(target.GetValue("unique"), "keep");
}

/* An empty source clears only the comparison output and leaves the target unchanged. */
HWTEST_F(BaseConfigurationTest, Configuration_EmptySource_001, TestSize.Level1)
{
    Configuration source;
    Configuration target;
    target.configParameter_ = {{"key", "value"}, {"empty", ""}};
    const auto expected = target.configParameter_;
    std::vector<std::string> diff {"stale"};
    target.CompareDifferent(diff, source);
    EXPECT_TRUE(diff.empty());
    target.Merge({"key", "missing"}, source);
    EXPECT_EQ(target.configParameter_, expected);
    target.FilterDuplicates(source);
    EXPECT_EQ(target.configParameter_, expected);
}

/* Source map replacement must not tear a copy, merge or comparison into two versions. */
HWTEST_F(BaseConfigurationTest, Configuration_ConcurrentSnapshot_001, TestSize.Level1)
{
    const auto language = AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE;
    const auto locale = AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE;
    Configuration first;
    first.AddItem(0, language, "one");
    first.AddItem(0, locale, "one");
    Configuration second;
    second.AddItem(0, language, "two");
    second.AddItem(0, locale, "two");
    Configuration source(first);
    std::vector<std::string> keys;
    first.GetAllKey(keys);
    std::atomic<bool> start {false};
    std::atomic<bool> consistent {true};
    auto check = [&language, &locale, &consistent](const Configuration &config) {
        const auto value = config.GetItem(0, language);
        if ((value != "one" && value != "two") || config.GetItem(0, locale) != value) {
            consistent.store(false);
        }
    };
    std::thread writer([&start, &source, &first, &second] {
        while (!start.load()) {
            std::this_thread::yield();
        }
        for (int i = 0; i < 1000; ++i) {
            // Replace only the map under its lock; display ID stays unchanged.
            std::lock_guard<std::recursive_mutex> lock(source.configParameterMutex_);
            source.configParameter_ = ((i % 2 == 0) ? second : first).configParameter_;
        }
    });
    std::thread reader([&start, &source, &check, &keys, &first, &consistent] {
        start.store(true);
        for (int i = 0; i < 1000; ++i) {
            Configuration copy(source);
            check(copy);
            Configuration assigned;
            assigned = source;
            check(assigned);
            Configuration merged;
            merged.Merge(keys, source);
            check(merged);
            std::vector<std::string> diff;
            first.CompareDifferent(diff, source);
            if (!diff.empty() && diff.size() != 2) {
                consistent.store(false);
            }
        }
    });
    writer.join();
    reader.join();
    EXPECT_TRUE(consistent.load());
}

/* Concurrent opposite-direction calls must complete without an object-lock cycle. */
HWTEST_F(BaseConfigurationTest, Configuration_OppositeDirections_001, TestSize.Level1)
{
    const auto key = AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE;
    Configuration first;
    Configuration second;
    first.AddItem(key, "one");
    second.AddItem(key, "two");
    std::vector<std::string> keys;
    first.GetAllKey(keys);
    std::atomic<bool> start {false};
    auto update = [&start, &key, &keys](Configuration &target, Configuration &source) {
        while (!start.load()) {
            std::this_thread::yield();
        }
        for (int i = 0; i < 1000; ++i) {
            target.AddItem(0, key, "value");
            std::vector<std::string> diff;
            target.CompareDifferent(diff, source);
            target.Merge(keys, source);
            target.FilterDuplicates(source);
            target = source;
        }
    };
    std::thread forward([&update, &first, &second] { update(first, second); });
    std::thread reverse([&update, &first, &second] { update(second, first); });
    start.store(true);
    forward.join();
    reverse.join();
    EXPECT_LE(first.GetItemSize(), 1);
    EXPECT_LE(second.GetItemSize(), 1);
}
} // namespace AppExecFwk
} // namespace OHOS
