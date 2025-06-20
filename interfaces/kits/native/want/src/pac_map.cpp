/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "pac_map.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>

#include "bool_wrapper.h"
#include "byte_wrapper.h"
#include "cJSON.h"
#include "double_wrapper.h"
#include "float_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "short_wrapper.h"
#include "string_wrapper.h"
#include "user_object_base.h"
#include "user_object_wrapper.h"
#include "zchar_wrapper.h"
#include "array_wrapper.h"
#include "parcel_macro_base.h"
#include "string_ex.h"

using IUserObject = OHOS::AAFwk::IUserObject;
using InterfaceID = OHOS::AAFwk::InterfaceID;
using Short = OHOS::AAFwk::Short;
using Integer = OHOS::AAFwk::Integer;
using Long = OHOS::AAFwk::Long;
using Boolean = OHOS::AAFwk::Boolean;
using Char = OHOS::AAFwk::Char;
using Byte = OHOS::AAFwk::Byte;
using Float = OHOS::AAFwk::Float;
using Double = OHOS::AAFwk::Double;
using String = OHOS::AAFwk::String;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int MAX_ARRAY_ALLOW_SIZE = 1024;
const int FLOAT_PRECISION = 7;
const int DOUBLE_PRECISION = 17;
const std::regex NUMBER_REGEX("^[-+]?([0-9]+)([.]([0-9]+))?$");
};  // namespace

#define PAC_MAP_PUT_VALUE(id, iid, key, value, mapList) \
    do {                                                \
        RemoveData(mapList, key);                       \
        sptr<iid> val = id::Box(value);                 \
        (mapList).emplace(key, val);                    \
    } while (0);

#define PAC_MAP_GET_VALUE(id, key, value, mapList, defaultValue)      \
    do {                                                              \
        auto it = (mapList).find(key);                                \
        if (it != (mapList).end()) {                                  \
            if (id::Query(it->second.GetRefPtr()) != nullptr) {       \
                sptr<id> idValue = id::Query(it->second.GetRefPtr()); \
                value retVal = 0;                                     \
                idValue->GetValue(retVal);                            \
                return retVal;                                        \
            }                                                         \
            return defaultValue;                                      \
        }                                                             \
    } while (0);

#define PAC_MAP_GET_STRING_VALUE(id, key, value, mapList, defaultValue) \
    do {                                                                \
        auto it = (mapList).find(key);                                  \
        if (it != (mapList).end()) {                                    \
            if (id::Query((it)->second.GetRefPtr()) != nullptr) {       \
                sptr<id> idValue = id::Query(it->second.GetRefPtr());   \
                value retVal;                                           \
                idValue->GetString(retVal);                             \
                return retVal;                                          \
            }                                                           \
            return defaultValue;                                        \
        }                                                               \
    } while (0);

#define PAC_MAP_ADD_ARRAY(id, key, value, mapList)                                           \
    do {                                                                                     \
        RemoveData(mapList, key);                                                            \
        std::size_t size = (value).size();                                                     \
        sptr<IArray> ao = new Array(size, g_IID_##I##id);                                    \
        for (std::size_t i = 0; i < size; i++) {                                             \
            ao->Set(i, id::Box((value)[i]));                                                 \
        }                                                                                    \
        (mapList).emplace(key, sptr<IInterface>(static_cast<IInterface *>(ao.GetRefPtr()))); \
    } while (0);

#define GET_PAC_MAP_ARRAY(id, mapList, key, value)                                 \
    do {                                                                           \
        auto it = (mapList).find(key);                                             \
        if (it != (mapList).end()) {                                               \
            if (IArray::Query(it->second.GetRefPtr()) != nullptr) {                \
                if (Array::Is##id##Array(IArray::Query(it->second.GetRefPtr()))) { \
                    auto func = [ & ](IInterface * object) {                       \
                        if (I##id::Query(object) != nullptr) {                     \
                            (value).push_back(id::Unbox(I##id::Query(object)));    \
                        }                                                          \
                    };                                                             \
                    Array::ForEach(IArray::Query(it->second.GetRefPtr()), func);   \
                }                                                                  \
            }                                                                      \
        }                                                                          \
    } while (0);

template<typename IClassName, typename baseValue>
static void GetBaseDataValue(OHOS::AAFwk::IInterface *baseObj, cJSON *&json, int type)
{
    if (json == nullptr) {
        return;
    }
    IClassName *data = IClassName::Query(baseObj);
    baseValue val = 0;
    if (data != nullptr) {
        data->GetValue(val);
    }
    cJSON_AddNumberToObject(json, "data", static_cast<double>(val));
    cJSON_AddNumberToObject(json, "type", static_cast<double>(type));
}

template<typename RawType>
static std::string RawTypeToString(const RawType value, unsigned int precisionAfterPoint);

template<typename IClassName, typename ClassName, typename baseValue>
static void GetBaseFloatDoubleDataValue(OHOS::AAFwk::IInterface *baseObj, cJSON *&json, int type, int precision)
{
    if (json == nullptr) {
        return;
    }
    IClassName *data = IClassName::Query(baseObj);
    if (data != nullptr) {
        baseValue val = ClassName::Unbox(data);
        std::string value = RawTypeToString<baseValue>(val, precision);
        cJSON_AddStringToObject(json, "data", value.c_str());
        cJSON_AddNumberToObject(json, "type", static_cast<double>(type));
    }
}

static void GetBaseBoolDataValue(OHOS::AAFwk::IInterface *baseObj, cJSON *&json, int type)
{
    if (json == nullptr) {
        return;
    }
    IBoolean *data = IBoolean::Query(baseObj);
    bool val = false;
    if (data != nullptr) {
        data->GetValue(val);
    }
    cJSON_AddNumberToObject(json, "type", static_cast<double>(type));
    cJSON_AddBoolToObject(json, "data", val);
}

static void GetBaseStringDataValue(OHOS::AAFwk::IInterface *baseObj, cJSON *&json, int type)
{
    if (json == nullptr) {
        return;
    }
    IString *data = IString::Query(baseObj);
    std::string val;
    if (data != nullptr) {
        data->GetString(val);
    }
    cJSON_AddNumberToObject(json, "type", static_cast<double>(type));
    cJSON_AddStringToObject(json, "data", val.c_str());
}

static void GetBaseLongDataValue(OHOS::AAFwk::IInterface *baseObj, cJSON *&json, int type)
{
    if (json == nullptr) {
        return;
    }
    ILong *data = ILong::Query(baseObj);
    long val = 0;
    if (data != nullptr) {
        data->GetValue(val);
    }
    std::string value = std::to_string(val);
    cJSON_AddStringToObject(json, "data", value.c_str());
    cJSON_AddNumberToObject(json, "type", static_cast<double>(type));
}

template<typename IClassName, typename ClassName, typename valueType>
static void PacmapGetArrayVal(OHOS::AAFwk::IInterface *ao, std::vector<valueType> &array)
{
    if (ao == nullptr) {
        return;
    }
    if (IArray::Query(ao) != nullptr) {
        auto func = [&array](AAFwk::IInterface *object) {
            if (object != nullptr) {
                IClassName *value = IClassName::Query(object);
                if (value != nullptr) {
                    array.emplace_back(ClassName::Unbox(value));
                }
            }
        };
        Array::ForEach(IArray::Query(ao), func);
    }
}

using namespace OHOS::AAFwk;
IINTERFACE_IMPL_1(PacMap, Object, IPacMap);
/**
 * @brief A replication structure with deep copy.
 */
PacMap::PacMap(const PacMap &other)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    dataList_.clear();
    std::string str = MapListToString(other.dataList_);
    StringToMapList(str, dataList_);
}

PacMap::~PacMap()
{
    Clear();
}

/**
 * @brief A overload operation with shallow copy.
 */
PacMap &PacMap::operator=(const PacMap &other)
{
    if (&other != this) {
        dataList_.clear();
        std::string str = MapListToString(other.dataList_);
        StringToMapList(str, dataList_);
    }
    return *this;
}

/**
 * @brief Clear all key-value pairs and free resources.
 */
void PacMap::Clear(void)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    dataList_.clear();
}

/**
 * @brief Creates and returns a copy of this object with shallow copy.
 *
 * @return A clone of this instance.
 */
PacMap PacMap::Clone(void)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    PacMap pac_map;
    std::string currentString = MapListToString(dataList_);
    StringToMapList(currentString, pac_map.dataList_);
    return pac_map;
}

/**
 * @brief Creates a deep copy of this PacMap object with deep copy.
 *
 * @return
 */
PacMap PacMap::DeepCopy(void)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    PacMap pac_map;
    std::string str = MapListToString(dataList_);
    StringToMapList(str, pac_map.dataList_);
    return pac_map;
}

void PacMap::DeepCopy(PacMap &other)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    dataList_.clear();
    StringToMapList(MapListToString(other.dataList_), dataList_);
}

/**
 * @brief Adds a short value matching a specified key.
 * @param key A specified key.
 * @param value The value that matches the specified key.
 */
void PacMap::PutShortValue(const std::string &key, short value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutShortValue(dataList_, key, value);
}
void PacMap::InnerPutShortValue(PacMapList &mapList, const std::string &key, short value)
{
    PAC_MAP_PUT_VALUE(Short, IShort, key, value, mapList)
}
/**
 * @brief Adds a integer value matching a specified key.
 * @param key A specified key.
 * @param value The value that matches the specified key.
 */
void PacMap::PutIntValue(const std::string &key, int value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutIntValue(dataList_, key, value);
}
void PacMap::InnerPutIntValue(PacMapList &mapList, const std::string &key, int value)
{
    PAC_MAP_PUT_VALUE(Integer, IInteger, key, value, mapList)
}
/**
 * @brief Adds a long value matching a specified key.
 * @param key A specified key.
 * @param value The value that matches the specified key.
 */
void PacMap::PutLongValue(const std::string &key, long value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutLongValue(dataList_, key, value);
}
void PacMap::InnerPutLongValue(PacMapList &mapList, const std::string &key, long value)
{
    PAC_MAP_PUT_VALUE(Long, ILong, key, value, mapList)
}
/**
 * @brief Adds a boolean value matching a specified key.
 * @param key A specified key.
 * @param value The value that matches the specified key.
 */
void PacMap::PutBooleanValue(const std::string &key, bool value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutBooleanValue(dataList_, key, value);
}
void PacMap::InnerPutBooleanValue(PacMapList &mapList, const std::string &key, bool value)
{
    PAC_MAP_PUT_VALUE(Boolean, IBoolean, key, value, mapList)
}
/**
 * @brief Adds a char value matching a specified key.
 * @param key A specified key.
 * @param value The value that matches the specified key.
 */
void PacMap::PutCharValue(const std::string &key, char value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutCharValue(dataList_, key, value);
}
void PacMap::InnerPutCharValue(PacMapList &mapList, const std::string &key, char value)
{
    PAC_MAP_PUT_VALUE(Char, IChar, key, value, mapList)
}

/**
 * @brief Adds a byte value matching a specified key.
 * @param key A specified key.
 * @param value The value that matches the specified key.
 */
void PacMap::PutByteValue(const std::string &key, AAFwk::byte value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutByteValue(dataList_, key, value);
}
void PacMap::InnerPutByteValue(PacMapList &mapList, const std::string &key, AAFwk::byte value)
{
    PAC_MAP_PUT_VALUE(Byte, IByte, key, value, mapList)
}
/**
 * @brief Adds a float value matching a specified key.
 * @param key A specified key.
 * @param value The value that matches the specified key.
 */
void PacMap::PutFloatValue(const std::string &key, float value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutFloatValue(dataList_, key, value);
}
void PacMap::InnerPutFloatValue(PacMapList &mapList, const std::string &key, float value)
{
    PAC_MAP_PUT_VALUE(Float, IFloat, key, value, mapList)
}

/**
 * @brief Adds a double value matching a specified key.
 * @param key A specified key.
 * @param value The value that matches the specified key.
 */
void PacMap::PutDoubleValue(const std::string &key, double value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutDoubleValue(dataList_, key, value);
}
void PacMap::InnerPutDoubleValue(PacMapList &mapList, const std::string &key, double value)
{
    PAC_MAP_PUT_VALUE(Double, IDouble, key, value, mapList)
}
/**
 * @brief Adds a string {std::string} value matching a specified key.
 * @param key A specified key.
 * @param value The value that matches the specified key.
 */
void PacMap::PutStringValue(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutStringValue(dataList_, key, value);
}
void PacMap::InnerPutStringValue(PacMapList &mapList, const std::string &key, const std::string &value)
{
    PAC_MAP_PUT_VALUE(String, IString, key, value, mapList);
}

/**
 * @brief Adds an object value matching a specified key. The object must be a subclass of UserObjectBase.
 * @param key A specified key.
 * @param value A smart pointer to the object that matches the specified key.
 */
void PacMap::PutObject(const std::string &key, const std::shared_ptr<UserObjectBase> &value)
{
    if (value == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutObject(dataList_, key, value);
}
void PacMap::InnerPutObject(PacMapList &mapList, const std::string &key, const std::shared_ptr<UserObjectBase> &value)
{
    RemoveData(mapList, key);
    sptr<IUserObject> valObject = OHOS::AAFwk::UserObject::Box(value);
    if (valObject == nullptr) {
        return;
    }
    mapList.emplace(key, valObject);
}
/**
 * @brief Adds an PacMap value matching a specified key.
 * @param key A specified key.
 * @param value The value that matches the specified key.
 */
bool PacMap::PutPacMap(const std::string &key, const PacMap &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    return InnerPutPacMap(dataList_, key, const_cast<PacMap &>(value));
}
bool PacMap::InnerPutPacMap(PacMapList &mapList, const std::string &key, PacMap &value)
{
    RemoveData(mapList, key);
    sptr<IPacMap> pacMap = new (std::nothrow) PacMap(value);
    if (pacMap != nullptr) {
        mapList.emplace(key, pacMap);
        return true;
    }
    return false;
}
/**
 * @brief Adds some short values matching a specified key.
 * @param key A specified key.
 * @param value Store a list of short values.
 */
void PacMap::PutShortValueArray(const std::string &key, const std::vector<short> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutShortValueArray(dataList_, key, value);
}
void PacMap::InnerPutShortValueArray(PacMapList &mapList, const std::string &key, const std::vector<short> &value)
{
    PAC_MAP_ADD_ARRAY(Short, key, value, mapList)
}
/**
 * @brief Adds some integer values matching a specified key.
 * @param key A specified key.
 * @param value Store a list of integer values.
 */
void PacMap::PutIntValueArray(const std::string &key, const std::vector<int> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutIntValueArray(dataList_, key, value);
}
void PacMap::InnerPutIntValueArray(PacMapList &mapList, const std::string &key, const std::vector<int> &value)
{
    PAC_MAP_ADD_ARRAY(Integer, key, value, mapList)
}
/**
 * @brief Adds some long values matching a specified key.
 * @param key A specified key.
 * @param value Store a list of long values.
 */
void PacMap::PutLongValueArray(const std::string &key, const std::vector<long> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutLongValueArray(dataList_, key, value);
}
void PacMap::InnerPutLongValueArray(PacMapList &mapList, const std::string &key, const std::vector<long> &value)
{
    PAC_MAP_ADD_ARRAY(Long, key, value, mapList)
}
/**
 * @brief Adds some boolean values matching a specified key.
 * @param key A specified key.
 * @param value Store a list of boolean values.
 */
void PacMap::PutBooleanValueArray(const std::string &key, const std::vector<bool> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutBooleanValueArray(dataList_, key, value);
}
void PacMap::InnerPutBooleanValueArray(PacMapList &mapList, const std::string &key, const std::vector<bool> &value)
{
    PAC_MAP_ADD_ARRAY(Boolean, key, value, mapList)
}
/**
 * @brief Adds some char values matching a specified key.
 * @param key A specified key.
 * @param value Store a list of char values.
 */
void PacMap::PutCharValueArray(const std::string &key, const std::vector<char> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutCharValueArray(dataList_, key, value);
}
void PacMap::InnerPutCharValueArray(PacMapList &mapList, const std::string &key, const std::vector<char> &value)
{
    PAC_MAP_ADD_ARRAY(Char, key, value, mapList)
}
/**
 * @brief Adds some byte values matching a specified key.
 * @param key A specified key.
 * @param value Store a list of byte values.
 */
void PacMap::PutByteValueArray(const std::string &key, const std::vector<AAFwk::byte> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutByteValueArray(dataList_, key, value);
}
void PacMap::InnerPutByteValueArray(PacMapList &mapList, const std::string &key, const std::vector<AAFwk::byte> &value)
{
    PAC_MAP_ADD_ARRAY(Byte, key, value, mapList)
}
/**
 * @brief Adds some float values matching a specified key.
 * @param key A specified key.
 * @param value Store a list of float values.
 */
void PacMap::PutFloatValueArray(const std::string &key, const std::vector<float> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutFloatValueArray(dataList_, key, value);
}
void PacMap::InnerPutFloatValueArray(PacMapList &mapList, const std::string &key, const std::vector<float> &value)
{
    PAC_MAP_ADD_ARRAY(Float, key, value, mapList)
}
/**
 * @brief Adds some double values matching a specified key.
 * @param key A specified key.
 * @param value Store a list of double values.
 */
void PacMap::PutDoubleValueArray(const std::string &key, const std::vector<double> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutDoubleValueArray(dataList_, key, value);
}
void PacMap::InnerPutDoubleValueArray(PacMapList &mapList, const std::string &key, const std::vector<double> &value)
{
    PAC_MAP_ADD_ARRAY(Double, key, value, mapList)
}
/**
 * @brief Adds some string {std::string} values matching a specified key.
 * @param key A specified key.
 * @param value Store a list of string values.
 */
void PacMap::PutStringValueArray(const std::string &key, const std::vector<std::string> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    InnerPutStringValueArray(dataList_, key, value);
}
void PacMap::InnerPutStringValueArray(
    PacMapList &mapList, const std::string &key, const std::vector<std::string> &value)
{
    PAC_MAP_ADD_ARRAY(String, key, value, mapList)
}
/**
 * @brief Inserts all key-value pairs of a map object into the built-in data object.
 * Duplicate key values will be replaced.
 * @param mapData Store a list of key-value pairs.
 */
void PacMap::PutAll(std::map<std::string, PacMapObject::INTERFACE> &mapData)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    dataList_.clear();
    StringToMapList(MapListToString(mapData), dataList_);
}

/**
 * @brief Saves the data in a PacMap object to the current object. Duplicate key values will be replaced.
 * @param pacMap Store the date of PacMap.
 */
void PacMap::PutAll(PacMap &pacMap)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    dataList_.clear();
    StringToMapList(MapListToString(pacMap.dataList_), dataList_);
}

/**
 * @brief Obtains the int value matching a specified key.
 * @param key A specified key.
 * @param defaultValue The return value when the function fails.
 * @return If the match is successful, return the value matching the key, otherwise return the @a defaultValue.
 */
int PacMap::GetIntValue(const std::string &key, int defaultValue)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    PAC_MAP_GET_VALUE(IInteger, key, int, dataList_, defaultValue)
    return defaultValue;
}

/**
 * @brief Obtains the short value matching a specified key.
 * @param key A specified key.
 * @param defaultValue The return value when the function fails.
 * @return If the match is successful, return the value matching the key, otherwise return the @a defaultValue.
 */
short PacMap::GetShortValue(const std::string &key, short defaultValue)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    PAC_MAP_GET_VALUE(IShort, key, short, dataList_, defaultValue)
    return defaultValue;
}

/**
 * @brief Obtains the boolean value matching a specified key.
 * @param key A specified key.
 * @param defaultValue The return value when the function fails.
 * @return If the match is successful, return the value matching the key, otherwise return the @a defaultValue.
 */
bool PacMap::GetBooleanValue(const std::string &key, bool defaultValue)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    PAC_MAP_GET_VALUE(IBoolean, key, bool, dataList_, defaultValue)
    return defaultValue;
}

/**
 * @brief Obtains the long value matching a specified key.
 * @param key A specified key.
 * @param defaultValue The return value when the function fails.
 * @return If the match is successful, return the value matching the key, otherwise return the @a defaultValue.
 */
long PacMap::GetLongValue(const std::string &key, long defaultValue)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    PAC_MAP_GET_VALUE(ILong, key, long, dataList_, defaultValue)
    return defaultValue;
}

/**
 * @brief Obtains the char value matching a specified key.
 * @param key A specified key.
 * @param defaultValue The return value when the function fails.
 * @return If the match is successful, return the value matching the key, otherwise return the @a defaultValue.
 */
char PacMap::GetCharValue(const std::string &key, char defaultValue)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    PAC_MAP_GET_VALUE(IChar, key, zchar, dataList_, defaultValue)
    return defaultValue;
}

/**
 * @brief Obtains the byte value matching a specified key.
 * @param key A specified key.
 * @param defaultValue The return value when the function fails.
 * @return If the match is successful, return the value matching the key, otherwise return the @a defaultValue.
 */
AAFwk::byte PacMap::GetByteValue(const std::string &key, AAFwk::byte defaultValue)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    PAC_MAP_GET_VALUE(IByte, key, byte, dataList_, defaultValue)
    return defaultValue;
}

/**
 * @brief Obtains the float value matching a specified key.
 * @param key A specified key.
 * @param defaultValue The return value when the function fails.
 * @return If the match is successful, return the value matching the key, otherwise return the @a defaultValue.
 */
float PacMap::GetFloatValue(const std::string &key, float defaultValue)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    PAC_MAP_GET_VALUE(IFloat, key, float, dataList_, defaultValue)
    return defaultValue;
}

/**
 * @brief Obtains the double value matching a specified key.
 * @param key A specified key.
 * @param defaultValue The return value when the function fails.
 * @return If the match is successful, return the value matching the key, otherwise return the @a defaultValue.
 */
double PacMap::GetDoubleValue(const std::string &key, double defaultValue)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    PAC_MAP_GET_VALUE(IDouble, key, double, dataList_, defaultValue)
    return defaultValue;
}

/**
 * @brief Obtains the string {std::string} value matching a specified key.
 * @param key A specified key.
 * @param defaultValue The return value when the function fails.
 * @return If the match is successful, return the value matching the key, otherwise return the @a defaultValue.
 */
std::string PacMap::GetStringValue(const std::string &key, const std::string &defaultValue)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    PAC_MAP_GET_STRING_VALUE(IString, key, std::string, dataList_, defaultValue)
    return defaultValue;
}

/**
 * @brief Obtains some int values matching a specified key.
 * @param key A specified key.
 * @param value Save the returned int values.
 */
void PacMap::GetIntValueArray(const std::string &key, std::vector<int> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    GET_PAC_MAP_ARRAY(Integer, dataList_, key, value)
}

/**
 * @brief Obtains some short values matching a specified key.
 * @param key A specified key.
 * @param value Save the returned short values.
 */
void PacMap::GetShortValueArray(const std::string &key, std::vector<short> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    GET_PAC_MAP_ARRAY(Short, dataList_, key, value)
}
/**
 * @brief Obtains some boolean values matching a specified key.
 * @param key A specified key.
 * @param value Save the returned boolean values.
 */
void PacMap::GetBooleanValueArray(const std::string &key, std::vector<bool> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    GET_PAC_MAP_ARRAY(Boolean, dataList_, key, value)
}

/**
 * @brief Obtains some long values matching a specified key.
 * @param key A specified key.
 * @param value Save the returned long values.
 */
void PacMap::GetLongValueArray(const std::string &key, std::vector<long> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    GET_PAC_MAP_ARRAY(Long, dataList_, key, value)
}

/**
 * @brief Obtains some char values matching a specified key.
 * @param key A specified key.
 * @param value Save the returned char values.
 */
void PacMap::GetCharValueArray(const std::string &key, std::vector<char> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    GET_PAC_MAP_ARRAY(Char, dataList_, key, value)
}

/**
 * @brief Obtains some byte values matching a specified key.
 * @param key A specified key.
 * @param value Save the returned byte values.
 */
void PacMap::GetByteValueArray(const std::string &key, std::vector<AAFwk::byte> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    GET_PAC_MAP_ARRAY(Byte, dataList_, key, value)
}

/**
 * @brief Obtains some float values matching a specified key.
 * @param key A specified key.
 * @param value Save the returned float values.
 */
void PacMap::GetFloatValueArray(const std::string &key, std::vector<float> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    GET_PAC_MAP_ARRAY(Float, dataList_, key, value)
}

/**
 * @brief Obtains some double values matching a specified key.
 * @param key A specified key.
 * @param value Save the returned double values.
 */
void PacMap::GetDoubleValueArray(const std::string &key, std::vector<double> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    GET_PAC_MAP_ARRAY(Double, dataList_, key, value)
}

/**
 * @brief Obtains some string {std::string} values matching a specified key.
 * @param key A specified key.
 * @param value Save the returned string {std::string} values.
 */
void PacMap::GetStringValueArray(const std::string &key, std::vector<std::string> &value)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    GET_PAC_MAP_ARRAY(String, dataList_, key, value)
}

/**
 * @brief Obtains the object matching a specified key.
 * @param key A specified key.
 * @return Returns the smart pointer to object that matches the key.
 */
std::shared_ptr<UserObjectBase> PacMap::GetObject(const std::string &key)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    auto it = dataList_.find(key);
    if (it == dataList_.end()) {
        return nullptr;
    }

    if (it->second != nullptr) {
        if (IUserObject::Query(it->second.GetRefPtr()) != nullptr) {
            return UserObject::Unbox(static_cast<IUserObject *>(it->second.GetRefPtr()));
        }
    }

    return nullptr;
}

/**
 * @brief Obtains the PacMap matching a specified key.
 * @param key A specified key.
 * @return Returns PacMap that matches the key.
 */
PacMap PacMap::GetPacMap(const std::string &key)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    PacMap pacmap;
    auto it = dataList_.find(key);
    if (it != dataList_.end()) {
        if (IPacMap::Query(it->second.GetRefPtr()) != nullptr) {
            pacmap.DeepCopy(*static_cast<PacMap *>(IPacMap::Query(it->second.GetRefPtr())));
        }
    }
    return pacmap;
}

/**
 * @brief Obtains all the data that has been stored with shallow copy.
 * @return Returns all data in current PacMap. There is no dependency between the returned data and
 * the original data.
 */
std::map<std::string, PacMapObject::INTERFACE> PacMap::GetAll(void)
{
    std::lock_guard<std::mutex> mLock(mapLock_);

    PacMapList tmpMapList;
    StringToMapList(MapListToString(dataList_), tmpMapList);

    return tmpMapList;
}

void PacMap::ShallowCopyData(PacMapList &desPacMap, const PacMapList &srcPacMap)
{
    desPacMap.clear();
    for (auto it = srcPacMap.begin(); it != srcPacMap.end(); it++) {
        desPacMap.emplace(it->first, it->second);
    }
}

void PacMap::RemoveData(PacMapList &pacMapList, const std::string &key)
{
    auto it = pacMapList.find(key);
    if (it != pacMapList.end()) {
        pacMapList.erase(it);
    }
}

bool PacMap::EqualPacMapData(const PacMapList &leftPacMapList, const PacMapList &rightPacMapList)
{
    if (leftPacMapList.size() != rightPacMapList.size()) {
        return false;
    }

    for (auto right = rightPacMapList.begin(); right != rightPacMapList.end(); right++) {
        auto left = leftPacMapList.find(right->first);
        if (left == leftPacMapList.end()) {
            return false;
        }
        if (left->second.GetRefPtr() == right->second.GetRefPtr()) {
            continue;
        }

        // PacMap Object
        if (IPacMap::Query(right->second.GetRefPtr()) != nullptr) {
            auto leftMapIt = leftPacMapList.find(right->first);
            if (leftMapIt == leftPacMapList.end()) {
                return false;
            }
            if (IPacMap::Query(leftMapIt->second.GetRefPtr()) == nullptr) {
                return false;
            }

            PacMap *rightMap = static_cast<PacMap *>(IPacMap::Query(right->second.GetRefPtr()));
            PacMap *leftMap = static_cast<PacMap *>(IPacMap::Query(leftMapIt->second.GetRefPtr()));
            if (rightMap == nullptr || leftMap == nullptr ||
                !EqualPacMapData(leftMap->dataList_, rightMap->dataList_)) {
                return false;
            }
            continue;
        }

        if (!Object::Equals(*(right->second.GetRefPtr()), *(left->second.GetRefPtr()))) {
            return false;
        }
    }
    return true;
}

template<typename iid, typename id, typename value>
static void GetArrayData(
    AAFwk::IInterface *interface, std::vector<value> &array, std::function<bool(IArray *)> IsArrayfunc)
{
    if (interface == nullptr) {
        return;
    }
    if (IsArrayfunc(IArray::Query(interface))) {
        auto func = [&array](IInterface *object) { array.push_back(id::Unbox(iid::Query(object))); };
        Array::ForEach(IArray::Query(interface), func);
    }
}

template<typename iid, typename id, typename value>
static bool CompareTwoArrayData(
    AAFwk::IInterface *one_interface, AAFwk::IInterface *two_interface, std::function<bool(IArray *)> IsArrayfunc)
{
    typename std::vector<value> array;
    GetArrayData<iid, id, value>(IArray::Query(one_interface), array, IsArrayfunc);

    if (!IsArrayfunc(IArray::Query(two_interface))) {
        return false;
    }
    typename std::vector<value> otherArray;
    GetArrayData<iid, id, value>(IArray::Query(two_interface), otherArray, IsArrayfunc);
    if (array.size() != 0 && otherArray.size() != 0 && array.size() != otherArray.size()) {
        return false;
    }
    for (std::size_t i = 0; i < array.size(); i++) {
        auto it = std::find(otherArray.begin(), otherArray.end(), array[i]);
        if (it == array.end()) {
            return false;
        }
    }
    return true;
}
bool PacMap::CompareArrayData(AAFwk::IInterface *one_interface, AAFwk::IInterface *two_interface)
{
    if (IArray::Query(one_interface) != nullptr && IArray::Query(two_interface) != nullptr) {
        IArray *array_one = IArray::Query(one_interface);
        IArray *array_two = IArray::Query(two_interface);
        long size1 = 0;
        array_one->GetLength(size1);
        long size2 = 0;
        array_two->GetLength(size2);

        if (size1 != 0 && size2 != 0 && size1 != size2) {
            return false;
        }
        if (AAFwk::Array::IsBooleanArray(IArray::Query(one_interface))) {
            if (!CompareTwoArrayData<AAFwk::IBoolean, AAFwk::Boolean, bool>(
                one_interface, two_interface, AAFwk::Array::IsBooleanArray)) {
                return false;
            }
        } else if (AAFwk::Array::IsCharArray(AAFwk::IArray::Query(one_interface))) {
            return false;
        } else if (AAFwk::Array::IsByteArray(IArray::Query(one_interface))) {
            if (!CompareTwoArrayData<AAFwk::IByte, AAFwk::Byte, byte>(
                one_interface, two_interface, AAFwk::Array::IsByteArray)) {
                return false;
            }
        } else if (AAFwk::Array::IsShortArray(IArray::Query(one_interface))) {
            if (!CompareTwoArrayData<AAFwk::IShort, AAFwk::Short, short>(
                one_interface, two_interface, AAFwk::Array::IsShortArray)) {
                return false;
            }
        } else if (AAFwk::Array::IsIntegerArray(IArray::Query(one_interface))) {
            if (!CompareTwoArrayData<AAFwk::IInteger, AAFwk::Integer, int>(
                one_interface, two_interface, AAFwk::Array::IsIntegerArray)) {
                return false;
            }
        } else if (AAFwk::Array::IsLongArray(IArray::Query(one_interface))) {
            if (!CompareTwoArrayData<AAFwk::ILong, AAFwk::Long, long>(
                one_interface, two_interface, AAFwk::Array::IsLongArray)) {
                return false;
            }
        } else if (AAFwk::Array::IsFloatArray(IArray::Query(one_interface))) {
            if (!CompareTwoArrayData<AAFwk::IFloat, AAFwk::Float, float>(
                one_interface, two_interface, AAFwk::Array::IsFloatArray)) {
                return false;
            }
        } else if (AAFwk::Array::IsDoubleArray(IArray::Query(one_interface))) {
            if (!CompareTwoArrayData<AAFwk::IDouble, AAFwk::Double, double>(
                one_interface, two_interface, AAFwk::Array::IsDoubleArray)) {
                return false;
            }
        } else if (AAFwk::Array::IsStringArray(IArray::Query(one_interface))) {
            if (!CompareTwoArrayData<AAFwk::IString, AAFwk::String, std::string>(
                one_interface, two_interface, AAFwk::Array::IsStringArray)) {
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}
/**
 * @brief Indicates whether some other object is "equal to" this one.
 * @param pacMap The object with which to compare.
 * @return Returns true if this object is the same as the pacMap argument; false otherwise.
 */
bool PacMap::Equals(const PacMap *pacMap)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    if (pacMap == nullptr) {
        return false;
    }

    if (this == pacMap) {
        return true;
    }

    if (dataList_.size() != pacMap->dataList_.size()) {
        return false;
    }

    if (!EqualPacMapData(dataList_, pacMap->dataList_)) {
        return false;
    }

    return true;
}

bool PacMap::Equals(const PacMap &pacMap)
{
    return Equals(&pacMap);
}

/**
 * @brief Checks whether the current object is empty.
 * @return If there is no data, return true, otherwise return false.
 */
bool PacMap::IsEmpty(void) const
{
    return dataList_.empty();
}

/**
 * @brief Obtains the number of key-value pairs stored in the current object.
 * @return Returns the number of key-value pairs.
 */
int PacMap::GetSize(void) const
{
    return dataList_.size();
}

/**
 * @brief Obtains all the keys of the current object.
 */
const std::set<std::string> PacMap::GetKeys(void)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    std::set<std::string> keys;

    for (auto it = dataList_.begin(); it != dataList_.end(); it++) {
        keys.emplace(it->first);
    }
    return keys;
}

/**
 * @brief Checks whether a specified key is contained.
 * @param key Indicates the key in String
 * @return Returns true if the key is contained; returns false otherwise.
 */
bool PacMap::HasKey(const std::string &key)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    return (dataList_.find(key) != dataList_.end());
}

/**
 * @brief Deletes a key-value pair with a specified key.
 * @param key Specifies the key of the deleted data.
 */
void PacMap::Remove(const std::string &key)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    auto it = dataList_.find(key);
    if (it != dataList_.end()) {
        dataList_.erase(it);
    }
}

bool PacMap::ReadFromParcel(Parcel &parcel)
{
    std::string value = parcel.ReadString();
    if (!value.empty()) {
        return StringToMapList(value, dataList_);
    } else {
        return true;
    }
}

/**
 * @brief Marshals this Sequenceable object to a Parcel.
 * @param parcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
 * @return Marshals success returns true, otherwise returns false.
 */
bool PacMap::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString("PACMAP")) {
        return false;
    }
    std::string str = MapListToString(dataList_);
    return parcel.WriteString(str);
}

/**
 * @brief Unmarshals this S`nceable object from a Parcel.
 * @param parcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
 * @return Unmarshals success returns a smart pointer to PacMap, otherwise returns nullptr.
 */
PacMap *PacMap::Unmarshalling(Parcel &parcel)
{
    std::string value = parcel.ReadString();
    if (value != "PACMAP") {
        return nullptr;
    }
    PacMap *pPacMap = new (std::nothrow) PacMap();
    if (pPacMap != nullptr && !pPacMap->ReadFromParcel(parcel)) {
        delete pPacMap;
        return nullptr;
    }
    return pPacMap;
}

/**
 * @brief Object serialization to string.
 * @return Returns the serialized string.
 */
std::string PacMap::ToString()
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    return MapListToString(dataList_);
}

std::string PacMap::MapListToString(const PacMapList &mapList) const
{
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        return "";
    }
    cJSON *dataObject = nullptr;
    if (mapList.size() == 0 || !ToJson(mapList, dataObject)) {
        cJSON_AddNullToObject(root, "pacmap");
    } else {
        cJSON_AddItemToObject(root, "pacmap", dataObject);
    }

    char *str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (str == nullptr) {
        return "";
    }
    std::string jsonStr(str);
    cJSON_free(str);

    return jsonStr;
}

bool PacMap::ToJson(const PacMapList &mapList, cJSON *&dataObject) const
{
    dataObject = cJSON_CreateObject();
    if (dataObject == nullptr) {
        return false;
    }
    for (auto it = mapList.begin(); it != mapList.end(); it++) {
        cJSON *item = cJSON_CreateObject();
        if (item == nullptr) {
            cJSON_Delete(dataObject);
            return false;
        }
        bool isOK = false;
        if (IPacMap::Query(it->second.GetRefPtr()) != nullptr) {
            PacMap *pacmap = static_cast<PacMap *>(IPacMap::Query(it->second.GetRefPtr()));
            if (pacmap == nullptr) {
                continue;
            }
            cJSON *item2 = nullptr;
            isOK = pacmap->ToJson(pacmap->dataList_, item2);
            if (isOK) {
                cJSON_AddNumberToObject(item, "type", static_cast<double>(PACMAP_DATA_PACMAP));
                cJSON_AddItemToObject(item, "data", item2);
            } else {
                cJSON_Delete(item2);
            }
        } else {
            isOK = GetBaseJsonValue(it, item);
        }
        if (isOK) {
            cJSON_AddItemToObject(dataObject, it->first.c_str(), item);
        } else {
            cJSON_Delete(dataObject);
            cJSON_Delete(item);
            return false;
        }
    }
    return true;
}

template<typename RawType>
static std::string RawTypeToString(const RawType value, unsigned int precisionAfterPoint)
{
    std::ostringstream out("RawTypeToString");
    out << std::setw(0) << std::setprecision(precisionAfterPoint) << value;

    std::string res = out.str();
    auto pos = res.find('.');
    if (pos == std::string::npos) {
        return res;
    }

    auto splitLen = pos + 1 + precisionAfterPoint;
    if (res.size() <= splitLen) {
        return res;
    }

    return res.substr(0, splitLen);
}

bool PacMap::GetBaseJsonValue(PacMapList::const_iterator &it, cJSON *&json) const
{
    // base data  : short
    if (IShort::Query(it->second.GetRefPtr()) != nullptr) {
        GetBaseDataValue<IShort, short>(it->second.GetRefPtr(), json, PACMAP_DATA_SHORT);
    } else if (IInteger::Query(it->second.GetRefPtr()) != nullptr) {
        GetBaseDataValue<IInteger, int>(it->second.GetRefPtr(), json, PACMAP_DATA_INTEGER);
    } else if (ILong::Query(it->second.GetRefPtr()) != nullptr) {
        // long:string
        GetBaseLongDataValue(it->second.GetRefPtr(), json, PACMAP_DATA_LONG);
    } else if (IChar::Query(it->second.GetRefPtr()) != nullptr) {
        GetBaseDataValue<IChar, zchar>(it->second.GetRefPtr(), json, PACMAP_DATA_CHAR);
    } else if (IByte::Query(it->second.GetRefPtr()) != nullptr) {
        GetBaseDataValue<IByte, byte>(it->second.GetRefPtr(), json, PACMAP_DATA_BYTE);
    } else if (IBoolean::Query(it->second.GetRefPtr()) != nullptr) {
        GetBaseBoolDataValue(it->second.GetRefPtr(), json, PACMAP_DATA_BOOLEAN);
    } else if (IFloat::Query(it->second.GetRefPtr()) != nullptr) {
        // base long:string
        GetBaseFloatDoubleDataValue<IFloat, Float, float>(
            it->second.GetRefPtr(), json, PACMAP_DATA_FLOAT, FLOAT_PRECISION);
    } else if (IDouble::Query(it->second.GetRefPtr()) != nullptr) {
        // base :double to string
        GetBaseFloatDoubleDataValue<IDouble, Double, double>(
            it->second.GetRefPtr(), json, PACMAP_DATA_DOUBLE, DOUBLE_PRECISION);
    } else if (IString::Query(it->second.GetRefPtr()) != nullptr) {
        GetBaseStringDataValue(it->second.GetRefPtr(), json, PACMAP_DATA_STRING);
    } else if (IArray::Query(it->second.GetRefPtr()) != nullptr) {
        // array data
        return GetArrayJsonValue(it, json);
    } else if (IUserObject::Query(it->second.GetRefPtr()) != nullptr) {
        // Object data [UserObject--data:UserObjectBase]
        return GetUserObjectJsonValue(it, json);
    } else {
        return false;
    }

    return true;
}

// for short / int / byte
template<typename T>
static bool PacmapToJsonArrayNumber(std::vector<T> &array, cJSON *&item, int type)
{
    ABILITYBASE_LOGD("start");
    if (array.size() > 0) {
        item = cJSON_CreateObject();
        if (item == nullptr) {
            return false;
        }
        cJSON *datasItem = cJSON_CreateArray();
        if (datasItem == nullptr) {
            cJSON_Delete(item);
            return false;
        }
        for (size_t i = 0; i < array.size(); i++) {
            cJSON *dataItem = cJSON_CreateNumber(static_cast<double>(array[i]));
            if (dataItem == nullptr) {
                cJSON_Delete(item);
                cJSON_Delete(datasItem);
                return false;
            }
            cJSON_AddItemToArray(datasItem, dataItem);
        }
        cJSON_AddItemToObject(item, "data", datasItem);
        cJSON_AddNumberToObject(item, "type", static_cast<double>(type));
        return true;
    }
    return false;
}

// for float / double
template<typename T>
static bool PacmapToJsonArrayFloatOrDouble(std::vector<T> &array, cJSON *&item, int type, int precision)
{
    ABILITYBASE_LOGD("start");
    if (array.size() > 0) {
        if (precision != FLOAT_PRECISION && precision != DOUBLE_PRECISION) {
            return false;
        }
        item = cJSON_CreateObject();
        if (item == nullptr) {
            return false;
        }
        cJSON *datasItem = cJSON_CreateArray();
        if (datasItem == nullptr) {
            cJSON_Delete(item);
            return false;
        }
        for (size_t i = 0; i < array.size(); i++) {
            std::string data = RawTypeToString<T>(array[i], precision);
            cJSON *dataItem = cJSON_CreateString(data.c_str());
            if (dataItem == nullptr) {
                cJSON_Delete(item);
                cJSON_Delete(datasItem);
                return false;
            }
            cJSON_AddItemToArray(datasItem, dataItem);
        }
        cJSON_AddItemToObject(item, "data", datasItem);
        cJSON_AddNumberToObject(item, "type", static_cast<double>(type));
        return true;
    }
    return false;
}

// Base data: short
bool PacMap::ToJsonArrayShort(std::vector<short> &array, cJSON *&item, int type) const
{
    return PacmapToJsonArrayNumber(array, item, type);
}
// Base data: Integer
bool PacMap::ToJsonArrayInt(std::vector<int> &array, cJSON *&item, int type) const
{
    return PacmapToJsonArrayNumber(array, item, type);
}
// Base data: long:sting
bool PacMap::ToJsonArrayLong(std::vector<long> &array, cJSON *&item, int type) const
{
    ABILITYBASE_LOGD("start");
    if (array.size() > 0) {
        item = cJSON_CreateObject();
        if (item == nullptr) {
            return false;
        }
        cJSON *datasItem = cJSON_CreateArray();
        if (datasItem == nullptr) {
            cJSON_Delete(item);
            return false;
        }
        for (size_t i = 0; i < array.size(); i++) {
            cJSON *dataItem = cJSON_CreateString(std::to_string(array[i]).c_str());
            if (dataItem == nullptr) {
                cJSON_Delete(item);
                cJSON_Delete(datasItem);
                return false;
            }
            cJSON_AddItemToArray(datasItem, dataItem);
        }
        cJSON_AddItemToObject(item, "data", datasItem);
        cJSON_AddNumberToObject(item, "type", static_cast<double>(type));
        return true;
    }
    return false;
}

// Base data: byte
bool PacMap::ToJsonArrayByte(std::vector<byte> &array, cJSON *&item, int type) const
{
    return PacmapToJsonArrayNumber(array, item, type);
}
// Base data: bool
bool PacMap::ToJsonArrayBoolean(std::vector<bool> &array, cJSON *&item, int type) const
{
    ABILITYBASE_LOGD("start");
    if (array.size() > 0) {
        item = cJSON_CreateObject();
        if (item == nullptr) {
            return false;
        }
        cJSON *datasItem = cJSON_CreateArray();
        if (datasItem == nullptr) {
            cJSON_Delete(item);
            return false;
        }
        for (size_t i = 0; i < array.size(); i++) {
            cJSON *dataItem = cJSON_CreateBool(array[i]);
            if (dataItem == nullptr) {
                cJSON_Delete(item);
                cJSON_Delete(datasItem);
                return false;
            }
            cJSON_AddItemToArray(datasItem, dataItem);
        }
        cJSON_AddItemToObject(item, "data", datasItem);
        cJSON_AddNumberToObject(item, "type", static_cast<double>(type));
        return true;
    }
    return false;
}
// Base data: Float to string
bool PacMap::ToJsonArrayFloat(std::vector<float> &array, cJSON *&item, int type) const
{
    return PacmapToJsonArrayFloatOrDouble(array, item, type, FLOAT_PRECISION);
}
// Base data: Double to string
bool PacMap::ToJsonArrayDouble(std::vector<double> &array, cJSON *&item, int type) const
{
    return PacmapToJsonArrayFloatOrDouble(array, item, type, DOUBLE_PRECISION);
}
// Base data: string
bool PacMap::ToJsonArrayString(std::vector<std::string> &array, cJSON *&item, int type) const
{
    ABILITYBASE_LOGD("start");
    if (array.size() > 0) {
        item = cJSON_CreateObject();
        if (item == nullptr) {
            return false;
        }
        cJSON *datasItem = cJSON_CreateArray();
        if (datasItem == nullptr) {
            cJSON_Delete(item);
            return false;
        }
        for (size_t i = 0; i < array.size(); i++) {
            cJSON *dataItem = cJSON_CreateString(array[i].c_str());
            if (dataItem == nullptr) {
                cJSON_Delete(item);
                cJSON_Delete(datasItem);
                return false;
            }
            cJSON_AddItemToArray(datasItem, dataItem);
        }
        cJSON_AddItemToObject(item, "data", datasItem);
        cJSON_AddNumberToObject(item, "type", static_cast<double>(type));
        return true;
    }
    return false;
}

bool PacMap::GetArrayJsonValue(PacMapList::const_iterator &it, cJSON *&json) const
{
    if (IArray::Query(it->second.GetRefPtr()) == nullptr) {
        return false;
    }
    IArray *array = static_cast<IArray *>(it->second.GetRefPtr());
    if (array == nullptr) {
        return false;
    }
    if (Array::IsShortArray(array)) {
        std::vector<short> arrayData;
        PacmapGetArrayVal<AAFwk::IShort, AAFwk::Short, short>(it->second.GetRefPtr(), arrayData);
        return ToJsonArrayShort(arrayData, json, PACMAP_DATA_ARRAY_SHORT);
    } else if (Array::IsIntegerArray(array)) {
        std::vector<int> arrayData;
        PacmapGetArrayVal<AAFwk::IInteger, AAFwk::Integer, int>(it->second.GetRefPtr(), arrayData);
        return ToJsonArrayInt(arrayData, json, PACMAP_DATA_ARRAY_INTEGER);
    } else if (Array::IsLongArray(array)) {
        std::vector<long> arrayData;
        PacmapGetArrayVal<AAFwk::ILong, AAFwk::Long, long>(it->second.GetRefPtr(), arrayData);
        return ToJsonArrayLong(arrayData, json, PACMAP_DATA_ARRAY_LONG);
    } else if (Array::IsCharArray(array)) {
        return false;
    } else if (Array::IsByteArray(array)) {
        std::vector<byte> arrayData;
        PacmapGetArrayVal<AAFwk::IByte, AAFwk::Byte, byte>(it->second.GetRefPtr(), arrayData);
        return ToJsonArrayByte(arrayData, json, PACMAP_DATA_ARRAY_BYTE);
    } else if (Array::IsBooleanArray(array)) {
        std::vector<bool> arrayData;
        PacmapGetArrayVal<AAFwk::IBoolean, AAFwk::Boolean, bool>(it->second.GetRefPtr(), arrayData);
        return ToJsonArrayBoolean(arrayData, json, PACMAP_DATA_ARRAY_BOOLEAN);
    } else if (Array::IsFloatArray(array)) {
        std::vector<float> arrayData;
        PacmapGetArrayVal<AAFwk::IFloat, AAFwk::Float, float>(it->second.GetRefPtr(), arrayData);
        return ToJsonArrayFloat(arrayData, json, PACMAP_DATA_ARRAY_FLOAT);
    } else if (Array::IsDoubleArray(array)) {
        std::vector<double> arrayData;
        PacmapGetArrayVal<AAFwk::IDouble, AAFwk::Double, double>(it->second.GetRefPtr(), arrayData);
        return ToJsonArrayDouble(arrayData, json, PACMAP_DATA_ARRAY_DOUBLE);
    } else if (Array::IsStringArray(array)) {
        std::vector<std::string> arrayData;
        PacmapGetArrayVal<AAFwk::IString, AAFwk::String, std::string>(it->second.GetRefPtr(), arrayData);
        return ToJsonArrayString(arrayData, json, PACMAP_DATA_ARRAY_STRING);
    } else {
        return false;
    }
    return true;
}
bool PacMap::GetUserObjectJsonValue(PacMapList::const_iterator &it, cJSON *&json) const
{
    std::shared_ptr<UserObjectBase> myObjectBase = UserObject::Unbox(IUserObject::Query(it->second.GetRefPtr()));
    if (myObjectBase == nullptr) {
        return false;
    }
    json = cJSON_CreateObject();
    if (json == nullptr) {
        return false;
    }
    std::string userObjectString = myObjectBase->ToString();
    cJSON_AddNumberToObject(json, "type", static_cast<double>(PACMAP_DATA_USEROBJECT));
    cJSON_AddStringToObject(json, "class", myObjectBase->GetClassName().c_str());
    cJSON_AddStringToObject(json, "data", userObjectString.c_str());
    return true;
}

/**
 * @brief Restore pacmap from the string.
 * @return Return true if successful, otherwise false.
 */
bool PacMap::FromString(const std::string &str)
{
    std::lock_guard<std::mutex> mLock(mapLock_);
    dataList_.clear();
    return StringToMapList(str, dataList_);
}

bool PacMap::StringToMapList(const std::string &str, PacMapList &mapList)
{
    if (str.empty()) {
        return false;
    }

    cJSON *root = cJSON_Parse(str.c_str());
    if (root == nullptr) {
        return false;
    }
    if (!cJSON_IsObject(root)) {
        cJSON_Delete(root);
        return false;
    }

    cJSON *pacmapItem = cJSON_GetObjectItem(root, "pacmap");
    if (pacmapItem == nullptr) {
        cJSON_Delete(root);
        return false;
    }
    if (cJSON_IsNull(pacmapItem)) {
        cJSON_Delete(root);
        return true;
    }
    bool ret = ParseJson(pacmapItem, mapList);
    cJSON_Delete(root);
    return ret;
}

bool PacMap::ParseJson(const cJSON *data, PacMapList &mapList)
{
    if (data == nullptr || !cJSON_IsObject(data)) {
        return false;
    }
    cJSON *childItem = data->child;
    while (childItem != nullptr) {
        std::string key = childItem->string == nullptr ? "" : childItem->string;
        if (cJSON_IsObject(childItem) && JudgeType(childItem)) {
            if (!ParseJsonItem(mapList, key, childItem)) {
                return false;
            }
        }
        childItem = childItem->next;
    }
    return true;
}

bool PacMap::JudgeType(cJSON *item)
{
    cJSON *typeItem = cJSON_GetObjectItem(item, "type");
    if (typeItem != nullptr && cJSON_IsNumber(typeItem)) {
        cJSON *dataItem = cJSON_GetObjectItem(item, "data");
        int type = static_cast<int>(typeItem->valuedouble);
        switch (type) {
            case PACMAP_DATA_SHORT:
            case PACMAP_DATA_INTEGER:
            case PACMAP_DATA_CHAR:
            case PACMAP_DATA_BYTE:
                return dataItem != nullptr && cJSON_IsNumber(dataItem);
            case PACMAP_DATA_LONG:
            case PACMAP_DATA_FLOAT:
            case PACMAP_DATA_DOUBLE:
            case PACMAP_DATA_STRING:
                return dataItem != nullptr && cJSON_IsString(dataItem);
            case PACMAP_DATA_BOOLEAN:
                return dataItem != nullptr && cJSON_IsBool(dataItem);
            case PACMAP_DATA_USEROBJECT: {
                    cJSON *classItem = cJSON_GetObjectItem(item, "class");
                    return dataItem != nullptr && cJSON_IsString(dataItem) &&
                        classItem != nullptr && cJSON_IsString(classItem);
                }
            case PACMAP_DATA_PACMAP:
                return true;
            default:
                return dataItem != nullptr && cJSON_IsArray(dataItem);
        }
    }
    return false;
}

bool PacMap::ParseJsonItem(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    // base data, object data, arry data
    cJSON *typeItem = cJSON_GetObjectItem(item, "type");
    cJSON *dataItem = cJSON_GetObjectItem(item, "data");
    if (typeItem == nullptr || !cJSON_IsNumber(typeItem)) {
        return false;
    }
    int type = static_cast<int>(typeItem->valuedouble);
    switch (type) {
        case PACMAP_DATA_SHORT:
            if (dataItem == nullptr || !cJSON_IsNumber(dataItem)) {
                return false;
            }
            InnerPutShortValue(mapList, key, static_cast<short>(dataItem->valuedouble));
            break;
        case PACMAP_DATA_INTEGER:
            if (dataItem == nullptr || !cJSON_IsNumber(dataItem)) {
                return false;
            }
            InnerPutIntValue(mapList, key, static_cast<int>(dataItem->valuedouble));
            break;
        case PACMAP_DATA_LONG:
            if (dataItem == nullptr || !cJSON_IsString(dataItem)) {
                return false;
            }
            InnerPutLongValue(mapList, key, std::atol(dataItem->valuestring));
            break;
        case PACMAP_DATA_CHAR:
            if (dataItem == nullptr || !cJSON_IsNumber(dataItem)) {
                return false;
            }
            InnerPutCharValue(mapList, key, static_cast<char>(dataItem->valuedouble));
            break;
        case PACMAP_DATA_BYTE:
            if (dataItem == nullptr || !cJSON_IsNumber(dataItem)) {
                return false;
            }
            InnerPutByteValue(mapList, key, static_cast<AAFwk::byte>(dataItem->valuedouble));
            break;
        case PACMAP_DATA_BOOLEAN:
            if (dataItem == nullptr || !cJSON_IsBool(dataItem)) {
                return false;
            }
            InnerPutBooleanValue(mapList, key, dataItem->type == cJSON_True ? true : false);
            break;
        case PACMAP_DATA_FLOAT:
            if (dataItem == nullptr || !cJSON_IsString(dataItem)) {
                return false;
            }
            InnerPutFloatValue(mapList, key, std::atof(dataItem->valuestring));
            break;
        case PACMAP_DATA_DOUBLE:
            if (dataItem == nullptr || !cJSON_IsString(dataItem)) {
                return false;
            }
            InnerPutDoubleValue(mapList, key, std::atof(dataItem->valuestring));
            break;
        case PACMAP_DATA_STRING:
            if (dataItem == nullptr || !cJSON_IsString(dataItem)) {
                return false;
            }
            InnerPutStringValue(mapList, key, std::string(dataItem->valuestring));
            break;
        case PACMAP_DATA_USEROBJECT:
            InnerPutObjectValue(mapList, key, item);
            break;
        case PACMAP_DATA_PACMAP:
            InnerPutPacMapValue(mapList, key, item);
            break;
        default:
            return ParseJsonItemArray(mapList, key, item);
    }
    return true;
}

bool PacMap::ParseJsonItemArray(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    cJSON *typeItem = cJSON_GetObjectItem(item, "type");
    if (typeItem == nullptr || !cJSON_IsNumber(typeItem)) {
        return false;
    }
    int type = static_cast<int>(typeItem->valuedouble);
    switch (type) {
        case PACMAP_DATA_ARRAY_SHORT:
            return ParseJsonItemArrayShort(mapList, key, item);
        case PACMAP_DATA_ARRAY_INTEGER:
            return ParseJsonItemArrayInteger(mapList, key, item);
        case PACMAP_DATA_ARRAY_LONG:
            return ParseJsonItemArrayLong(mapList, key, item);
        case PACMAP_DATA_ARRAY_CHAR:
            return ParseJsonItemArrayChar(mapList, key, item);
        case PACMAP_DATA_ARRAY_BYTE:
            return ParseJsonItemArrayByte(mapList, key, item);
        case PACMAP_DATA_ARRAY_BOOLEAN:
            return ParseJsonItemArrayBoolean(mapList, key, item);
        case PACMAP_DATA_ARRAY_FLOAT:
            return ParseJsonItemArrayFloat(mapList, key, item);
        case PACMAP_DATA_ARRAY_DOUBLE:
            return ParseJsonItemArrayDouble(mapList, key, item);
        case PACMAP_DATA_ARRAY_STRING:
            return ParseJsonItemArrayString(mapList, key, item);
        default:
            return false;
    }
}

bool PacMap::ParseJsonItemArrayShort(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    cJSON *datasItem = cJSON_GetObjectItem(item, "data");
    if (datasItem == nullptr) {
        return true;
    }
    if (!cJSON_IsArray(datasItem)) {
        return false;
    }
    int size = cJSON_GetArraySize(datasItem);
    if (size < 0 || size > MAX_ARRAY_ALLOW_SIZE) {
        return false;
    }
    std::vector<short> shortList;
    for (int i = 0; i < size; i++) {
        cJSON *dataItem = cJSON_GetArrayItem(datasItem, i);
        if (dataItem == nullptr || !cJSON_IsNumber(dataItem)) {
            return false;
        }
        short data = static_cast<short>(dataItem->valuedouble);
        shortList.push_back(data);
    }
    InnerPutShortValueArray(mapList, key, shortList);
    return true;
}

bool PacMap::ParseJsonItemArrayInteger(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    cJSON *datasItem = cJSON_GetObjectItem(item, "data");
    if (datasItem == nullptr) {
        return true;
    }
    if (!cJSON_IsArray(datasItem)) {
        return false;
    }
    int size = cJSON_GetArraySize(datasItem);
    if (size < 0 || size > MAX_ARRAY_ALLOW_SIZE) {
        return false;
    }
    std::vector<int> intList;
    for (int i = 0; i < size; i++) {
        cJSON *dataItem = cJSON_GetArrayItem(datasItem, i);
        if (dataItem == nullptr || !cJSON_IsNumber(dataItem)) {
            return false;
        }
        int data = static_cast<int>(dataItem->valuedouble);
        intList.push_back(data);
    }
    InnerPutIntValueArray(mapList, key, intList);
    return true;
}
/**
 * @brief Determine whether the string content is a numeric string
 * @param str indicates stirng.
 * @return bool
 */
bool PacMap::IsNumber(const std::string &str)
{
    try {
        if (std::regex_match(str, NUMBER_REGEX)) {
            return true;
        }
        return false;
    } catch (const std::regex_error& e) {
        return false;
    }
}

bool PacMap::ParseJsonItemArrayLong(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    cJSON *datasItem = cJSON_GetObjectItem(item, "data");
    if (datasItem == nullptr) {
        return true;
    }
    if (!cJSON_IsArray(datasItem)) {
        return false;
    }
    int size = cJSON_GetArraySize(datasItem);
    if (size < 0 || size > MAX_ARRAY_ALLOW_SIZE) {
        return false;
    }
    std::vector<long> longList;
    for (int i = 0; i < size; i++) {
        cJSON *dataItem = cJSON_GetArrayItem(datasItem, i);
        if (dataItem == nullptr || !cJSON_IsString(dataItem)) {
            return false;
        }
        std::string data = dataItem->valuestring;
        if (!IsNumber(data)) {
            return false;
        }
        long longValue = std::atol(data.c_str());
        longList.push_back(longValue);
    }
    InnerPutLongValueArray(mapList, key, longList);
    return true;
}

bool PacMap::ParseJsonItemArrayChar(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    cJSON *datasItem = cJSON_GetObjectItem(item, "data");
    if (datasItem == nullptr) {
        return true;
    }
    if (!cJSON_IsArray(datasItem)) {
        return false;
    }
    int size = cJSON_GetArraySize(datasItem);
    if (size < 0 || size > MAX_ARRAY_ALLOW_SIZE) {
        return false;
    }
    std::vector<char> charList;
    for (int i = 0; i < size; i++) {
        cJSON *dataItem = cJSON_GetArrayItem(datasItem, i);
        if (dataItem == nullptr || !cJSON_IsNumber(dataItem)) {
            return false;
        }
        char data = static_cast<char>(dataItem->valuedouble);
        charList.push_back(data);
    }
    InnerPutCharValueArray(mapList, key, charList);
    return true;
}

bool PacMap::ParseJsonItemArrayByte(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    cJSON *datasItem = cJSON_GetObjectItem(item, "data");
    if (datasItem == nullptr) {
        return true;
    }
    if (!cJSON_IsArray(datasItem)) {
        return false;
    }
    int size = cJSON_GetArraySize(datasItem);
    if (size < 0 || size > MAX_ARRAY_ALLOW_SIZE) {
        return false;
    }
    std::vector<AAFwk::byte> byteList;
    for (int i = 0; i < size; i++) {
        cJSON *dataItem = cJSON_GetArrayItem(datasItem, i);
        if (dataItem == nullptr || !cJSON_IsNumber(dataItem)) {
            return false;
        }
        AAFwk::byte data = static_cast<AAFwk::byte>(dataItem->valuedouble);
        byteList.push_back(data);
    }
    InnerPutByteValueArray(mapList, key, byteList);
    return true;
}

bool PacMap::ParseJsonItemArrayBoolean(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    cJSON *datasItem = cJSON_GetObjectItem(item, "data");
    if (datasItem == nullptr) {
        return true;
    }
    if (!cJSON_IsArray(datasItem)) {
        return false;
    }
    int size = cJSON_GetArraySize(datasItem);
    if (size < 0 || size > MAX_ARRAY_ALLOW_SIZE) {
        return false;
    }
    std::vector<bool> boolList;
    for (int i = 0; i < size; i++) {
        cJSON *dataItem = cJSON_GetArrayItem(datasItem, i);
        if (dataItem == nullptr) {
            return false;
        }
        bool data = false;
        if (cJSON_IsBool(dataItem)) {
            data = dataItem->type == cJSON_True ? true : false;
        } else {
            return false;
        }
        boolList.push_back(data);
    }
    InnerPutBooleanValueArray(mapList, key, boolList);
    return true;
}

bool PacMap::ParseJsonItemArrayFloat(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    cJSON *datasItem = cJSON_GetObjectItem(item, "data");
    if (datasItem == nullptr) {
        return true;
    }
    if (!cJSON_IsArray(datasItem)) {
        return false;
    }
    int size = cJSON_GetArraySize(datasItem);
    if (size < 0 || size > MAX_ARRAY_ALLOW_SIZE) {
        return false;
    }
    std::vector<float> floatList;
    for (int i = 0; i < size; i++) {
        cJSON *dataItem = cJSON_GetArrayItem(datasItem, i);
        if (dataItem == nullptr || !cJSON_IsString(dataItem)) {
            return false;
        }
        std::string data = dataItem->valuestring;
        floatList.push_back(std::atof(data.c_str()));
    }
    InnerPutFloatValueArray(mapList, key, floatList);
    return true;
}

bool PacMap::ParseJsonItemArrayDouble(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    cJSON *datasItem = cJSON_GetObjectItem(item, "data");
    if (datasItem == nullptr) {
        return true;
    }
    if (!cJSON_IsArray(datasItem)) {
        return false;
    }
    int size = cJSON_GetArraySize(datasItem);
    if (size < 0 || size > MAX_ARRAY_ALLOW_SIZE) {
        return false;
    }
    std::vector<double> doubleList;
    for (int i = 0; i < size; i++) {
        cJSON *dataItem = cJSON_GetArrayItem(datasItem, i);
        if (dataItem == nullptr || !cJSON_IsString(dataItem)) {
            return false;
        }
        std::string data = dataItem->valuestring;
        doubleList.push_back(std::atof(data.c_str()));
    }
    InnerPutDoubleValueArray(mapList, key, doubleList);
    return true;
}

bool PacMap::ParseJsonItemArrayString(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    cJSON *datasItem = cJSON_GetObjectItem(item, "data");
    if (datasItem == nullptr) {
        return true;
    }
    if (!cJSON_IsArray(datasItem)) {
        return false;
    }
    int size = cJSON_GetArraySize(datasItem);
    if (size < 0 || size > MAX_ARRAY_ALLOW_SIZE) {
        return false;
    }
    std::vector<std::string> stringList;
    for (int i = 0; i < size; i++) {
        cJSON *dataItem = cJSON_GetArrayItem(datasItem, i);
        if (dataItem == nullptr || !cJSON_IsString(dataItem)) {
            return false;
        }
        std::string data = dataItem->valuestring;
        stringList.push_back(data);
    }
    InnerPutStringValueArray(mapList, key, stringList);
    return true;
}

bool PacMap::InnerPutObjectValue(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    cJSON *classItem = cJSON_GetObjectItem(item, "class");
    if (classItem == nullptr || !cJSON_IsString(classItem)) {
        return false;
    }
    std::string className = classItem->valuestring;
    if (className.empty()) {
        return false;
    }

    UserObjectBase *userObjectIns = UserObjectBaseLoader::GetInstance().GetUserObjectByName(className);
    if (userObjectIns == nullptr) {
        return false;
    }

    cJSON *dataItem = cJSON_GetObjectItem(item, "data");
    if (dataItem == nullptr || !cJSON_IsString(dataItem)) {
        return false;
    }
    std::string userObjectString = dataItem->valuestring;
    if (!userObjectString.empty()) {
        userObjectIns->Parse(userObjectString);
    }

    std::shared_ptr<UserObjectBase> userObject(userObjectIns);
    InnerPutObject(mapList, key, userObject);
    return true;
}

bool PacMap::InnerPutPacMapValue(PacMapList &mapList, const std::string &key, const cJSON *item)
{
    cJSON *dataItem = cJSON_GetObjectItem(item, "data");
    if (dataItem == nullptr) {
        return false;
    }

    PacMap *p = new (std::nothrow) PacMap();
    if (p == nullptr) {
        return false;
    }
    sptr<IPacMap> sp = p;
    if (p->ParseJson(dataItem, p->dataList_)) {
        mapList.emplace(key, sp);
        return true;
    }

    return false;
}

bool PacMap::Equals(IObject &other)
{
    PacMap *otherObj = static_cast<PacMap *>(IPacMap::Query(&other));
    if (otherObj == nullptr) {
        return false;
    }

    return Equals(otherObj);
}

sptr<IPacMap> PacMap::Parse(const std::string &str)
{
    PacMap *pacmap = new (std::nothrow) PacMap();
    if (pacmap != nullptr) {
        pacmap->StringToMapList(str, pacmap->dataList_);
    }
    sptr<IPacMap> ret = pacmap;

    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
