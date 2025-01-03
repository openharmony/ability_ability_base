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

#ifndef OHOS_ABILITY_BASE_ZIP_FILE_READER_IO_H
#define OHOS_ABILITY_BASE_ZIP_FILE_READER_IO_H

#include <string>

#include "zip_file_reader.h"

namespace OHOS {
namespace AbilityBase {
class ZipFileReaderIo : public ZipFileReader {
public:
    ZipFileReaderIo(const std::string &filePath) : ZipFileReader(filePath) {}
    ~ZipFileReaderIo() = default;

    std::string ReadBuffer(size_t startPos, size_t bufferSize) override;
    bool ReadBuffer(uint8_t *dst, size_t startPos, size_t bufferSize) override;
};
}
}
#endif