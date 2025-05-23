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

#include "file_mapper.h"
#include <memory>
#include <sys/mman.h>

#include "ability_base_log_wrapper.h"
#include "zip_file_reader.h"

namespace OHOS {
namespace AbilityBase {
namespace {
long g_pageSize = 0;
const int32_t MAP_XPM = 0x40;
}
FileMapper::FileMapper()
{
    if (g_pageSize <= 0) {
        g_pageSize = sysconf(_SC_PAGESIZE);
    }
}

FileMapper::~FileMapper()
{
    if (basePtr_ != nullptr && autoReleaseMem_) {
        munmap(basePtr_, baseLen_);
    }
}

bool FileMapper::CreateFileMapper(const std::string &fileName, bool compress,
    int32_t fd, size_t offset, size_t len, FileMapperType type)
{
    if (fd < 0 || len == 0 || type == FileMapperType::NORMAL_MEM) {
        ABILITYBASE_LOGE("Invalid param fileName: %{public}s", fileName.c_str());
        return false;
    }
    if (g_pageSize <= 0) {
        ABILITYBASE_LOGE("Wrong Pagesize: %{public}ld", g_pageSize);
        return false;
    }
    if (dataLen_ > 0) {
        ABILITYBASE_LOGE("data not empty fileName: %{public}s", fileName_.c_str());
        return false;
    }

    size_t adjust = offset % static_cast<size_t>(g_pageSize);
    size_t adjOffset = offset - adjust;
    baseLen_ = len + adjust;
    int32_t mmapFlag = MAP_PRIVATE | MAP_XPM;
    if (type == FileMapperType::SHARED_MMAP) {
        mmapFlag = MAP_SHARED;
        autoReleaseMem_ = true;
    }
    basePtr_ = (uint8_t*)mmap(nullptr, baseLen_, PROT_READ,
        mmapFlag, fd, adjOffset);
    if (basePtr_ == MAP_FAILED) {
        ABILITYBASE_LOGE("mmap failed, errno[%{public}d]. fileName: %{public}s, "
            "offset: %{public}zu, pageSize: %{public}ld, mmapFlag: %{public}d",
            errno, fileName.c_str(), offset, g_pageSize, mmapFlag);
        baseLen_ = 0;
        return false;
    }

    isCompressed = compress;
    fileName_ = fileName;
    offset_ = offset;
    dataLen_ = len;
    usePtr_ = reinterpret_cast<uint8_t *>(basePtr_) + adjust;
    return true;
}

bool FileMapper::CreateFileMapper(std::shared_ptr<ZipFileReader> fileReader, const std::string &fileName,
    size_t offset, size_t len, bool compress)
{
    if (!fileReader) {
        ABILITYBASE_LOGE("file null fileName: %{public}s", fileName.c_str());
        return false;
    }
    if (!fileName_.empty()) {
        ABILITYBASE_LOGE("data not empty fileName: %{public}s", fileName_.c_str());
        return false;
    }

    dataPtr_ = std::make_unique<uint8_t[]>(len);
    if (!fileReader->ReadBuffer(dataPtr_.get(), offset, len)) {
        ABILITYBASE_LOGE("read failed, len[%{public}zu]. fileName: %{public}s, "
            "offset: %{public}zu", len, fileName.c_str(), offset);
        dataPtr_.reset();
        return false;
    }
    isCompressed = compress;
    dataLen_ = len;
    offset_ = offset;
    fileName_ = fileName;

    return true;
}

bool FileMapper::IsCompressed()
{
    return isCompressed;
}

uint8_t* FileMapper::GetDataPtr()
{
    return dataPtr_ ? dataPtr_.get() : usePtr_;
}

size_t FileMapper::GetDataLen()
{
    return dataLen_;
}

std::string FileMapper::GetFileName()
{
    return fileName_;
}

int32_t FileMapper::GetOffset()
{
    return offset_;
}
}  // namespace AbilityBase
}  // namespace OHOS
