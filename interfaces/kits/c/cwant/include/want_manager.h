/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef ABILITY_BASE_WANT_MANAGER_H
#define ABILITY_BASE_WANT_MANAGER_H

#include "want.h"
#include "want/include/want.h"

struct AbilityBase_Want {
    AbilityBase_Element element;
    std::map<std::string, std::string> params;
    std::map<std::string, int32_t> fds;
    std::map<std::string, int32_t> intParams;
    std::map<std::string, bool> boolParams;
    std::map<std::string, double> doubleParams;
    std::string uri = "";
    int32_t flag = 0;
};

namespace OHOS {
namespace AAFwk {
class CWantManager {
public:
    [[maybe_unused]] static AbilityBase_ErrorCode TransformToWant(AbilityBase_Want cwant, bool isDup,
        Want &want);
    [[maybe_unused]] static AbilityBase_ErrorCode TransformToCWantWithoutElement(
        Want want, bool isDup, AbilityBase_Want &cwant);
};
}
}
#endif // ABILITY_BASE_WANT_MANAGER_H