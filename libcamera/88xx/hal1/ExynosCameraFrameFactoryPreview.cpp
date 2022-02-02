/*
**
** Copyright 2013, Samsung Electronics Co. LTD
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/* #define LOG_NDEBUG 0 */
#define LOG_TAG "ExynosCameraFrameFactoryPreview"
#include <cutils/log.h>

#include "ExynosCameraFrameFactoryPreview.h"

namespace android {

ExynosCameraFrameFactoryPreview::~ExynosCameraFrameFactoryPreview()
{
    status_t ret = NO_ERROR;

    ret = destroy();
    if (ret != NO_ERROR)
        CLOGE("ERR(%s[%d]):destroy fail", __FUNCTION__, __LINE__);
}

status_t ExynosCameraFrameFactoryPreview::create(__unused bool active)
{
    Mutex::Autolock lock(ExynosCameraStreamMutex::getInstance()->getStreamMutex());

    CLOGI("INFO(%s[%d])", __FUNCTION__, __LINE__);

    status_t ret = NO_ERROR;

    m_setupConfig();
    m_constructMainPipes();

    /* FLITE pipe initialize */
    ret = m_pipes[PIPE_FLITE]->create(m_sensorIds[PIPE_FLITE]);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):FLITE create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
            m_pipes[PIPE_FLITE]->getPipeName(), PIPE_FLITE);

    /* 3AA pipe initialize */
    ret = m_pipes[PIPE_3AA]->create();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):3AA create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
            m_pipes[PIPE_3AA]->getPipeName(), PIPE_3AA);

    /* ISP pipe initialize */
    ret = m_pipes[PIPE_ISP]->create();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):ISP create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
            m_pipes[PIPE_ISP]->getPipeName(), PIPE_ISP);

    /* TPU pipe initialize */
    ret = m_pipes[PIPE_TPU]->create();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):TPU create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
            m_pipes[PIPE_TPU]->getPipeName(), PIPE_TPU);

    /* MCSC pipe initialize */
    ret = m_pipes[PIPE_MCSC]->create();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):MCSC create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
            m_pipes[PIPE_MCSC]->getPipeName(), PIPE_MCSC);

    /* GSC_PREVIEW pipe initialize */
    ret = m_pipes[PIPE_GSC]->create();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):GSC create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
            m_pipes[PIPE_GSC]->getPipeName(), PIPE_GSC);

#ifdef BOARD_CAMERA_USES_DUAL_CAMERA
    /* FUSION pipe initialize */
    ret = m_pipes[INDEX(PIPE_FUSION)]->create();
    if (ret < 0) {
        CLOGE("ERR(%s[%d]):FUSION create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
        m_pipes[PIPE_FUSION]->getPipeName(), PIPE_FUSION);
#endif

    ret = m_pipes[PIPE_GSC_VIDEO]->create();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):PIPE_GSC_VIDEO create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
            m_pipes[PIPE_GSC_VIDEO]->getPipeName(), PIPE_GSC_VIDEO);

    if (m_supportReprocessing == false) {
        /* GSC_PICTURE pipe initialize */
        ret = m_pipes[PIPE_GSC_PICTURE]->create();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):GSC_PICTURE create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
        CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
                m_pipes[PIPE_GSC_PICTURE]->getPipeName(), PIPE_GSC_PICTURE);

        /* JPEG pipe initialize */
        ret = m_pipes[PIPE_JPEG]->create();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):JPEG create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
        CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
                m_pipes[PIPE_JPEG]->getPipeName(), PIPE_JPEG);
    }

#if 0
    /* ISP Suspend/Resume
       For improving the camera entrance time,
       The TWZ environment information must be delivered to F/W
    */
    bool useFWSuspendResume = true;

    if (m_parameters->getDualMode() == false
    ) {
        ret = m_pipes[PIPE_3AA]->setControl(V4L2_CID_IS_CAMERA_TYPE, (int)useFWSuspendResume);
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):PIPE_%d V4L2_CID_IS_CAMERA_TYPE fail, ret(%d)", __FUNCTION__, __LINE__, PIPE_3AA, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }
#endif

    /* EOS */
    ret = m_pipes[PIPE_3AA]->setControl(V4L2_CID_IS_END_OF_STREAM, 1);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):PIPE_%d V4L2_CID_IS_END_OF_STREAM fail, ret(%d)", __FUNCTION__, __LINE__, PIPE_3AA, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    m_setCreate(true);

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::precreate(void)
{
    CLOGI("INFO(%s[%d])", __FUNCTION__, __LINE__);

    status_t ret = NO_ERROR;

    m_setupConfig();
    m_constructMainPipes();

    /* flite pipe initialize */
    ret = m_pipes[PIPE_FLITE]->create(m_sensorIds[PIPE_FLITE]);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):FLITE create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
            m_pipes[PIPE_FLITE]->getPipeName(), PIPE_FLITE);

    /* 3AA pipe initialize */
    ret = m_pipes[PIPE_3AA]->precreate();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):3AA create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) precreated", __FUNCTION__, __LINE__,
            m_pipes[PIPE_3AA]->getPipeName(), PIPE_3AA);

    /* ISP pipe initialize */
    ret = m_pipes[PIPE_ISP]->precreate();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):ISP create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) precreated", __FUNCTION__, __LINE__,
            m_pipes[PIPE_ISP]->getPipeName(), PIPE_ISP);

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::postcreate(void)
{
    CLOGI("INFO(%s[%d])", __FUNCTION__, __LINE__);
    status_t ret = NO_ERROR;

    /* 3AA_ISP pipe initialize */
    ret = m_pipes[PIPE_3AA]->postcreate();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):3AA_ISP postcreate fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) postcreated", __FUNCTION__, __LINE__,
            m_pipes[PIPE_3AA]->getPipeName(), PIPE_3AA);

    /* ISP pipe initialize */
    ret = m_pipes[PIPE_ISP]->postcreate();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):ISP create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) postcreated", __FUNCTION__, __LINE__,
            m_pipes[PIPE_ISP]->getPipeName(), PIPE_ISP);

    /* TPU pipe initialize */
    ret = m_pipes[PIPE_TPU]->create();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):TPU create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
            m_pipes[PIPE_TPU]->getPipeName(), PIPE_TPU);

    /* MCSC pipe initialize */
    ret = m_pipes[PIPE_MCSC]->create();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):MCSC create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
            m_pipes[PIPE_MCSC]->getPipeName(), PIPE_MCSC);

    /* GSC_PREVIEW pipe initialize */
    ret = m_pipes[PIPE_GSC]->create();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):GSC create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
            m_pipes[PIPE_GSC]->getPipeName(), PIPE_GSC);

#ifdef BOARD_CAMERA_USES_DUAL_CAMERA
    /* FUSION pipe initialize */
    ret = m_pipes[INDEX(PIPE_FUSION)]->create();
    if (ret < 0) {
        CLOGE("ERR(%s[%d]):FUSION create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
        m_pipes[PIPE_FUSION]->getPipeName(), PIPE_FUSION);
#endif

    ret = m_pipes[PIPE_GSC_VIDEO]->create();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):PIPE_GSC_VIDEO create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }
    CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
            m_pipes[PIPE_GSC_VIDEO]->getPipeName(), PIPE_GSC_VIDEO);

    if (m_supportReprocessing == false) {
        /* GSC_PICTURE pipe initialize */
        ret = m_pipes[PIPE_GSC_PICTURE]->create();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):GSC_PICTURE create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
        CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
                m_pipes[PIPE_GSC_PICTURE]->getPipeName(), PIPE_GSC_PICTURE);

        /* JPEG pipe initialize */
        ret = m_pipes[PIPE_JPEG]->create();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):JPEG create fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
        CLOGD("DEBUG(%s[%d]):%s(%d) created", __FUNCTION__, __LINE__,
                m_pipes[PIPE_JPEG]->getPipeName(), PIPE_JPEG);
    }

    /* EOS */
    ret = m_pipes[PIPE_3AA]->setControl(V4L2_CID_IS_END_OF_STREAM, 1);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):PIPE_%d V4L2_CID_IS_END_OF_STREAM fail, ret(%d)", __FUNCTION__, __LINE__, PIPE_3AA, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    m_setCreate(true);

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::fastenAeStable(int32_t numFrames, ExynosCameraBuffer *buffers)
{
    CLOGI("INFO(%s[%d]): Start", __FUNCTION__, __LINE__);

    CLOGI("INFO(%s[%d]):Done", __FUNCTION__, __LINE__);

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::initPipes(void)
{
    CLOGI("INFO(%s[%d])", __FUNCTION__, __LINE__);

    status_t ret = NO_ERROR;

    int hwSensorW = 0, hwSensorH = 0;
    uint32_t minFrameRate = 0, maxFrameRate = 0, sensorFrameRate = 0;

    m_parameters->getHwSensorSize(&hwSensorW, &hwSensorH);
    m_parameters->getPreviewFpsRange(&minFrameRate, &maxFrameRate);
    if (m_parameters->getScalableSensorMode() == true)
        sensorFrameRate = 24;
    else
        sensorFrameRate = maxFrameRate;

    ret = m_initFlitePipe(hwSensorW, hwSensorH, sensorFrameRate);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):m_initFlitePipe() fail", __FUNCTION__, __LINE__);
        return ret;
    }

    /* setDeviceInfo does changing path */
    ret = m_setupConfig();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):m_setupConfig(%d) fail", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = m_initPipes();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):m_initPipes() fail", __FUNCTION__, __LINE__);
        return ret;
    }

    m_frameCount = 0;

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::mapBuffers(void)
{
    status_t ret = NO_ERROR;

    ret = m_pipes[PIPE_3AA]->setMapBuffer();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):3AA mapBuffer fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    if (m_flag3aaIspOTF == false) {
        ret = m_pipes[PIPE_ISP]->setMapBuffer();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):ISP mapBuffer fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    if (m_parameters->getTpuEnabledMode() == true && m_flagIspTpuOTF == false) {
        ret = m_pipes[PIPE_TPU]->setMapBuffer();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):TPU mapBuffer fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    if (m_flagIspMcscOTF == false && m_flagTpuMcscOTF == false) {
        ret = m_pipes[PIPE_MCSC]->setMapBuffer();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):MCSC mapBuffer fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    CLOGI("INFO(%s[%d]):Map buffer Success!", __FUNCTION__, __LINE__);

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::preparePipes(void)
{
    status_t ret = NO_ERROR;

    /* NOTE: Prepare for 3AA is moved after ISP stream on */

    if (m_requestFLITE) {
        ret = m_pipes[PIPE_FLITE]->prepare();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):FLITE prepare fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::startPipes(void)
{
    status_t ret = NO_ERROR;

#ifdef BOARD_CAMERA_USES_DUAL_CAMERA
    if (m_parameters->isFusionEnabled() == true) {
        ret = m_pipes[INDEX(PIPE_FUSION)]->start();
        if (ret < 0) {
            CLOGE("ERR(%s[%d]):PIPE_FUSION start fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }
#endif

    if (m_flagIspMcscOTF == false && m_flagTpuMcscOTF == false) {
        ret = m_pipes[PIPE_MCSC]->start();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):MCSC start fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    if (m_parameters->getTpuEnabledMode() == true && m_flagIspTpuOTF == false) {
        ret = m_pipes[PIPE_TPU]->start();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):TPU start fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    if (m_flag3aaIspOTF == false) {
        ret = m_pipes[PIPE_ISP]->start();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):ISP start fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    ret = m_pipes[PIPE_3AA]->start();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):3AA start fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    ret = m_pipes[PIPE_FLITE]->start();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):FLITE start fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    if (m_flagFlite3aaOTF == true) {
        /* Here is doing 3AA prepare(qbuf) */
        ret = m_pipes[PIPE_3AA]->prepare();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):3AA prepare fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    ret = m_pipes[PIPE_FLITE]->sensorStream(true);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):FLITE sensorStream on fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    CLOGI("INFO(%s[%d]):Starting Success!", __FUNCTION__, __LINE__);

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::stopPipes(void)
{
    status_t ret = NO_ERROR;

    if (m_pipes[PIPE_3AA]->isThreadRunning() == true) {
        ret = m_pipes[PIPE_3AA]->stopThread();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):3AA stopThread fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    if (m_pipes[PIPE_ISP]->isThreadRunning() == true) {
        ret = m_pipes[PIPE_ISP]->stopThread();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):ISP stopThread fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    if (m_pipes[PIPE_TPU]->isThreadRunning() == true) {
        ret = m_pipes[PIPE_TPU]->stopThread();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):TPU stopThread fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    if (m_pipes[PIPE_MCSC]->isThreadRunning() == true) {
        ret = m_pipes[PIPE_MCSC]->stopThread();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):MCSC stopThread fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    if (m_requestFLITE) {
        ret = m_pipes[PIPE_FLITE]->stopThread();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):FLITE stopThread fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    if (m_pipes[PIPE_GSC]->isThreadRunning() == true) {
        ret = m_pipes[PIPE_GSC]->stopThread();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):PIPE_GSC stopThread fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

#ifdef BOARD_CAMERA_USES_DUAL_CAMERA
    if (m_pipes[INDEX(PIPE_FUSION)] != NULL &&
        m_pipes[INDEX(PIPE_FUSION)]->isThreadRunning() == true) {
        ret = stopThread(INDEX(PIPE_FUSION));
        if (ret < 0) {
            CLOGE("ERR(%s[%d]):PIPE_FUSION stopThread fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }
#endif

    ret = m_pipes[PIPE_FLITE]->sensorStream(false);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):FLITE sensorStream off fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    ret = m_pipes[PIPE_FLITE]->stop();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):FLITE stop fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    /* 3AA force done */
    ret = m_pipes[PIPE_3AA]->forceDone(V4L2_CID_IS_FORCE_DONE, 0x1000);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):PIPE_3AA force done fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        /* return INVALID_OPERATION; */
    }

    /* stream off for 3AA */
    ret = m_pipes[PIPE_3AA]->stop();
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):3AA stop fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    if (m_pipes[PIPE_ISP]->flagStart() == true) {
        /* ISP force done */
        ret = m_pipes[PIPE_ISP]->forceDone(V4L2_CID_IS_FORCE_DONE, 0x1000);
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):PIPE_ISP force done fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            /* return INVALID_OPERATION; */
        }

        /* stream off for ISP */
        ret = m_pipes[PIPE_ISP]->stop();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):ISP stop fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    if (m_pipes[PIPE_TPU]->flagStart() == true) {
        /* TPU force done */
        ret = m_pipes[PIPE_TPU]->forceDone(V4L2_CID_IS_FORCE_DONE, 0x1000);
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):PIPE_TPU force done fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            /* return INVALID_OPERATION; */
        }

        ret = m_pipes[PIPE_TPU]->stop();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):TPU stop fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    if (m_pipes[PIPE_MCSC]->flagStart() == true) {
        /* MCSC force done */
        ret = m_pipes[PIPE_MCSC]->forceDone(V4L2_CID_IS_FORCE_DONE, 0x1000);
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):PIPE_MCSC force done fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            /* return INVALID_OPERATION; */
        }

        ret = m_pipes[PIPE_MCSC]->stop();
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):MCSC stop fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }

    ret = stopThreadAndWait(PIPE_GSC);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):PIPE_GSC stopThreadAndWait fail, ret(%d)", __FUNCTION__, __LINE__, ret);
    }

#ifdef BOARD_CAMERA_USES_DUAL_CAMERA
    if (m_pipes[INDEX(PIPE_FUSION)] != NULL) {
        ret = m_pipes[INDEX(PIPE_FUSION)]->stop();
        if (ret < 0) {
            CLOGE("ERR(%s[%d]): m_pipes[INDEX(PIPE_FUSION)]->stop() fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }
    }
#endif

    CLOGI("INFO(%s[%d]):Stopping  Success!", __FUNCTION__, __LINE__);

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::startInitialThreads(void)
{
    status_t ret = NO_ERROR;

    CLOGI("INFO(%s[%d]):start pre-ordered initial pipe thread", __FUNCTION__, __LINE__);

    if (m_requestFLITE) {
        ret = startThread(PIPE_FLITE);
        if (ret != NO_ERROR)
            return ret;
    }

    ret = startThread(PIPE_3AA);
    if (ret != NO_ERROR)
        return ret;

    if (m_flag3aaIspOTF == false) {
        ret = startThread(PIPE_ISP);
        if (ret != NO_ERROR)
            return ret;
    }

    if (m_parameters->getTpuEnabledMode() == true && m_flagIspTpuOTF == false) {
        ret = startThread(PIPE_TPU);
        if (ret != NO_ERROR)
            return ret;
    }

    if (m_flagIspMcscOTF == false && m_flagTpuMcscOTF == false) {
        ret = startThread(PIPE_MCSC);
        if (ret != NO_ERROR)
            return ret;
    }

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::setStopFlag(void)
{
    CLOGI("INFO(%s[%d]):", __FUNCTION__, __LINE__);

    status_t ret = NO_ERROR;

    ret = m_pipes[PIPE_FLITE]->setStopFlag();

    if (m_pipes[PIPE_3AA]->flagStart() == true)
        ret = m_pipes[PIPE_3AA]->setStopFlag();

    if (m_pipes[PIPE_ISP]->flagStart() == true)
        ret = m_pipes[PIPE_ISP]->setStopFlag();

    if (m_pipes[PIPE_TPU]->flagStart() == true)
        ret = m_pipes[PIPE_TPU]->setStopFlag();

    if (m_pipes[PIPE_MCSC]->flagStart() == true)
        ret = m_pipes[PIPE_MCSC]->setStopFlag();

#ifdef BOARD_CAMERA_USES_DUAL_CAMERA
    if (m_pipes[INDEX(PIPE_FUSION)] != NULL &&
        m_pipes[INDEX(PIPE_FUSION)]->flagStart() == true)
        ret = m_pipes[INDEX(PIPE_FUSION)]->setStopFlag();
#endif

    return NO_ERROR;
}

ExynosCameraFrame *ExynosCameraFrameFactoryPreview::createNewFrame(void)
{
    status_t ret = NO_ERROR;
    ExynosCameraFrameEntity *newEntity[MAX_NUM_PIPES] = {0};
    ExynosCameraFrame *frame = m_frameMgr->createFrame(m_parameters, m_frameCount, FRAME_TYPE_PREVIEW);

    int requestEntityCount = 0;
    int pipeId = -1;
    int parentPipeId = -1;

    ret = m_initFrameMetadata(frame);
    if (ret != NO_ERROR)
        CLOGE("(%s[%d]):frame(%d) metadata initialize fail", __FUNCTION__, __LINE__, m_frameCount);

    if (m_requestFLITE == true) {
        /* set flite pipe to linkageList */
        pipeId = PIPE_FLITE;
        newEntity[pipeId] = new ExynosCameraFrameEntity(pipeId, ENTITY_TYPE_OUTPUT_ONLY, ENTITY_BUFFER_FIXED);
        frame->addSiblingEntity(NULL, newEntity[pipeId]);
        requestEntityCount++;
    }

    /* set 3AA pipe to linkageList */
    pipeId = PIPE_3AA;
    newEntity[pipeId] = new ExynosCameraFrameEntity(pipeId, ENTITY_TYPE_INPUT_ONLY, ENTITY_BUFFER_FIXED);
    frame->addSiblingEntity(NULL, newEntity[pipeId]);
    requestEntityCount++;
    parentPipeId = pipeId;

    /* set ISP pipe to linkageList */
    if (m_flag3aaIspOTF == false) {
        pipeId = PIPE_ISP;
        newEntity[pipeId] = new ExynosCameraFrameEntity(pipeId, ENTITY_TYPE_INPUT_ONLY, ENTITY_BUFFER_FIXED);
        frame->addChildEntity(newEntity[parentPipeId], newEntity[pipeId], PIPE_3AP);
        requestEntityCount++;
        parentPipeId = pipeId;
    }

    /* set TPU pipe to linkageList */
    if (m_parameters->getTpuEnabledMode() == true && m_flagIspTpuOTF == false) {
        pipeId = PIPE_TPU;
        newEntity[pipeId] = new ExynosCameraFrameEntity(pipeId, ENTITY_TYPE_INPUT_ONLY, ENTITY_BUFFER_FIXED);
        frame->addChildEntity(newEntity[parentPipeId], newEntity[pipeId], PIPE_ISPP);
        requestEntityCount++;
        parentPipeId = pipeId;
    }

    /* set MCSC pipe to linkageList */
    if (m_flagIspMcscOTF == false && m_flagTpuMcscOTF == false) {
        pipeId = PIPE_MCSC;
        newEntity[pipeId] = new ExynosCameraFrameEntity(pipeId, ENTITY_TYPE_INPUT_ONLY, ENTITY_BUFFER_FIXED);
        if (m_parameters->getTpuEnabledMode() == true)
            frame->addChildEntity(newEntity[parentPipeId], newEntity[pipeId], PIPE_TPUP);
        else
            frame->addChildEntity(newEntity[parentPipeId], newEntity[pipeId], PIPE_ISPC);
        requestEntityCount++;
    }

#ifdef CAMERA_VENDOR_TURNKEY_FEATURE
    if (m_parameters->getVendorMode(SFLIBRARY_MGR::FLAWLESS)) {
        pipeId = PIPE_VENDOR_3PARTY_LIB;
        newEntity[pipeId] = new ExynosCameraFrameEntity(pipeId, ENTITY_TYPE_INPUT_OUTPUT, ENTITY_BUFFER_FIXED);
        frame->addSiblingEntity(NULL, newEntity[pipeId]);
        requestEntityCount++;
    }
#endif

    /* set GSC pipe to linkageList */
    pipeId = PIPE_GSC;
    newEntity[pipeId] = new ExynosCameraFrameEntity(pipeId, ENTITY_TYPE_INPUT_OUTPUT, ENTITY_BUFFER_FIXED);
    frame->addSiblingEntity(NULL, newEntity[pipeId]);

#ifdef BOARD_CAMERA_USES_DUAL_CAMERA
    if (m_parameters->isFusionEnabled() == true) {
        newEntity[INDEX(PIPE_FUSION)] = new ExynosCameraFrameEntity(PIPE_FUSION, ENTITY_TYPE_INPUT_OUTPUT, ENTITY_BUFFER_FIXED);
        frame->addSiblingEntity(NULL, newEntity[INDEX(PIPE_FUSION)]);
        requestEntityCount++;
    }
#endif

    /* set GSC for Video pipe to linkageList */
    pipeId = PIPE_GSC_VIDEO;
    newEntity[pipeId] = new ExynosCameraFrameEntity(pipeId, ENTITY_TYPE_INPUT_OUTPUT, ENTITY_BUFFER_FIXED);
    frame->addSiblingEntity(NULL, newEntity[pipeId]);

    if (m_supportReprocessing == false) {
        /* set GSC for Capture pipe to linkageList */
        pipeId = PIPE_GSC_PICTURE;
        newEntity[pipeId] = new ExynosCameraFrameEntity(pipeId, ENTITY_TYPE_INPUT_OUTPUT, ENTITY_BUFFER_FIXED);
        frame->addSiblingEntity(NULL, newEntity[pipeId]);

        /* set JPEG pipe to linkageList */
        pipeId = PIPE_JPEG;
        newEntity[pipeId] = new ExynosCameraFrameEntity(pipeId, ENTITY_TYPE_INPUT_OUTPUT, ENTITY_BUFFER_FIXED);
        frame->addSiblingEntity(NULL, newEntity[pipeId]);
    }

    ret = m_initPipelines(frame);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):m_initPipelines fail, ret(%d)", __FUNCTION__, __LINE__, ret);
    }

    /* TODO: make it dynamic */
    frame->setNumRequestPipe(requestEntityCount);

    m_fillNodeGroupInfo(frame);

    m_frameCount++;

    return frame;
}

status_t ExynosCameraFrameFactoryPreview::m_setupConfig(bool isFastAE)
{
    CLOGI("INFO(%s[%d])", __FUNCTION__, __LINE__);

    status_t ret = NO_ERROR;

    int pipeId = -1;

    m_flagFlite3aaOTF = m_parameters->isFlite3aaOtf();
    m_flag3aaIspOTF = m_parameters->is3aaIspOtf();
    m_flagIspTpuOTF = m_parameters->isIspTpuOtf();
    m_flagIspMcscOTF = m_parameters->isIspMcscOtf();
    m_flagTpuMcscOTF = m_parameters->isTpuMcscOtf();

    m_supportReprocessing = m_parameters->isReprocessing();
    m_supportPureBayerReprocessing = m_parameters->getUsePureBayerReprocessing();

    m_request3AP = !(m_flag3aaIspOTF);
    if (m_flagIspTpuOTF == false && m_flagIspMcscOTF == false) {
        if (m_parameters->getTpuEnabledMode() == true)
            m_requestISPP = 1;
        else
            m_requestISPC = 1;
    }
    m_requestMCSC0 = 1;

    /* FLITE */
    pipeId = PIPE_FLITE;
    m_nodeNums[pipeId][CAPTURE_NODE_1] = m_getFliteNodenum();
    m_sensorIds[pipeId][CAPTURE_NODE_1] = m_getSensorId(m_nodeNums[pipeId][CAPTURE_NODE_1], m_flagReprocessing);

    /* 3AA ~ MCSC */
    ret = m_setDeviceInfo(isFastAE);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):m_setDeviceInfo() fail, ret(%d)",
                __FUNCTION__, __LINE__, ret);
        return ret;
    }

    /* GSC */
    m_nodeNums[PIPE_GSC][OUTPUT_NODE] = PREVIEW_GSC_NODE_NUM;

    /* GSC for Recording */
    m_nodeNums[PIPE_GSC_VIDEO][OUTPUT_NODE] = VIDEO_GSC_NODE_NUM;

    /* GSC for Capture */
    m_nodeNums[PIPE_GSC_PICTURE][OUTPUT_NODE] = PICTURE_GSC_NODE_NUM;

    /* JPEG */
    m_nodeNums[PIPE_JPEG][OUTPUT_NODE] = -1;

#ifdef BOARD_CAMERA_USES_DUAL_CAMERA
    m_nodeNums[INDEX(PIPE_FUSION)][OUTPUT_NODE] = 0;
    m_nodeNums[INDEX(PIPE_FUSION)][CAPTURE_NODE_1] = -1;
    m_nodeNums[INDEX(PIPE_FUSION)][CAPTURE_NODE_2] = -1;
#endif

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::m_constructMainPipes()
{
    CLOGI("INFO(%s[%d])", __FUNCTION__, __LINE__);

    int pipeId = -1;

    /* FLITE */
    pipeId = PIPE_FLITE;
    m_pipes[pipeId] = (ExynosCameraPipe*)new ExynosCameraPipeFlite(m_cameraId, m_parameters, m_flagReprocessing, m_nodeNums[pipeId]);
    m_pipes[pipeId]->setPipeId(pipeId);
    m_pipes[pipeId]->setPipeName("PIPE_FLITE");

    /* 3AA */
    pipeId = PIPE_3AA;
    m_pipes[pipeId] = (ExynosCameraPipe*)new ExynosCameraMCPipe(m_cameraId, m_parameters, m_flagReprocessing, &m_deviceInfo[pipeId]);
    m_pipes[pipeId]->setPipeId(pipeId);
    m_pipes[pipeId]->setPipeName("PIPE_3AA");

    /* ISP */
    pipeId = PIPE_ISP;
    m_pipes[pipeId] = (ExynosCameraPipe*)new ExynosCameraMCPipe(m_cameraId, m_parameters, m_flagReprocessing, &m_deviceInfo[pipeId]);
    m_pipes[pipeId]->setPipeId(pipeId);
    m_pipes[pipeId]->setPipeName("PIPE_ISP");

    /* TPU */
    pipeId = PIPE_TPU;
    m_pipes[pipeId] = (ExynosCameraPipe*)new ExynosCameraMCPipe(m_cameraId, m_parameters, m_flagReprocessing, &m_deviceInfo[pipeId]);
    m_pipes[pipeId]->setPipeId(pipeId);
    m_pipes[pipeId]->setPipeName("PIPE_TPU");

    /* MCSC */
    pipeId = PIPE_MCSC;
    m_pipes[pipeId] = (ExynosCameraPipe*)new ExynosCameraMCPipe(m_cameraId, m_parameters, m_flagReprocessing, &m_deviceInfo[pipeId]);
    m_pipes[pipeId]->setPipeId(pipeId);
    m_pipes[pipeId]->setPipeName("PIPE_MCSC");

    /* GSC */
    pipeId = PIPE_GSC;
    m_pipes[pipeId] = (ExynosCameraPipe*)new ExynosCameraPipeGSC(m_cameraId, m_parameters, m_flagReprocessing, m_nodeNums[pipeId]);
    m_pipes[PIPE_GSC]->setPipeId(PIPE_GSC);
    m_pipes[PIPE_GSC]->setPipeName("PIPE_GSC");

#ifdef BOARD_CAMERA_USES_DUAL_CAMERA
    m_pipes[INDEX(PIPE_FUSION)] = (ExynosCameraPipe*)new ExynosCameraPipeFusion(m_cameraId, m_parameters, false, m_nodeNums[INDEX(PIPE_FUSION)]);
    m_pipes[INDEX(PIPE_FUSION)]->setPipeId(PIPE_FUSION);
    m_pipes[INDEX(PIPE_FUSION)]->setPipeName("PIPE_FUSION");
#endif

    /* GSC for Recording */
    pipeId = PIPE_GSC_VIDEO;
    m_pipes[pipeId] = (ExynosCameraPipe*)new ExynosCameraPipeGSC(m_cameraId, m_parameters, m_flagReprocessing, m_nodeNums[pipeId]);
    m_pipes[pipeId]->setPipeId(pipeId);
    m_pipes[pipeId]->setPipeName("PIPE_GSC_VIDEO");

    /* GSC for Capture */
    pipeId = PIPE_GSC_PICTURE;
    m_pipes[pipeId] = (ExynosCameraPipe*)new ExynosCameraPipeGSC(m_cameraId, m_parameters, m_flagReprocessing, m_nodeNums[pipeId]);
    m_pipes[pipeId]->setPipeId(pipeId);
    m_pipes[pipeId]->setPipeName("PIPE_GSC_PICTURE");

    /* JPEG */
    pipeId = PIPE_JPEG;
    m_pipes[pipeId] = (ExynosCameraPipe*)new ExynosCameraPipeJpeg(m_cameraId, m_parameters, m_flagReprocessing, m_nodeNums[pipeId]);
    m_pipes[pipeId]->setPipeId(pipeId);
    m_pipes[pipeId]->setPipeName("PIPE_JPEG");

    CLOGI("INFO(%s[%d]):pipe ids for reprocessing", __FUNCTION__, __LINE__);
    for (int i = 0; i < MAX_NUM_PIPES; i++) {
        if (m_pipes[i] != NULL) {
            CLOGI("INFO(%s[%d]):-> m_pipes[%d] : PipeId(%d)", __FUNCTION__, __LINE__ , i, m_pipes[i]->getPipeId());
        }
    }

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::m_setDeviceInfo(bool isFastAE)
{
    CLOGI("INFO(%s[%d])", __FUNCTION__, __LINE__);


    int pipeId = -1;
    int node3aa = -1, node3ac = -1, node3ap = -1;
    int nodeIsp = -1, nodeIspc = -1, nodeIspp = -1;
    int nodeTpu = -1;
    int nodeMcsc = -1, nodeMcscp0 = -1, nodeMcscp1 = -1, nodeMcscp2 = -1;
    int previousPipeId = -1;
    int mcscSrcPipeId = -1;
    unsigned int flite3aaConnectionMode = (unsigned int)m_flagFlite3aaOTF;
    unsigned int taaIspConnectionMode = (unsigned int)m_flag3aaIspOTF;
    unsigned int ispMcscConnectionMode = (unsigned int)m_flagIspMcscOTF;
    unsigned int ispTpuConnectionMode = (unsigned int)m_flagIspTpuOTF;
    unsigned int tpuMcscConnectionMode = (unsigned int)m_flagTpuMcscOTF;
    unsigned int mcscConnectionMode = 0;
    enum NODE_TYPE nodeType = INVALID_NODE;
    bool flagStreamLeader = false;

    m_initDeviceInfo(PIPE_3AA);
    m_initDeviceInfo(PIPE_ISP);
    m_initDeviceInfo(PIPE_TPU);
    m_initDeviceInfo(PIPE_MCSC);

    if ((m_cameraId == CAMERA_ID_FRONT || m_cameraId == CAMERA_ID_BACK_1) &&
        (m_parameters->getDualMode() == true)) {
        node3aa = FIMC_IS_VIDEO_31S_NUM;
        node3ac = FIMC_IS_VIDEO_31C_NUM;
        node3ap = FIMC_IS_VIDEO_31P_NUM;
        nodeIsp = FIMC_IS_VIDEO_I1S_NUM;
        nodeIspc = FIMC_IS_VIDEO_I1C_NUM;
        nodeIspp = FIMC_IS_VIDEO_I1P_NUM;
        nodeMcsc = FIMC_IS_VIDEO_M1S_NUM;
        nodeMcscp0 = FIMC_IS_VIDEO_M2P_NUM;
        nodeMcscp1 = FIMC_IS_VIDEO_M4P_NUM;
    } else {
        node3aa = FIMC_IS_VIDEO_30S_NUM;
        node3ac = FIMC_IS_VIDEO_30C_NUM;
        node3ap = FIMC_IS_VIDEO_30P_NUM;
        nodeIsp = FIMC_IS_VIDEO_I0S_NUM;
        nodeIspc = FIMC_IS_VIDEO_I0C_NUM;
        nodeIspp = FIMC_IS_VIDEO_I0P_NUM;
        nodeMcsc = FIMC_IS_VIDEO_M0S_NUM;
        nodeMcscp0 = FIMC_IS_VIDEO_M0P_NUM;
        nodeMcscp1 = FIMC_IS_VIDEO_M1P_NUM;
    }

    nodeTpu = FIMC_IS_VIDEO_TPU_NUM;
    nodeMcscp2 = FIMC_IS_VIDEO_M2P_NUM;

    if (m_cameraId == CAMERA_ID_BACK || m_parameters->getDualMode() == false) {
#ifdef USE_FLITE_3AA_BUFFER_HIDING_MODE
        if (USE_FLITE_3AA_BUFFER_HIDING_MODE == true && m_flagFlite3aaOTF == true)
            flite3aaConnectionMode = HW_CONNECTION_MODE_M2M_BUFFER_HIDING;
#endif
#ifdef USE_3AA_ISP_BUFFER_HIDING_MODE
        if (USE_3AA_ISP_BUFFER_HIDING_MODE == true && m_flag3aaIspOTF == true)
            taaIspConnectionMode = HW_CONNECTION_MODE_M2M_BUFFER_HIDING;
#endif
#ifdef USE_ISP_MCSC_BUFFER_HIDING_MODE
        if (USE_ISP_MCSC_BUFFER_HIDING_MODE == true && m_flagIspMcscOTF == true)
            ispMcscConnectionMode = HW_CONNECTION_MODE_M2M_BUFFER_HIDING;
#endif
#ifdef USE_ISP_TPU_BUFFER_HIDING_MODE
        if (USE_ISP_TPU_BUFFER_HIDING_MODE == true && m_flagIspTpuOTF == true)
            ispTpuConnectionMode = HW_CONNECTION_MODE_M2M_BUFFER_HIDING;
#endif
#ifdef USE_TPU_MCSC_BUFFER_HIDING_MODE
        if (USE_TPU_MCSC_BUFFER_HIDING_MODE == true && m_flagTpuMcscOTF == true)
            tpuMcscConnectionMode = HW_CONNECTION_MODE_M2M_BUFFER_HIDING;
#endif
    }

    /*
     * 3AA
     */
    pipeId = PIPE_3AA;
    previousPipeId = PIPE_FLITE;

    /* 3AS */
    flagStreamLeader = true;
    nodeType = getNodeType(PIPE_3AA);
    m_deviceInfo[pipeId].pipeId[nodeType]  = PIPE_3AA;
    m_deviceInfo[pipeId].nodeNum[nodeType] = node3aa;
    m_deviceInfo[pipeId].connectionMode[nodeType] = flite3aaConnectionMode;
    strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "3AA_OUTPUT", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
    if (isFastAE == true && flite3aaConnectionMode == HW_CONNECTION_MODE_M2M_BUFFER_HIDING)
        flite3aaConnectionMode = false;
    m_sensorIds[pipeId][nodeType] = m_getSensorId(m_nodeNums[previousPipeId][getNodeType(previousPipeId)], flite3aaConnectionMode, flagStreamLeader, m_flagReprocessing);

    /* Other nodes is not stream leader */
    flagStreamLeader = false;

    /* 3AC */
    nodeType = getNodeType(PIPE_3AC);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_3AC;
    m_deviceInfo[pipeId].nodeNum[nodeType] = node3ac;
    m_deviceInfo[pipeId].connectionMode[nodeType] = HW_CONNECTION_MODE_OTF;
    strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "3AA_CAPTURE", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
    m_sensorIds[pipeId][nodeType] = m_getSensorId(m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_3AA)], true, flagStreamLeader, m_flagReprocessing);

    /* 3AP */
    nodeType = getNodeType(PIPE_3AP);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_3AP;
    m_deviceInfo[pipeId].nodeNum[nodeType] = node3ap;
    m_deviceInfo[pipeId].connectionMode[nodeType] = taaIspConnectionMode;
    strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "3AA_PREVIEW", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
    m_sensorIds[pipeId][nodeType] = m_getSensorId(m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_3AA)], true, flagStreamLeader, m_flagReprocessing);


    /*
     * ISP
     */
    previousPipeId = pipeId;
    mcscSrcPipeId = PIPE_ISPC;
    mcscConnectionMode = ispMcscConnectionMode;
    pipeId = m_flag3aaIspOTF ? PIPE_3AA : PIPE_ISP;

    /* ISPS */
    nodeType = getNodeType(PIPE_ISP);
    m_deviceInfo[pipeId].pipeId[nodeType]  = PIPE_ISP;
    m_deviceInfo[pipeId].nodeNum[nodeType] = nodeIsp;
    m_deviceInfo[pipeId].connectionMode[nodeType] = taaIspConnectionMode;
    strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "ISP_OUTPUT", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
    if (isFastAE == true && taaIspConnectionMode == HW_CONNECTION_MODE_M2M_BUFFER_HIDING)
        taaIspConnectionMode = false;
    m_sensorIds[pipeId][nodeType] = m_getSensorId(m_deviceInfo[previousPipeId].nodeNum[getNodeType(PIPE_3AP)], taaIspConnectionMode, flagStreamLeader, m_flagReprocessing);

    /* ISPC */
    nodeType = getNodeType(PIPE_ISPC);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_ISPC;
    m_deviceInfo[pipeId].nodeNum[nodeType] = nodeIspc;
    m_deviceInfo[pipeId].connectionMode[nodeType] = HW_CONNECTION_MODE_OTF;
    strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "ISP_CAPTURE", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
    m_sensorIds[pipeId][nodeType] = m_getSensorId(m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_ISP)], true, flagStreamLeader, m_flagReprocessing);

    /* ISPP */
    nodeType = getNodeType(PIPE_ISPP);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_ISPP;
    m_deviceInfo[pipeId].nodeNum[nodeType] = nodeIspp;
    m_deviceInfo[pipeId].connectionMode[nodeType] = (ispTpuConnectionMode | ispMcscConnectionMode);
    strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "ISP_PREVIEW", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
    m_sensorIds[pipeId][nodeType] = m_getSensorId(m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_ISP)], true, flagStreamLeader, m_flagReprocessing);

    /*
     * TPU
     */
    if (m_parameters->getTpuEnabledMode() == true) {
        previousPipeId = pipeId;
        mcscSrcPipeId = PIPE_TPU;
        mcscConnectionMode = tpuMcscConnectionMode;

        if (m_flagIspTpuOTF == false)
            pipeId = PIPE_TPU;

        /* TPU */
        nodeType = getNodeType(PIPE_TPU);
        m_deviceInfo[pipeId].pipeId[nodeType]  = PIPE_TPU;
        m_deviceInfo[pipeId].nodeNum[nodeType] = nodeTpu;
        m_deviceInfo[pipeId].connectionMode[nodeType] = ispTpuConnectionMode;
        strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "TPU_OUTPUT", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
        if (isFastAE == true && ispTpuConnectionMode == HW_CONNECTION_MODE_M2M_BUFFER_HIDING)
            ispTpuConnectionMode = false;
        m_sensorIds[pipeId][nodeType] = m_getSensorId(m_deviceInfo[previousPipeId].nodeNum[getNodeType(PIPE_ISPP)], ispTpuConnectionMode, flagStreamLeader, m_flagReprocessing);
    }

    /*
     * MCSC
     */
    previousPipeId = pipeId;

    if (m_flagIspMcscOTF == false && m_flagTpuMcscOTF == false)
        pipeId = PIPE_MCSC;

    /* MCSC */
    nodeType = getNodeType(PIPE_MCSC);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_MCSC;
    m_deviceInfo[pipeId].nodeNum[nodeType] = nodeMcsc;
    m_deviceInfo[pipeId].connectionMode[nodeType] = mcscConnectionMode;
    strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "MCSC_OUTPUT", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
    if (isFastAE == true && mcscConnectionMode == HW_CONNECTION_MODE_M2M_BUFFER_HIDING)
        mcscConnectionMode = false;
    m_sensorIds[pipeId][nodeType] = m_getSensorId(m_deviceInfo[previousPipeId].nodeNum[getNodeType(mcscSrcPipeId)], mcscConnectionMode, flagStreamLeader, m_flagReprocessing);

    /* MCSC0 */
    nodeType = getNodeType(PIPE_MCSC0);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_MCSC0;
    m_deviceInfo[pipeId].nodeNum[nodeType] = nodeMcscp0;
    m_deviceInfo[pipeId].connectionMode[nodeType] = HW_CONNECTION_MODE_OTF;
    strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "MCSC_PREVIEW", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
    m_sensorIds[pipeId][nodeType] = m_getSensorId(m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_MCSC)], true, flagStreamLeader, m_flagReprocessing);

    /* MCSC1 */
    nodeType = getNodeType(PIPE_MCSC1);
    m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_MCSC1;
    m_deviceInfo[pipeId].nodeNum[nodeType] = nodeMcscp1;
    m_deviceInfo[pipeId].connectionMode[nodeType] = HW_CONNECTION_MODE_OTF;
    strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "MCSC_PREVIEW_CALLBACK", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
    m_sensorIds[pipeId][nodeType] = m_getSensorId(m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_MCSC)], true, flagStreamLeader, m_flagReprocessing);

    if (m_cameraId == CAMERA_ID_BACK || m_parameters->getDualMode() == false) {
        /* MCSC2 */
        nodeType = getNodeType(PIPE_MCSC2);
        m_deviceInfo[pipeId].pipeId[nodeType] = PIPE_MCSC2;
        m_deviceInfo[pipeId].nodeNum[nodeType] = nodeMcscp2;
        m_deviceInfo[pipeId].connectionMode[nodeType] = HW_CONNECTION_MODE_OTF;
        strncpy(m_deviceInfo[pipeId].nodeName[nodeType], "MCSC_RECORDING", EXYNOS_CAMERA_NAME_STR_SIZE - 1);
        m_sensorIds[pipeId][nodeType] = m_getSensorId(m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_MCSC)], true, flagStreamLeader, m_flagReprocessing);
    }

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::m_initPipes(void)
{
    CLOGI("INFO(%s[%d])", __FUNCTION__, __LINE__);

    status_t ret = NO_ERROR;
    camera_pipe_info_t pipeInfo[MAX_NODE];
    camera_pipe_info_t nullPipeInfo;

    int pipeId = -1;
    enum NODE_TYPE nodeType = INVALID_NODE;
    enum NODE_TYPE leaderNodeType = OUTPUT_NODE;

    ExynosRect tempRect;
    int maxSensorW = 0, maxSensorH = 0, hwSensorW = 0, hwSensorH = 0;
    int maxPreviewW = 0, maxPreviewH = 0, hwPreviewW = 0, hwPreviewH = 0;
    int maxPictureW = 0, maxPictureH = 0, hwPictureW = 0, hwPictureH = 0;
    int videoH = 0, videoW = 0;
    int bayerFormat = m_parameters->getBayerFormat(PIPE_3AA);
    int hwPreviewFormat = m_parameters->getHwPreviewFormat();
    int previewFormat = m_parameters->getPreviewFormat();
    int videoFormat = m_parameters->getVideoFormat();
    int pictureFormat = m_parameters->getHwPictureFormat();
    int hwVdisformat = m_parameters->getHWVdisFormat();
    struct ExynosConfigInfo *config = m_parameters->getConfig();
    ExynosRect bnsSize;
    ExynosRect bcropSize;
    ExynosRect bdsSize;
    int perFramePos = 0;
    int stride = 0;

#ifdef DEBUG_RAWDUMP
    if (m_parameters->checkBayerDumpEnable()) {
        bayerFormat = CAMERA_DUMP_BAYER_FORMAT;
    }
#endif

    m_parameters->getMaxSensorSize(&maxSensorW, &maxSensorH);
    m_parameters->getHwSensorSize(&hwSensorW, &hwSensorH);

    m_parameters->getMaxPreviewSize(&maxPreviewW, &maxPreviewH);
    m_parameters->getHwPreviewSize(&hwPreviewW, &hwPreviewH);

    m_parameters->getMaxPictureSize(&maxPictureW, &maxPictureH);
    m_parameters->getHwPictureSize(&hwPictureW, &hwPictureH);

    m_parameters->getPreviewBayerCropSize(&bnsSize, &bcropSize, false);
    m_parameters->getPreviewBdsSize(&bdsSize, false);

    m_parameters->getHwVideoSize(&videoW, &videoH);
    /* When high speed recording mode, hw sensor size is fixed.
     * So, maxPreview size cannot exceed hw sensor size
     */
    if (m_parameters->getHighSpeedRecording()) {
        maxPreviewW = hwSensorW;
        maxPreviewH = hwSensorH;
    }

    CLOGI("INFO(%s[%d]): MaxSensorSize(%dx%d), HWSensorSize(%dx%d)", __FUNCTION__, __LINE__, maxSensorW, maxSensorH, hwSensorW, hwSensorH);
    CLOGI("INFO(%s[%d]): MaxPreviewSize(%dx%d), HwPreviewSize(%dx%d)", __FUNCTION__, __LINE__, maxPreviewW, maxPreviewH, hwPreviewW, hwPreviewH);
    CLOGI("INFO(%s[%d]): HWPictureSize(%dx%d)", __FUNCTION__, __LINE__, hwPictureW, hwPictureH);


    /*
     * 3AA
     */
    pipeId = PIPE_3AA;

    /* 3AS */
    nodeType = getNodeType(PIPE_3AA);
    bayerFormat = m_parameters->getBayerFormat(PIPE_3AA);

    if (m_flagFlite3aaOTF == true
#ifdef USE_FLITE_3AA_BUFFER_HIDING_MODE
        && (USE_FLITE_3AA_BUFFER_HIDING_MODE == false
            || ((m_cameraId == CAMERA_ID_FRONT || m_cameraId == CAMERA_ID_BACK_1)
                && m_parameters->getDualMode() == true))
#endif
       ) {
        /* set v4l2 buffer size */
        tempRect.fullW = 32;
        tempRect.fullH = 64;
        tempRect.colorFormat = bayerFormat;

        /* set v4l2 video node buffer count */
        pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_3aa_buffers;
    } else {
        /* set v4l2 buffer size */
        tempRect.fullW = maxSensorW;
        tempRect.fullH = maxSensorH;
        tempRect.colorFormat = bayerFormat;

        /* set v4l2 video node bytes per plane */
        pipeInfo[nodeType].bytesPerPlane[0] = getBayerLineSize(tempRect.fullW, bayerFormat);

        /* set v4l2 video node buffer count */
#ifdef USE_FLITE_3AA_BUFFER_HIDING_MODE
        if (USE_FLITE_3AA_BUFFER_HIDING_MODE == true
            && m_flagFlite3aaOTF == true
            && (m_cameraId == CAMERA_ID_BACK
                || m_parameters->getDualMode() == false))
            pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_hiding_mode_buffers;
        else
#endif
        pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_bayer_buffers;
    }

    /* Set output node default info */
    SET_OUTPUT_DEVICE_BASIC_INFO(PERFRAME_INFO_3AA);

    /* 3AC */
    nodeType = getNodeType(PIPE_3AC);
    perFramePos = PERFRAME_BACK_3AC_POS;
    bayerFormat = m_parameters->getBayerFormat(PIPE_3AC);

    /* set v4l2 buffer size */
    tempRect.fullW = bcropSize.w;
    tempRect.fullH = bcropSize.h;
    tempRect.colorFormat = bayerFormat;

    /* set v4l2 video node bytes per plane */
    pipeInfo[nodeType].bytesPerPlane[0] = getBayerLineSize(tempRect.fullW, bayerFormat);

    /* set v4l2 video node buffer count */
    pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_3aa_buffers;

    /* Set capture node default info */
    SET_CAPTURE_DEVICE_BASIC_INFO();

    /* 3AP */
    nodeType = getNodeType(PIPE_3AP);
    perFramePos = PERFRAME_BACK_3AP_POS;
    bayerFormat = m_parameters->getBayerFormat(PIPE_3AP);

    /* set v4l2 buffer size */
    tempRect.fullW = bdsSize.w;
    tempRect.fullH = bdsSize.h;
    tempRect.colorFormat = bayerFormat;

    /* set v4l2 video node bytes per plane */
    pipeInfo[nodeType].bytesPerPlane[0] = getBayerLineSize(tempRect.fullW, bayerFormat);

    /* set v4l2 video node buffer count */
#ifdef USE_3AA_ISP_BUFFER_HIDING_MODE
    if (USE_3AA_ISP_BUFFER_HIDING_MODE == true
        && m_flag3aaIspOTF == true
        && (m_cameraId == CAMERA_ID_BACK
            || m_parameters->getDualMode() == false))
        pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_hiding_mode_buffers;
    else
#endif
        pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_3aa_buffers;

    /* Set capture node default info */
    SET_CAPTURE_DEVICE_BASIC_INFO();

    /* setup pipe info to 3AA pipe */
    if (m_flag3aaIspOTF == false) {
        ret = m_pipes[pipeId]->setupPipe(pipeInfo, m_sensorIds[pipeId]);
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):3AA setupPipe fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }

        /* clear pipeInfo for next setupPipe */
        for (int i = 0; i < MAX_NODE; i++)
            pipeInfo[i] = nullPipeInfo;
    }

    /*
     * ISP
     */

    /* ISPS */
    if (m_flag3aaIspOTF == false)
        pipeId = PIPE_ISP;
    nodeType = getNodeType(PIPE_ISP);
    bayerFormat = m_parameters->getBayerFormat(PIPE_ISP);

    /* set v4l2 buffer size */
    tempRect.fullW = bdsSize.w;
    tempRect.fullH = bdsSize.h;
    tempRect.colorFormat = bayerFormat;

    /* set v4l2 video node bytes per plane */
    pipeInfo[nodeType].bytesPerPlane[0] = getBayerLineSize(tempRect.fullW, bayerFormat);

    /* set v4l2 video node buffer count */
#ifdef USE_3AA_ISP_BUFFER_HIDING_MODE
    if (USE_3AA_ISP_BUFFER_HIDING_MODE == true
        && m_flag3aaIspOTF == true
        && (m_cameraId == CAMERA_ID_BACK
            || m_parameters->getDualMode() == false))
        pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_hiding_mode_buffers;
    else
#endif
        pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_3aa_buffers;

    /* Set output node default info */
    SET_OUTPUT_DEVICE_BASIC_INFO(PERFRAME_INFO_ISP);

    /* ISPC */
    nodeType = getNodeType(PIPE_ISPC);
    perFramePos = PERFRAME_BACK_ISPC_POS;

    /* set v4l2 buffer size */
    tempRect.fullW = bdsSize.w;
    tempRect.fullH = bdsSize.h;
    tempRect.colorFormat = hwVdisformat;

    /* set v4l2 video node bytes per plane */
    pipeInfo[nodeType].bytesPerPlane[0] = ROUND_UP(tempRect.fullW * 2, CAMERA_16PX_ALIGN);

    /* set v4l2 video node buffer count */
    pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_hwdis_buffers;

    /* Set capture node default info */
    SET_CAPTURE_DEVICE_BASIC_INFO();

    /* ISPP */
    nodeType = getNodeType(PIPE_ISPP);
    perFramePos = PERFRAME_BACK_ISPP_POS;

    /* set v4l2 buffer size */
    tempRect.fullW = bdsSize.w;
    tempRect.fullH = bdsSize.h;
    tempRect.colorFormat = hwVdisformat;

    /* set v4l2 video node bytes per plane */
    pipeInfo[nodeType].bytesPerPlane[0] = ROUND_UP(tempRect.fullW * 2, CAMERA_TPU_CHUNK_ALIGN_W);

    /* set v4l2 video node buffer count */
#ifdef USE_ISP_TPU_BUFFER_HIDING_MODE
        if (USE_ISP_TPU_BUFFER_HIDING_MODE == true
            && m_flagIspTpuOTF == true
            && (m_cameraId == CAMERA_ID_BACK
                || m_parameters->getDualMode() == false))
            pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_hiding_mode_buffers;
        else
#endif
#ifdef USE_ISP_MCSC_BUFFER_HIDING_MODE
        if (USE_ISP_MCSC_BUFFER_HIDING_MODE == true
            && m_flagIspMcscOTF == true
            && (m_cameraId == CAMERA_ID_BACK
                || m_parameters->getDualMode() == false))
            pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_hiding_mode_buffers;
        else
#endif
            pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_hwdis_buffers;

    /* Set capture node default info */
    SET_CAPTURE_DEVICE_BASIC_INFO();

    /* setup pipe info to ISP pipe */
    if (m_flagIspTpuOTF == false && m_flagIspMcscOTF == false) {
        ret = m_pipes[pipeId]->setupPipe(pipeInfo, m_sensorIds[pipeId]);
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):ISP setupPipe fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }

        /* clear pipeInfo for next setupPipe */
        for (int i = 0; i < MAX_NODE; i++)
            pipeInfo[i] = nullPipeInfo;
    }

    /*
     * TPU 
     */

    /* TPU */
    if (m_parameters->getTpuEnabledMode() == true) {
        if (m_flagIspTpuOTF == false)
            pipeId = PIPE_TPU;
        nodeType = getNodeType(PIPE_TPU);

        /* set v4l2 buffer size */
        tempRect.fullW = bdsSize.w;
        tempRect.fullH = bdsSize.h;
        tempRect.colorFormat = hwVdisformat;

        /* set v4l2 video node bytes per plane */
        pipeInfo[nodeType].bytesPerPlane[0] = ROUND_UP(tempRect.fullW * 2, CAMERA_TPU_CHUNK_ALIGN_W);

        /* set v4l2 video node buffer count */
#ifdef USE_ISP_TPU_BUFFER_HIDING_MODE
        if (USE_ISP_TPU_BUFFER_HIDING_MODE == true
            && m_flagIspTpuOTF == true
            && (m_cameraId == CAMERA_ID_BACK
                || m_parameters->getDualMode() == false))
            pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_hiding_mode_buffers;
        else
#endif
            pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_hwdis_buffers;

        /* Set output node default info */
        SET_OUTPUT_DEVICE_BASIC_INFO(PERFRAME_INFO_TPU);

        /* setup pipe info to TPU pipe */
        if (m_flagTpuMcscOTF == false) {
            ret = m_pipes[pipeId]->setupPipe(pipeInfo, m_sensorIds[pipeId]);
            if (ret != NO_ERROR) {
                CLOGE("ERR(%s[%d]):TPU setupPipe fail, ret(%d)", __FUNCTION__, __LINE__, ret);
                /* TODO: exception handling */
                return INVALID_OPERATION;
            }

            /* clear pipeInfo for next setupPipe */
            for (int i = 0; i < MAX_NODE; i++)
                pipeInfo[i] = nullPipeInfo;
        }
    }

    /*
     * MCSC
     */

    /* MCSC */
    if (m_flagIspMcscOTF == false && m_flagTpuMcscOTF == false)
        pipeId = PIPE_MCSC;
    nodeType = getNodeType(PIPE_MCSC);

    /* set v4l2 buffer size */
    tempRect.fullW = bdsSize.w;
    tempRect.fullH = bdsSize.h;
    tempRect.colorFormat = hwVdisformat;

    /* set v4l2 video node bytes per plane */
    pipeInfo[nodeType].bytesPerPlane[0] = ROUND_UP(tempRect.fullW * 2, CAMERA_16PX_ALIGN);

    /* set v4l2 video node buffer count */
#ifdef USE_ISP_MCSC_BUFFER_HIDING_MODE
    if (USE_ISP_MCSC_BUFFER_HIDING_MODE == true
        && m_flagIspMcscOTF == true
        && (m_cameraId == CAMERA_ID_BACK
            || m_parameters->getDualMode() == false))
        pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_hiding_mode_buffers;
    else
#endif
#ifdef USE_TPU_MCSC_BUFFER_HIDING_MODE
        if (USE_ISP_TPU_BUFFER_HIDING_MODE == true
            && m_flagTpuMcscOTF == true
            && (m_cameraId == CAMERA_ID_BACK
                || m_parameters->getDualMode() == false))
            pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_hiding_mode_buffers;
        else
#endif
            pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_hwdis_buffers;

    /* Set output node default info */
    SET_OUTPUT_DEVICE_BASIC_INFO(PERFRAME_INFO_MCSC);

    /* MCSC0 */
    nodeType = getNodeType(PIPE_MCSC0);
    perFramePos = PERFRAME_BACK_SCP_POS;

    /* set v4l2 buffer size */
    stride = m_parameters->getHwPreviewStride();
    CLOGV("INFO(%s[%d]):stride=%d", __FUNCTION__, __LINE__, stride);
    tempRect.fullW = stride;
    tempRect.fullH = hwPreviewH;
    tempRect.colorFormat = hwPreviewFormat;

    /* set v4l2 video node bytes per plane */
#ifdef USE_BUFFER_WITH_STRIDE
    /* to use stride for preview buffer, set the bytesPerPlane */
    pipeInfo[nodeType].bytesPerPlane[0] = hwPreviewW;
#endif

    /* set v4l2 video node buffer count */
    if (m_parameters->increaseMaxBufferOfPreview() == true)
        pipeInfo[nodeType].bufInfo.count = m_parameters->getPreviewBufferCount();
    else
        pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_preview_buffers;

    /* Set capture node default info */
    SET_CAPTURE_DEVICE_BASIC_INFO();

    /* MCSC1 */
    nodeType = getNodeType(PIPE_MCSC1);
    perFramePos = PERFRAME_BACK_MCSC1_POS;

    /* set v4l2 buffer size */
    stride = m_parameters->getHwPreviewStride();
    CLOGV("INFO(%s[%d]):stride=%d", __FUNCTION__, __LINE__, stride);
    tempRect.fullW = stride;
    tempRect.fullH = hwPreviewH;
    tempRect.colorFormat = previewFormat;

    /* set v4l2 video node bytes per plane */
#ifdef USE_BUFFER_WITH_STRIDE
    /* to use stride for preview buffer, set the bytesPerPlane */
    pipeInfo[nodeType].bytesPerPlane[0] = hwPreviewW;
#endif

    /* set v4l2 video node buffer count */
    if (m_parameters->increaseMaxBufferOfPreview() == true)
        pipeInfo[nodeType].bufInfo.count = m_parameters->getPreviewBufferCount();
    else
        pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_preview_buffers;

    /* Set capture node default info */
    SET_CAPTURE_DEVICE_BASIC_INFO();

    if (m_cameraId == CAMERA_ID_BACK || m_parameters->getDualMode() == false) {
        /* MCSC2 */
        nodeType = getNodeType(PIPE_MCSC2);
        perFramePos = PERFRAME_BACK_MCSC2_POS;

        /* set v4l2 buffer size */
        stride = m_parameters->getHwPreviewStride();
        CLOGV("INFO(%s[%d]):stride=%d", __FUNCTION__, __LINE__, stride);
        tempRect.fullW = videoW;
        tempRect.fullH = videoH;
        tempRect.colorFormat = videoFormat;

        /* set v4l2 video node buffer count */
        if (m_parameters->increaseMaxBufferOfPreview() == true)
            pipeInfo[nodeType].bufInfo.count = m_parameters->getPreviewBufferCount();
        else
            pipeInfo[nodeType].bufInfo.count = config->current->bufInfo.num_recording_buffers;

        /* Set capture node default info */
        SET_CAPTURE_DEVICE_BASIC_INFO();
    }

    ret = m_pipes[pipeId]->setupPipe(pipeInfo, m_sensorIds[pipeId]);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):MCSC setupPipe fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::m_initPipesFastenAeStable(int32_t numFrames,
                                                                         int hwSensorW,
                                                                         int hwSensorH)
{
    status_t ret = NO_ERROR;
    camera_pipe_info_t pipeInfo[MAX_NODE];
    camera_pipe_info_t nullPipeInfo;

    int pipeId = -1;
    enum NODE_TYPE nodeType = INVALID_NODE;
    enum NODE_TYPE leaderNodeType = OUTPUT_NODE;

    ExynosRect tempRect;
    int bayerFormat = m_parameters->getBayerFormat(PIPE_3AA);
    int hwVdisformat = m_parameters->getHWVdisFormat();
    int hwPreviewFormat = m_parameters->getHwPreviewFormat();
    int perFramePos = 0;

#ifdef DEBUG_RAWDUMP
    if (m_parameters->checkBayerDumpEnable()) {
        bayerFormat = CAMERA_DUMP_BAYER_FORMAT;
    }
#endif

    pipeId = PIPE_3AA;

    /* 3AS */
    nodeType = getNodeType(PIPE_3AA);
    bayerFormat = m_parameters->getBayerFormat(PIPE_3AA);

    /* set v4l2 buffer size */
    tempRect.fullW = 32;
    tempRect.fullH = 64;
    tempRect.colorFormat = bayerFormat;

    pipeInfo[nodeType].bufInfo.count = numFrames;

    SET_OUTPUT_DEVICE_BASIC_INFO(PERFRAME_INFO_3AA);

    /* 3AC */
    nodeType = getNodeType(PIPE_3AC);
    perFramePos = PERFRAME_BACK_3AC_POS;
    bayerFormat = m_parameters->getBayerFormat(PIPE_3AC);

    /* set v4l2 buffer size */
    tempRect.fullW = hwSensorW;
    tempRect.fullH = hwSensorH;
    tempRect.colorFormat = bayerFormat;

    pipeInfo[nodeType].bufInfo.count = numFrames;

    SET_CAPTURE_DEVICE_BASIC_INFO();

    /* 3AP */
    nodeType = getNodeType(PIPE_3AP);
    perFramePos = PERFRAME_BACK_3AP_POS;
    bayerFormat = m_parameters->getBayerFormat(PIPE_3AP);

    tempRect.colorFormat = bayerFormat;
    pipeInfo[nodeType].bufInfo.count = numFrames;

    SET_CAPTURE_DEVICE_BASIC_INFO();

    /* setup pipe info to 3AA pipe */
    if (m_flag3aaIspOTF == false) {
        ret = m_pipes[pipeId]->setupPipe(pipeInfo, m_sensorIds[pipeId]);
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):3AA setupPipe fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }

        /* clear pipeInfo for next setupPipe */
        for (int i = 0; i < MAX_NODE; i++)
            pipeInfo[i] = nullPipeInfo;
    }

    /* ISPS */
    if (m_flag3aaIspOTF == false)
        pipeId = PIPE_ISP;
    nodeType = getNodeType(PIPE_ISP);
    bayerFormat = m_parameters->getBayerFormat(PIPE_ISP);

    tempRect.colorFormat = bayerFormat;
    pipeInfo[nodeType].bufInfo.count = numFrames;

    SET_OUTPUT_DEVICE_BASIC_INFO(PERFRAME_INFO_ISP);

    /* ISPC */
    nodeType = getNodeType(PIPE_ISPC);
    perFramePos = PERFRAME_BACK_ISPC_POS;

    tempRect.colorFormat = hwVdisformat;

    pipeInfo[nodeType].bufInfo.count = numFrames;

    SET_CAPTURE_DEVICE_BASIC_INFO();

    /* ISPP */
    nodeType = getNodeType(PIPE_ISPP);
    perFramePos = PERFRAME_BACK_ISPP_POS;

    pipeInfo[nodeType].bufInfo.count = numFrames;

    SET_CAPTURE_DEVICE_BASIC_INFO();

    /* setup pipe info to ISP pipe */
    if (m_flagIspTpuOTF == false && m_flagIspMcscOTF == false) {
        ret = m_pipes[pipeId]->setupPipe(pipeInfo, m_sensorIds[pipeId]);
        if (ret != NO_ERROR) {
            CLOGE("ERR(%s[%d]):ISP setupPipe fail, ret(%d)", __FUNCTION__, __LINE__, ret);
            /* TODO: exception handling */
            return INVALID_OPERATION;
        }

        /* clear pipeInfo for next setupPipe */
        for (int i = 0; i < MAX_NODE; i++)
            pipeInfo[i] = nullPipeInfo;
    }

    /* TPU */
    if (m_parameters->getTpuEnabledMode() == true) {
        if (m_flagIspTpuOTF == false)
            pipeId = PIPE_TPU;
        nodeType = getNodeType(PIPE_TPU);

        pipeInfo[nodeType].bufInfo.count = numFrames;

        SET_OUTPUT_DEVICE_BASIC_INFO(PERFRAME_INFO_ISP);

        /* setup pipe info to TPU pipe */
        if (m_flagTpuMcscOTF == false) {
            ret = m_pipes[pipeId]->setupPipe(pipeInfo, m_sensorIds[pipeId]);
            if (ret != NO_ERROR) {
                CLOGE("ERR(%s[%d]):TPU setupPipe fail, ret(%d)", __FUNCTION__, __LINE__, ret);
                /* TODO: exception handling */
                return INVALID_OPERATION;
            }

            /* clear pipeInfo for next setupPipe */
            for (int i = 0; i < MAX_NODE; i++)
                pipeInfo[i] = nullPipeInfo;
        }
    }

    /* MCSC */
    if (m_flagIspMcscOTF == false && m_flagTpuMcscOTF == false)
        pipeId = PIPE_MCSC;
    nodeType = getNodeType(PIPE_MCSC);

    pipeInfo[nodeType].bufInfo.count = numFrames;

    SET_OUTPUT_DEVICE_BASIC_INFO(PERFRAME_INFO_MCSC);

    /* MCSC0 */
    nodeType = getNodeType(PIPE_MCSC0);
    perFramePos = PERFRAME_BACK_SCP_POS;

    tempRect.colorFormat = hwPreviewFormat;

    pipeInfo[nodeType].bufInfo.count = numFrames;

    SET_CAPTURE_DEVICE_BASIC_INFO();

    /* MCSC1 */
    nodeType = getNodeType(PIPE_MCSC1);
    perFramePos = PERFRAME_BACK_MCSC1_POS;

    pipeInfo[nodeType].bufInfo.count = numFrames;

    SET_CAPTURE_DEVICE_BASIC_INFO();

    /* MCSC2 */
    nodeType = getNodeType(PIPE_MCSC2);
    perFramePos = PERFRAME_BACK_MCSC2_POS;

    pipeInfo[nodeType].bufInfo.count = numFrames;

    SET_CAPTURE_DEVICE_BASIC_INFO();

    ret = m_pipes[pipeId]->setupPipe(pipeInfo, m_sensorIds[pipeId]);
    if (ret != NO_ERROR) {
        CLOGE("ERR(%s[%d]):MCSC setupPipe fail, ret(%d)", __FUNCTION__, __LINE__, ret);
        /* TODO: exception handling */
        return INVALID_OPERATION;
    }

    return NO_ERROR;
}

status_t ExynosCameraFrameFactoryPreview::m_fillNodeGroupInfo(ExynosCameraFrame *frame)
{
    camera2_node_group node_group_info_3aa;
    camera2_node_group node_group_info_isp;
    camera2_node_group node_group_info_tpu;
    camera2_node_group node_group_info_mcsc;
    camera2_node_group *node_group_info_temp;

    int zoom = m_parameters->getZoomLevel();
    int pipeId = -1;
    uint32_t perframePosition = 0;

    memset(&node_group_info_3aa, 0x0, sizeof(camera2_node_group));
    memset(&node_group_info_isp, 0x0, sizeof(camera2_node_group));
    memset(&node_group_info_tpu, 0x0, sizeof(camera2_node_group));
    memset(&node_group_info_mcsc, 0x0, sizeof(camera2_node_group));


    /* 3AA */
    pipeId = PIPE_3AA;
    node_group_info_temp = &node_group_info_3aa;

    node_group_info_temp->leader.request = 1;

    node_group_info_temp->capture[perframePosition].request = m_request3AC;
    node_group_info_temp->capture[perframePosition].vid = m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_3AC)] - FIMC_IS_VIDEO_BAS_NUM;
    perframePosition++;

    node_group_info_temp->capture[perframePosition].request = m_request3AP;
    node_group_info_temp->capture[perframePosition].vid = m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_3AP)] - FIMC_IS_VIDEO_BAS_NUM;
    perframePosition++;

    /* ISP */
    if (m_flag3aaIspOTF == false) {
        pipeId = PIPE_ISP;
        perframePosition = 0;
        node_group_info_temp = &node_group_info_isp;
        node_group_info_temp->leader.request = 1;
    }

    /* TPU */
    if (m_parameters->getTpuEnabledMode() == true && m_flagIspTpuOTF == false) {
        node_group_info_temp->capture[perframePosition].request = m_requestISPP;
        node_group_info_temp->capture[perframePosition].vid = m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_ISPP)] - FIMC_IS_VIDEO_BAS_NUM;
        perframePosition++;

        pipeId = PIPE_TPU;
        perframePosition = 0;
        node_group_info_temp = &node_group_info_tpu;
        node_group_info_temp->leader.request = 1;
    } else if (m_parameters->getTpuEnabledMode() == false && m_flagIspMcscOTF == false) {
        node_group_info_temp->capture[perframePosition].request = m_requestISPC;
        node_group_info_temp->capture[perframePosition].vid = m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_ISPC)] - FIMC_IS_VIDEO_BAS_NUM;
        perframePosition++;
    }

    /* MCSC */
    if (m_flagIspMcscOTF == false && m_flagTpuMcscOTF == false) {
        pipeId = PIPE_MCSC;
        perframePosition = 0;
        node_group_info_temp = &node_group_info_mcsc;
        node_group_info_temp->leader.request = 1;
    }

    node_group_info_temp->capture[perframePosition].request = m_requestMCSC0;
    node_group_info_temp->capture[perframePosition].vid = m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_MCSC0)] - FIMC_IS_VIDEO_BAS_NUM;
    perframePosition++;

    node_group_info_temp->capture[perframePosition].request = m_requestMCSC1;
    node_group_info_temp->capture[perframePosition].vid = m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_MCSC1)] - FIMC_IS_VIDEO_BAS_NUM;
    perframePosition++;

    if (m_cameraId == CAMERA_ID_BACK || m_parameters->getDualMode() == false) {
        node_group_info_temp->capture[perframePosition].request = m_requestMCSC2;
        node_group_info_temp->capture[perframePosition].vid = m_deviceInfo[pipeId].nodeNum[getNodeType(PIPE_MCSC2)] - FIMC_IS_VIDEO_BAS_NUM;
    }

    updateNodeGroupInfo(
            PIPE_3AA,
            m_parameters,
            &node_group_info_3aa);
    frame->storeNodeGroupInfo(&node_group_info_3aa, PERFRAME_INFO_3AA, zoom);

    if (m_flag3aaIspOTF == false) {
        updateNodeGroupInfo(
                PIPE_ISP,
                m_parameters,
                &node_group_info_isp);
        frame->storeNodeGroupInfo(&node_group_info_isp, PERFRAME_INFO_ISP, zoom);
    }

    if (m_parameters->getTpuEnabledMode() == true && m_flagIspTpuOTF == false) {
        updateNodeGroupInfo(
                PIPE_TPU,
                m_parameters,
                &node_group_info_tpu);
        frame->storeNodeGroupInfo(&node_group_info_tpu, PERFRAME_INFO_TPU, zoom);
    }

    if (m_flagIspMcscOTF == false && m_flagTpuMcscOTF == false) {
        updateNodeGroupInfo(
                PIPE_MCSC,
                m_parameters,
                &node_group_info_mcsc);
        frame->storeNodeGroupInfo(&node_group_info_mcsc, PERFRAME_INFO_MCSC, zoom);
    }

    return NO_ERROR;
}

void ExynosCameraFrameFactoryPreview::m_init(void)
{
    m_flagReprocessing = false;
}

}; /* namespace android */
