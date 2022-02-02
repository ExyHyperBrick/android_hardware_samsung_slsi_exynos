/*
 * Copyright (c) 2018 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __TEE_SERVICE_GP_OPEN_SESSION_IN_H__
#define __TEE_SERVICE_GP_OPEN_SESSION_IN_H__

#include <vector>

#include <binder/Parcel.h>

#include "TeeServiceGpOperationIn.h"

namespace vendor {

namespace trustonic {

namespace teeservice {

struct TeeServiceGpOpenSessionIn : public ::android::Parcelable {
    uint64_t context = 0;
    uint64_t session = 0;
    std::vector<uint8_t> uuid;
    uint32_t login_type = 0;
    std::vector<uint8_t> login_data;
    TeeServiceGpOperation operation;

    virtual ::android::status_t writeToParcel(::android::Parcel* parcel) const {
        parcel->writeUint64(context);
        parcel->writeUint64(session);
        parcel->writeByteVector(uuid);
        parcel->writeUint32(login_type);
        parcel->writeByteVector(login_data);
        TeeServiceGpOperationIn::writeToParcel(operation, parcel);
        return ::android::OK;
    }

    virtual ::android::status_t readFromParcel(const ::android::Parcel* parcel) {
        parcel->readUint64(&context);
        parcel->readUint64(&session);
        parcel->readByteVector(&uuid);
        parcel->readUint32(&login_type);
        parcel->readByteVector(&login_data);
        TeeServiceGpOperationIn::readFromParcel(parcel, &operation);
        return ::android::OK;
    }
};

}

}

}

#endif // __TEE_SERVICE_GP_OPEN_SESSION_IN_H__
