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

#ifndef __TEE_FWK_MC_GET_SESSION_ERROR_CODE_OUT_H__
#define __TEE_FWK_MC_GET_SESSION_ERROR_CODE_OUT_H__

#include <binder/Parcel.h>

namespace vendor {

namespace trustonic {

namespace teeservice {

struct TeeServiceMcGetSessionErrorCodeOut : public ::android::Parcelable {
    uint32_t mc_result = 15;

    int32_t last_err = 0;

    virtual ::android::status_t writeToParcel(::android::Parcel* parcel) const {
        parcel->writeUint32(mc_result);
        parcel->writeInt32(last_err);
        return ::android::OK;
    }

    virtual ::android::status_t readFromParcel(const ::android::Parcel* parcel) {
        parcel->readUint32(&mc_result);
        parcel->readInt32(&last_err);
        return ::android::OK;
    }

    void callback(uint32_t mc_result_in, int last_err_in) {
        mc_result = mc_result_in;
        last_err = last_err_in;
    }
};

}

}

}

#endif // __TEE_FWK_MC_GET_SESSION_ERROR_CODE_OUT_H__
