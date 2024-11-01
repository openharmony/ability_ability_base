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

#ifndef OHOS_ABILITY_BASE_PARCEL_MACRO_BASE_H
#define OHOS_ABILITY_BASE_PARCEL_MACRO_BASE_H

#include "parcel.h"

#include "ability_base_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
#define READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(type, parcel, data)         \
    do {                                                                 \
        if (!(parcel).Read##type(data)) {                                \
            ABILITYBASE_LOGE("read failed: %{public}s", #type); \
            return false;                                                \
        }                                                                \
    } while (0)

#define WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(type, parcel, data)         \
    do {                                                                  \
        if (!(parcel).Write##type(data)) {                                \
            ABILITYBASE_LOGE("write failed: %{public}s", #type); \
            return false;                                                 \
        }                                                                 \
    } while (0)

#define WRITE_PARCEL_AND_RETURN(type, parcel, data)                               \
    do {                                                                          \
        if (!(parcel).Write##type(data)) {                                        \
            ABILITYBASE_LOGE("write failed: %{public}s", #type); \
            return;                                                               \
        }                                                                         \
    } while (0)
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_PARCEL_MACRO_BASE_H
