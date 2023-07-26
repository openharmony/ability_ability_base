/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_SESSION_INFO_H
#define OHOS_ABILITY_SESSION_INFO_H

#include <typeinfo>
#include <unistd.h>

#include "iremote_object.h"
#include "parcel.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityStartSetting;

enum class CallToState : uint32_t {
    UNKNOW = 0,
    FOREGROUND,
    BACKGROUND
};

class SessionInfo : public Parcelable {
public:
    SessionInfo() = default;
    virtual ~SessionInfo() = default;
    bool Marshalling(Parcel& parcel) const override;
    static SessionInfo* Unmarshalling(Parcel& parcel);

    sptr<IRemoteObject> sessionToken = nullptr;
    sptr<IRemoteObject> callerSession = nullptr;
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t persistentId = 0;
    // Status of the UIAbility invoked by the StartABilityByCall
    CallToState state = CallToState::UNKNOW;
    // Returned resultCode of terminateSelfWithResult
    int32_t resultCode = -1;
    // requestCode of StartAbilityForResult
    int32_t requestCode = -1;
    std::string errorReason;
    int32_t errorCode = -1;
    int64_t uiAbilityId = 0;
    std::shared_ptr<AbilityStartSetting> startSetting = nullptr;
    Want want;
    int32_t userId = -1;
    bool isNewWant = true;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_SESSION_INFO_H