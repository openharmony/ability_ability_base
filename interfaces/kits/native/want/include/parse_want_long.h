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

#ifndef PARSE_WANT_LONG_H
#define PARSE_WANT_LONG_H

#include <charconv>
#include <string>
#include <system_error>

namespace OHOS {
namespace AAFwk {
inline bool ParseWantLong(const std::string &text, long &out)
{
    if (text.empty()) {
        return false;
    }
    const char *first = text.data();
    const char *last = first + text.size();
    if (*first == '+') {
        ++first;
        if (first == last) {
            return false;
        }
    }
    long value = 0;
    auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc() || result.ptr == first) {
        return false;
    }
    if (result.ptr != last) {
        if (*result.ptr != '.') {
            return false;
        }
        const char *p = result.ptr + 1;
        if (p == last) {
            return false;
        }
        while (p != last) {
            if (*p < '0' || *p > '9') {
                return false;
            }
            ++p;
        }
    }
    out = value;
    return true;
}
} // namespace AAFwk
} // namespace OHOS
#endif // PARSE_WANT_LONG_H
