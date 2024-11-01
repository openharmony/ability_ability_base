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

#ifndef OHOS_ABILITY_BASE_ABILITY_BASE_HILOG_WRAPPER_H
#define OHOS_ABILITY_BASE_ABILITY_BASE_HILOG_WRAPPER_H

#define ABILITYBASE_CONFIG_HILOG
#ifdef ABILITYBASE_CONFIG_HILOG
#include "hilog/log.h"

#ifdef ABILITYBASE_LOGF
#undef ABILITYBASE_LOGF
#endif

#ifdef ABILITYBASE_LOGE
#undef ABILITYBASE_LOGE
#endif

#ifdef ABILITYBASE_LOGW
#undef ABILITYBASE_LOGW
#endif

#ifdef ABILITYBASE_LOGI
#undef ABILITYBASE_LOGI
#endif

#ifdef ABILITYBASE_LOGD
#undef ABILITYBASE_LOGD
#endif

#ifndef ABILITYBASE_LOG_DOMAIN
#define ABILITYBASE_LOG_DOMAIN 0xD001305
#endif

#ifndef ABILITYBASE_LOG_TAG
#define ABILITYBASE_LOG_TAG "Base"
#endif

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define ABILITYBASE_LOGF(fmt, ...)                             \
    ((void)HILOG_IMPL(LOG_CORE, LOG_FATAL, ABILITYBASE_LOG_DOMAIN, ABILITYBASE_LOG_TAG, \
        "[%{public}s(%{public}s:%{public}d)]" fmt,             \
        __FILENAME__,                                          \
        __FUNCTION__,                                          \
        __LINE__,                                              \
        ##__VA_ARGS__))
#define ABILITYBASE_LOGE(fmt, ...)                             \
    ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, ABILITYBASE_LOG_DOMAIN, ABILITYBASE_LOG_TAG, \
        "[%{public}s(%{public}s:%{public}d)]" fmt,             \
        __FILENAME__,                                          \
        __FUNCTION__,                                          \
        __LINE__,                                              \
        ##__VA_ARGS__))
#define ABILITYBASE_LOGW(fmt, ...)                            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_WARN, ABILITYBASE_LOG_DOMAIN, ABILITYBASE_LOG_TAG, \
        "[%{public}s(%{public}s:%{public}d)]" fmt,            \
        __FILENAME__,                                         \
        __FUNCTION__,                                         \
        __LINE__,                                             \
        ##__VA_ARGS__))
#define ABILITYBASE_LOGI(fmt, ...)                            \
    ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, ABILITYBASE_LOG_DOMAIN, ABILITYBASE_LOG_TAG, \
        "[%{public}s(%{public}s:%{public}d)]" fmt,            \
        __FILENAME__,                                         \
        __FUNCTION__,                                         \
        __LINE__,                                             \
        ##__VA_ARGS__))
#define ABILITYBASE_LOGD(fmt, ...)                             \
    ((void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, ABILITYBASE_LOG_DOMAIN, ABILITYBASE_LOG_TAG, \
        "[%{public}s(%{public}s:%{public}d)]" fmt,             \
        __FILENAME__,                                          \
        __FUNCTION__,                                          \
        __LINE__,                                              \
        ##__VA_ARGS__))
#else

#define ABILITYBASE_LOGF(...)
#define ABILITYBASE_LOGE(...)
#define ABILITYBASE_LOGW(...)
#define ABILITYBASE_LOGI(...)
#define ABILITYBASE_LOGD(...)
#endif // ABILITYBASE_CONFIG_HILOG

#endif // OHOS_ABILITY_BASE_ABILITY_BASE_HILOG_WRAPPER_H
