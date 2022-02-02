/*
 * Copyright Samsung Electronics Co.,LTD.
 * Copyright (C) 2012 The Android Open Source Project
 *
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

 /* #define LOG_NDEBUG 0 */
#define LOG_TAG "ExynosJpegEncoderForCameraGed"

#include "ExynosCameraConfig.h"
#include "ExynosJpegEncoderForCamera.h"

int iJpegSize;

int ExynosJpegEncoderForCamera::encode(int *size, exif_attribute_t *exifInfo, char** pcJpegBuffer, debug_attribute_t *debugInfo)
{
    int ret = ERROR_NONE;
    unsigned char *exifOut = NULL;
    char *debugOut = NULL;
    struct timeval start, end;

    if (m_flagCreate == false) {
        ALOGE("ERR(%s[%d]:not yet created. so, fail", __FUNCTION__, __LINE__);
        return ERROR_NOT_YET_CREATED;
    }

    gettimeofday(&start, NULL);
#if 0
    ret = m_jpegMain->encode();
    if (ret) {
        ALOGE("ERR(%s[%d]:encode() fail", __FUNCTION__, __LINE__);
        return ret;
    }

    int iJpegSize = m_jpegMain->getJpegSize();

    if (iJpegSize<=0) {
        ALOGE("ERR(%s): output_size is too small(%d)!!", __FUNCTION__, iJpegSize);
        return ERROR_OUT_BUFFER_SIZE_TOO_SMALL;
    }
#endif

    ALOGD("DEBUG(%s[%d]):JPEG main encoder start", __FUNCTION__, __LINE__);
    m_jpegMainEncodeThread->run(PRIORITY_DEFAULT);

    if (!(m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_USER_PTR
        || m_jpegMain->checkInBufType() & JPEG_BUF_TYPE_DMA_BUF)) {
        ALOGE("ERR(%s): invalid buffer type (%d) fail", __FUNCTION__, m_jpegMain->checkInBufType());
        return ERROR_BUFFR_IS_NULL;
    }

    if (*pcJpegBuffer == NULL) {
        ALOGE("ERR(%s):pcJpegBuffer is null!!", __FUNCTION__);
        return ERROR_OUT_BUFFER_CREATE_FAIL;
    }

    if (exifInfo != NULL) {
        unsigned int thumbLen = 0;
        unsigned int exifLen = 0;
        unsigned int bufSize = 0;

        if (exifInfo->enableThumb) {
            ALOGD("DEBUG(%s[%d]):JPEG thumbnail encoder enabled", __FUNCTION__, __LINE__);
            if (encodeThumbnail(&thumbLen)) {
                ALOGE("ERR(%s):encodeThumbnail() fail", __FUNCTION__);
                bufSize = EXIF_FILE_SIZE;
                exifInfo->enableThumb = false;
            } else {
                if (thumbLen > EXIF_LIMIT_SIZE) {
                    ALOGE("ERR(%s):thumbLen(%d) is too bigger than EXIF_LIMIT_SIZE(%d)",
                            __FUNCTION__, thumbLen, EXIF_LIMIT_SIZE);

                    bufSize = EXIF_FILE_SIZE;
                    exifInfo->enableThumb = false;
                } else {
                    ALOGD("DEBUG(%s[%d]):JPEG thumbnail encoder done", __FUNCTION__, __LINE__);
                    bufSize = EXIF_FILE_SIZE + thumbLen;
                }
            }
        } else {
            bufSize = EXIF_FILE_SIZE;
            exifInfo->enableThumb = false;
        }

        int totalDbgSize = 0;
        if (debugInfo != NULL && debugInfo->num_of_appmarker > 0) {
               for(int i = 0; i < debugInfo->num_of_appmarker; i++) {
                    bufSize += debugInfo->debugSize[debugInfo->idx[i][0]];
                    totalDbgSize += debugInfo->debugSize[debugInfo->idx[i][0]];
                    bufSize += 4;
               }
        }

        exifOut = new unsigned char[bufSize];
        if (exifOut == NULL) {
            ALOGE("ERR(%s):Failed to allocate for exifOut", __FUNCTION__);
            delete[] exifOut;
            return ERROR_EXIFOUT_ALLOC_FAIL;
        }
        memset(exifOut, 0, bufSize);

        if (makeExif (exifOut, exifInfo, &exifLen)) {
            ALOGE("ERR(%s):Failed to make EXIF", __FUNCTION__);
            delete[] exifOut;
            return ERROR_MAKE_EXIF_FAIL;
        }

        /* Exif info size is overflow */
        if (exifLen - thumbLen > EXIF_INFO_LIMIT_SIZE) {
        //    ALOGE("ERR(%s):Exif info size(%d) is bigger than EXIF_INFO_LIMIT_SIZE(%d)",
        //            __FUNCTION__, exifLen - thumbLen, EXIF_INFO_LIMIT_SIZE);
        }

        ALOGD("DEBUG(%s[%d]):wait JPEG main encoder", __FUNCTION__, __LINE__);
        m_jpegMainEncodeThread->join();

        iJpegSize = m_jpegMain->getJpegSize();

        if (iJpegSize<=0) {
            ALOGE("ERR(%s):output_size is too small(%d)!!", __FUNCTION__, iJpegSize);
            delete[] exifOut;
            return ERROR_OUT_BUFFER_SIZE_TOO_SMALL;
        }

        ALOGD("DEBUG(%s[%d]):JPEG main encoder done, size(%d)", __FUNCTION__, __LINE__, iJpegSize);

        if (exifLen <= EXIF_LIMIT_SIZE || (debugInfo != NULL && debugInfo->num_of_appmarker
            && exifLen <= EXIF_LIMIT_SIZE + totalDbgSize)) {
            memmove(*pcJpegBuffer + exifLen + 2, *pcJpegBuffer + 2, iJpegSize - 2);

            if (exifLen <= bufSize) {
                memcpy(*pcJpegBuffer + 2, exifOut, exifLen);
                iJpegSize += exifLen;
            } else {
                ALOGE("ERR(%s):exifLen(%d) is too bigger than EXIF_FILE_SIZE(%d)",
                      __FUNCTION__, exifLen, EXIF_FILE_SIZE);
            }
        } else {
            ALOGE("ERR(%s):exifLen(%d) is too bingger than EXIF_LIMIT_SIZE(%d)",
	          __FUNCTION__, exifLen, EXIF_LIMIT_SIZE);
        }

        delete[] exifOut;
    }

    *size = iJpegSize;

    gettimeofday(&end, NULL);
    ALOGI("INFO(%s[%d]: Total JPEG Encoding Time \n", __FUNCTION__, __LINE__);

    return ERROR_NONE;
}
