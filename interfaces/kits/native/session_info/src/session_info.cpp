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

#include "session_info.h"

#include "ability_base_log_wrapper.h"
#include "ability_start_setting.h"

namespace OHOS {
namespace AAFwk {
bool SessionInfo::Marshalling(Parcel& parcel) const
{
    if (sessionToken) {
        if (!parcel.WriteBool(true) ||
            !(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(sessionToken)) {
            ABILITYBASE_LOGE("Write session token failed.");
            return false;
        }
    } else {
        if (!parcel.WriteBool(false)) {
            return false;
        }
    }

    if (callerSession) {
        if (!parcel.WriteBool(true) ||
            !(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(callerSession)) {
            ABILITYBASE_LOGE("Write caller session failed.");
            return false;
        }
    } else {
        if (!parcel.WriteBool(false)) {
            return false;
        }
    }

    if (callerToken) {
        if (!parcel.WriteBool(true) ||
            !(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(callerToken)) {
            ABILITYBASE_LOGE("Write caller token failed.");
            return false;
        }
    } else {
        if (!parcel.WriteBool(false)) {
            return false;
        }
    }

    if (!parcel.WriteInt32(persistentId)) {
        ABILITYBASE_LOGE("Write persistent id failed.");
        return false;
    }

    if (!parcel.WriteUint32(static_cast<uint32_t>(state))) {
        ABILITYBASE_LOGE("Write state failed.");
        return false;
    }

    if (!parcel.WriteInt32(resultCode)) {
        ABILITYBASE_LOGE("Write resultCode failed.");
        return false;
    }

    if (!parcel.WriteInt32(requestCode)) {
        ABILITYBASE_LOGE("Write requestCode failed.");
        return false;
    }

    if (!parcel.WriteString(errorReason)) {
        ABILITYBASE_LOGE("Write errorReason failed.");
        return false;
    }

    if (!parcel.WriteInt32(errorCode)) {
        ABILITYBASE_LOGE("Write errorCode failed.");
        return false;
    }

    if (!parcel.WriteInt64(uiAbilityId)) {
        ABILITYBASE_LOGE("Write uiAbilityId failed.");
        return false;
    }

    if (!parcel.WriteParcelable(startSetting.get())) {
        ABILITYBASE_LOGE("Write startSetting failed.");
        return false;
    }

    if (!parcel.WriteBool(isNewWant)) {
        return false;
    }

    if (!parcel.WriteParcelable(&want)) {
        ABILITYBASE_LOGE("Write want failed.");
        return false;
    }
    return true;
}

SessionInfo* SessionInfo::Unmarshalling(Parcel& parcel)
{
    SessionInfo* info = new SessionInfo();
    if (parcel.ReadBool()) {
        info->sessionToken = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
    }

    if (parcel.ReadBool()) {
        info->callerSession = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
    }

    if (parcel.ReadBool()) {
        info->callerToken = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
    }

    info->persistentId = parcel.ReadInt32();
    info->state = static_cast<CallToState>(parcel.ReadUint32());
    info->resultCode = parcel.ReadInt32();
    info->requestCode = parcel.ReadInt32();
    info->errorReason = parcel.ReadString();
    info->errorCode = parcel.ReadInt32();
    info->uiAbilityId = parcel.ReadInt64();
    info->startSetting.reset(parcel.ReadParcelable<AbilityStartSetting>());
    info->isNewWant = parcel.ReadBool();

    std::unique_ptr<Want> want(parcel.ReadParcelable<Want>());
    if (want != nullptr) {
        info->want = *want;
    }
    return info;
}
}  // namespace AAFwk
}  // namespace OHOS