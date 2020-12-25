/*
* Copyright (c) 2019-2020, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file     decode_status_report.cpp
//! \brief    Defines the common interface for decode status reporter
//! \details
//!
#include <cmath>
#include "decode_status_report.h"

namespace decode {

    DecodeStatusReport::DecodeStatusReport(
        DecodeAllocator* allocator, bool enableRcs):
        m_enableRcs(enableRcs),
        m_allocator(allocator)
    {
        m_sizeOfReport = sizeof(DecodeStatusReportData);
    }

    DecodeStatusReport::~DecodeStatusReport()
    {
        Destroy();
    }

    MOS_STATUS DecodeStatusReport::Create()
    {
        DECODE_FUNC_CALL();

        // Allocate status buffer which includes completed count and decode status
        m_statusBufMfx = m_allocator->AllocateBuffer(m_completedCountSize + m_statusBufSizeMfx * m_statusNum, "StatusQueryBufferMfx", resourceInternalWrite, true, 0, true);
        DECODE_CHK_NULL(m_statusBufMfx);
        m_completedCountBuf = &(m_statusBufMfx->OsResource);

        DECODE_CHK_STATUS(m_allocator->SkipResourceSync(m_statusBufMfx));
        uint8_t *data = (uint8_t *)m_allocator->LockResouceForRead(m_statusBufMfx);
        DECODE_CHK_NULL(data);

        // complete count located at the beginging of the status buffer, following with decode status
        m_completedCount = (uint32_t *)data;
        m_dataStatusMfx  = data + m_completedCountSize;

        if (m_enableRcs)
        {
            m_statusBufRcs = m_allocator->AllocateBuffer(m_statusBufSizeRcs * m_statusNum, "StatusQueryBufferRcs", resourceInternalWrite, true, 0, true);

            DECODE_CHK_STATUS(m_allocator->SkipResourceSync(m_statusBufRcs));
            m_dataStatusRcs = (uint8_t *)m_allocator->LockResouceForRead(m_statusBufRcs);
            DECODE_CHK_NULL(m_dataStatusRcs);
        }

        m_submittedCount = 0;
        m_reportedCount = 0;

        m_statusBufAddr = MOS_NewArray(StatusBufAddr, statusReportMaxNum);
        DECODE_CHK_NULL(m_statusBufAddr);

        m_statusBufAddr[statusReportGlobalCount].osResource = m_completedCountBuf;
        m_statusBufAddr[statusReportGlobalCount].offset = 0;
        m_statusBufAddr[statusReportGlobalCount].bufSize = sizeof(uint32_t) * 2;

        for(int i = 0; i < statusReportGlobalCount; i++)
        {
            m_statusBufAddr[i].osResource = &m_statusBufMfx->OsResource;
            m_statusBufAddr[i].bufSize = m_statusBufSizeMfx;
        }

        m_statusBufAddr[statusReportRcs].osResource = &m_statusBufRcs->OsResource;
        m_statusBufAddr[statusReportRcs].bufSize = m_statusBufSizeRcs;

        SetOffsetsForStatusBuf();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS DecodeStatusReport::Init(void *inputPar)
    {
        DECODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        DecodeStatusParameters* inputParameters = (DecodeStatusParameters*)inputPar;
        uint32_t submitIndex = CounterToIndex(m_submittedCount);

        if (inputParameters)
        {
            m_statusReportData[submitIndex].codecStatus = CODECHAL_STATUS_UNAVAILABLE;
            m_statusReportData[submitIndex].statusReportNumber = inputParameters->statusReportFeedbackNumber;
            m_statusReportData[submitIndex].currDecodedPic = inputParameters->currOriginalPic;
            m_statusReportData[submitIndex].currDecodedPicRes = inputParameters->currDecodedPicRes;
        }

        DecodeStatusMfx* decodeStatusMfx = (DecodeStatusMfx*)(m_dataStatusMfx + submitIndex * m_statusBufSizeMfx);
        decodeStatusMfx->status = querySkipped;

        if (m_enableRcs)
        {
            DecodeStatusRcs *decodeStatusRcs = (DecodeStatusRcs *)(m_dataStatusRcs + submitIndex * m_statusBufSizeRcs);
            decodeStatusRcs->status = querySkipped;
        }

        return eStatus;
    }

    MOS_STATUS DecodeStatusReport::Reset()
    {
        DECODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        m_submittedCount++;
        uint32_t submitIndex = CounterToIndex(m_submittedCount);

        DecodeStatusMfx* decodeStatusMfx = (DecodeStatusMfx*)(m_dataStatusMfx + submitIndex * m_statusBufSizeMfx);
        MOS_ZeroMemory((uint8_t*)decodeStatusMfx, m_statusBufSizeMfx);

        if (m_enableRcs)
        {
            DecodeStatusRcs *decodeStatusRcs = (DecodeStatusRcs *)(m_dataStatusRcs + submitIndex * m_statusBufSizeRcs);
            MOS_ZeroMemory((uint8_t *)decodeStatusRcs, m_statusBufSizeRcs);
        }

        return eStatus;
    }

    MOS_STATUS DecodeStatusReport::ParseStatus(void* report, uint32_t index)
    {
        DECODE_FUNC_CALL();

        DecodeStatusMfx* decodeStatusMfx = nullptr;
        DecodeStatusRcs* decodeStatusRcs = nullptr;

        DecodeStatusReportData* statusReportData = &m_statusReportData[index];

        decodeStatusMfx = (DecodeStatusMfx*)(m_dataStatusMfx + index * m_statusBufSizeMfx);
        bool mfxCompleted = (decodeStatusMfx->status == queryEnd) || (decodeStatusMfx->status == querySkipped);

        bool rcsCompleted = false;
        if (m_enableRcs)
        {
            decodeStatusRcs = (DecodeStatusRcs *)(m_dataStatusRcs + index * m_statusBufSizeRcs);
            rcsCompleted    = (decodeStatusRcs->status == queryEnd) || (decodeStatusRcs->status == querySkipped);
        }
        else
        {
            rcsCompleted    = true;
        }

        UpdateCodecStatus(statusReportData, decodeStatusMfx, mfxCompleted && rcsCompleted);

        // The frame is completed, notify the observers
        if (statusReportData->codecStatus == CODECHAL_STATUS_SUCCESSFUL)
        {
            NotifyObservers(decodeStatusMfx, decodeStatusRcs, statusReportData);
        }

        *((DecodeStatusReportData*)report) = *statusReportData;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS DecodeStatusReport::SetStatus(void *report, uint32_t index, bool outOfRange)
    {
        DECODE_FUNC_CALL();

        DecodeStatusReportData* statusReportData = &m_statusReportData[index];

        statusReportData->codecStatus = outOfRange ? CODECHAL_STATUS_UNAVAILABLE : CODECHAL_STATUS_INCOMPLETE;

        *((DecodeStatusReportData*)report) = *statusReportData;

        return MOS_STATUS_SUCCESS;
    }

    void DecodeStatusReport::SetOffsetsForStatusBuf()
    {
        const uint32_t mfxStatusOffset = m_completedCountSize;
        m_statusBufAddr[statusReportMfx].offset      = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, status);
        m_statusBufAddr[DecErrorStatusOffset].offset = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_mmioErrorStatusReg);
        m_statusBufAddr[DecMBCountOffset].offset     = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_mmioMBCountReg);
        m_statusBufAddr[DecFrameCrcOffset].offset    = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_mmioFrameCrcReg);
        m_statusBufAddr[CsEngineIdOffset_0].offset   = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_mmioCsEngineIdReg[0]);
        m_statusBufAddr[CsEngineIdOffset_1].offset   = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_mmioCsEngineIdReg[1]);
        m_statusBufAddr[CsEngineIdOffset_2].offset   = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_mmioCsEngineIdReg[2]);
        m_statusBufAddr[CsEngineIdOffset_3].offset   = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_mmioCsEngineIdReg[3]);
        m_statusBufAddr[CsEngineIdOffset_4].offset   = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_mmioCsEngineIdReg[4]);
        m_statusBufAddr[CsEngineIdOffset_5].offset   = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_mmioCsEngineIdReg[5]);
        m_statusBufAddr[CsEngineIdOffset_6].offset   = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_mmioCsEngineIdReg[6]);
        m_statusBufAddr[CsEngineIdOffset_7].offset   = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_mmioCsEngineIdReg[7]);
        m_statusBufAddr[HucErrorStatus2Mask].offset  = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_hucErrorStatus2);
        m_statusBufAddr[HucErrorStatus2Reg].offset   = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_hucErrorStatus2) + sizeof(uint32_t);
        m_statusBufAddr[HucErrorStatusMask].offset   = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_hucErrorStatus);
        m_statusBufAddr[HucErrorStatusReg].offset    = mfxStatusOffset + CODECHAL_OFFSETOF(DecodeStatusMfx, m_hucErrorStatus) + sizeof(uint32_t);

        const uint32_t rcsStatusOffset = 0;
        m_statusBufAddr[statusReportRcs].offset      = rcsStatusOffset + CODECHAL_OFFSETOF(DecodeStatusRcs, status);
    }

    MOS_STATUS DecodeStatusReport::UpdateCodecStatus(
        DecodeStatusReportData* statusReportData,
        DecodeStatusMfx* decodeStatus,
        bool completed)
    {
        DECODE_CHK_NULL(statusReportData);
        DECODE_CHK_NULL(decodeStatus);

        if (!completed)
        {
            statusReportData->codecStatus = CODECHAL_STATUS_INCOMPLETE;
        }
        else
        {
            statusReportData->codecStatus = CODECHAL_STATUS_SUCCESSFUL;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS DecodeStatusReport::Destroy()
    {
        DECODE_FUNC_CALL();

        if (m_statusBufMfx != nullptr)
        {
            m_allocator->UnLock(m_statusBufMfx);
            m_allocator->Destroy(m_statusBufMfx);
            m_statusBufMfx = nullptr;
            m_completedCountBuf = nullptr;
        }

        if (m_statusBufRcs != nullptr)
        {
            m_allocator->UnLock(m_statusBufRcs);
            m_allocator->Destroy(m_statusBufRcs);
            m_statusBufRcs = nullptr;
        }

        if (m_statusBufAddr != nullptr)
        {
            MOS_DeleteArray(m_statusBufAddr);
            m_statusBufAddr = nullptr;
        }

        return MOS_STATUS_SUCCESS;
    }
}
