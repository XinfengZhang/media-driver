/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file      codechal_decode_avc.h
//! \brief     This modules implements Render interface layer for AVC decoding to be used on all operating systems/DDIs, across CODECHAL components.
//!
//!
//! \file     codechal_decode_avc.h
//! \brief    Defines the decode interface extension for AVC.
//! \details  Defines all types, macros, and functions required by CodecHal for AVC decoding.
//!           Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODER_AVC_H__
#define __CODECHAL_DECODER_AVC_H__

#include "codechal.h"
#include "codechal_decoder.h"
#include "codechal_decode_sfc_avc.h"
#include "codechal_common_avc.h"

//!
//! \def CODECHAL_DECODE_AVC_MONOPIC_CHROMA_DEFAULT
//! default chroma value for mono picture
//!
#define CODECHAL_DECODE_AVC_MONOPIC_CHROMA_DEFAULT        0x80

//!
//! \def CODECHAL_DECODE_AVC_INVALID_FRAME_IDX
//! invalid value for invalid frame index
//!
#define CODECHAL_DECODE_AVC_INVALID_FRAME_IDX             0xFF

//!
//! \def CODECHAL_DECODE_AVC_INVALID_REFPIC_VALUE
//! invalid value for invalid ref frame index
//!
#define CODECHAL_DECODE_AVC_INVALID_REFPIC_VALUE          0x80

//!
//! \def CODECHAL_DECODE_MAX_SLICE_HDR_SET_SIZE
//! max slice header set size
//!
#define CODECHAL_DECODE_MAX_SLICE_HDR_SET_SIZE            256

//!
//! \def CODECHAL_DECODE_MAX_SLICE_HDR_LEN_SIZE
//! max slice header length size
//!
#define CODECHAL_DECODE_MAX_SLICE_HDR_LEN_SIZE            2

//!
//! \def CODECHAL_DECODE_MAX_SLICE_HDR_SET_BUF_SIZE
//! max slice header buf size
//!
#define CODECHAL_DECODE_MAX_SLICE_HDR_SET_BUF_SIZE        CODECHAL_DECODE_MAX_SLICE_HDR_SET_SIZE - CODECHAL_DECODE_MAX_SLICE_HDR_LEN_SIZE

//!
//! \def CODECHAL_DECODE_AVC_MAX_NUM_MVC_VIEWS
//! max mvc views
//!
#define CODECHAL_DECODE_AVC_MAX_NUM_MVC_VIEWS              16

typedef class CodechalDecodeAvc *PCODECHAL_DECODE_AVC_STATE;

//!
//! \class CodechalDecodeAvc
//! \brief This class defines the member fields, functions etc used by AVC decoder.
//!
class CodechalDecodeAvc : public CodechalDecode
{
public:

    //!
    //! \brief    Define params for AVC picture decoding
    //!
    typedef struct
    {
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS  PipeModeSelectParams;
        MHW_VDBOX_SURFACE_PARAMS           SurfaceParams;
        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS     PipeBufAddrParams;
        MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS IndObjBaseAddrParams;
        MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS BspBufBaseAddrParams;
        MHW_VDBOX_QM_PARAMS                QmParams;
        MHW_VDBOX_PIC_ID_PARAMS            PicIdParams;
        MHW_VDBOX_AVC_IMG_PARAMS           ImgParams;
        MHW_VDBOX_AVC_DIRECTMODE_PARAMS    AvcDirectmodeParams;
    } PIC_MHW_PARAMS;

    //!
    //! \brief  Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //! \param    [in] standardInfo
    //!           The information of decode standard for this instance
    //!
    CodechalDecodeAvc(
        CodechalHwInterface *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalDecodeAvc();

    //!
    //! \brief    Allocate and initialize AVC decoder standard
    //! \param    [in] settings
    //!           Pointer to CODECHAL_SETTINGS
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateStandard(
        PCODECHAL_SETTINGS          settings) override;

    //!
    //! \brief  Set states for each frame to prepare for AVC decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetFrameStates() override;

    //!
    //! \brief    AVC decoder state level function
    //! \details  State level function for AVC decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodeStateLevel() override;

    //!
    //! \brief    AVC decoder primitive level function
    //! \details  Primitive level function for GEN specific AVC decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodePrimitiveLevel() override;

    //!
    //! \brief    AVC decoder downsampling calc function
    //! \details  calc downsample param for GEN specific AVC decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  CalcDownsamplingParams(
        void                        *picParams,
        uint32_t                    *refSurfWidth,
        uint32_t                    *refSurfHeight,
        MOS_FORMAT                  *format,
        uint8_t                     *frameIdx) override;

    //!
    //! \brief    Allocate fixed sized resources
    //! \details  Allocate fixed sized resources AVC decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          AllocateResources_FixedSizes();

    //!
    //! \brief    Allocate variable sized resources
    //! \details  Allocate variable sized resources in AVC decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          AllocateResources_VariableSizes();

    //!
    //! \brief    Allocate invalid ref buffer
    //! \details  Allocate invalid ref buffer in AVC decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          AllocateInvalidRefBuffer();

    //!
    //! \brief    Set Picture Struct
    //! \details  Set Picture Struct in AVC decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          SetPictureStructs();

    //!
    //! \brief    Set and Alloc Dmv Buffer
    //! \details  Set and Alloc Dmv Buffer in AVC decode driver
    //!
    //! \param    [in] avcMVBufList
    //!           Pointer to MV Buf List
    //! \param    [in] usedForRef
    //!           indicate it is used for ref
    //! \param    [in] frameIdx
    //!           frame idx
    //! \param    [in] avcDmvBufferSize
    //!           Dmv Bufer Size
    //! \param    [out] dmvIdx
    //!           Dmv Index
    //! \param    [out] avcDmvBuffers
    //!           Dmv Buffers
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          SetAndAllocateDmvBufferIndex(
        PCODEC_AVC_DMV_LIST         avcMVBufList,
        bool                        usedForRef,
        uint8_t                     frameIdx,
        uint32_t                    avcDmvBufferSize,
        uint8_t                     *dmvIdx,
        MOS_RESOURCE                *avcDmvBuffers);

    //!
    //! \brief    Init Mvc Dummy Dmv Buffer
    //! \details  Init Mvc Dummy Dmv Buffer in AVC decode driver
    //!
    //! \param    [in] mvcWaDummyDmvBuf
    //!           Pointer to Mvc Wa Dummy Dmv Buf
    //! \param    [in] size
    //!           Dmv Bufer Size
    //! \param    [out] mvcDummyDmvBuffer
    //!           Pointer to Mvc Dummy Dmv Buffer Resource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          InitMvcDummyDmvBuffer(
        uint32_t                    *mvcWaDummyDmvBuf,
        uint32_t                    size,
        PMOS_RESOURCE               mvcDummyDmvBuffer);

    //!
    //! \brief    Initialize AVC picture HW parameters
    //! \details  Initialize picture parameters for GEN specific AVC decoder
    //!
    //! \param    [in] picMhwParams
    //!           Pointer to PIC_MHW_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          InitPicMhwParams(
        PIC_MHW_PARAMS              *picMhwParams);

    //!
    //! \brief    Add picture commands
    //! \details  Add picture commands for GEN specific AVC decoder
    //!
    //! \param    [in] cmdBuf
    //!           Pointer to PMOS_COMMAND_BUFFER
    //! \param    [in] picMhwParams
    //!           Pointer to PIC_MHW_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS  AddPictureCmds(
        PMOS_COMMAND_BUFFER         cmdBuf,
        PIC_MHW_PARAMS              *picMhwParams);

    //!
    //! \brief    Parse AVC slice parameters
    //! \details  Parse slice parameters for GEN specific AVC decoder
    //!
    //! \param    [in] cmdBuf
    //!           Pointer to PMOS_COMMAND_BUFFER
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          ParseSlice(
        PMOS_COMMAND_BUFFER         cmdBuf);

    //!
    //! \brief    Send Slice Command
    //! \details  Send Slice Command in AVC decode driver
    //!
    //! \param    [in] avcSliceState
    //!           Pointer to AVC Slice State
    //! \param    [out] cmdBuffer
    //!           Pointer to Command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          SendSlice(
        PMHW_VDBOX_AVC_SLICE_STATE      avcSliceState,
        PMOS_COMMAND_BUFFER             cmdBuffer);

    //!
    //! \brief    Constrcut Mono Picture
    //! \details  Constrcut Mono Picture in AVC decode driver, Write 0x80 in the chroma plane for Monochrome clips
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          FormatAvcMonoPicture();

    MOS_STATUS InitMmcState() override;

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpMvcExtPicParams(
        PCODEC_MVC_EXT_PIC_PARAMS mvcExtPicParams);

    MOS_STATUS DumpPicParams(
        PCODEC_AVC_PIC_PARAMS picParams);

    MOS_STATUS DumpSliceParams(
        PCODEC_AVC_SLICE_PARAMS sliceParams,
        uint32_t                numSlices);

    MOS_STATUS DumpIQParams(
        PCODECHAL_AVC_IQ_MATRIX_PARAMS matrixData);
#endif

private:
    //!
    //! \brief  Indicates whether or not the SFC is inuse
    //! \return If SFC is inuse
    //!
    bool IsSfcInUse(PCODECHAL_SETTINGS codecHalSettings) override
    {
        return (codecHalSettings->bDownsamplingHinted && MEDIA_IS_SKU(m_skuTable, FtrSFCPipe));
    }

public:
                                                                                        // Parameters passed by application
    uint16_t                        u16PicWidthInMb;                                    //!< Picture Width in MB
    uint16_t                        u16PicHeightInMb;                                   //!< Picture Height in MB
    uint16_t                        u16PicWidthInMbLastMaxAlloced;                      //!< Max Picture Width in MB  used for buffer allocation in past frames
    uint16_t                        u16PicHeightInMbLastMaxAlloced;                     //!< Max Picture Height in MB used for buffer allocation in past frames
    uint32_t                        u32DataSize;                                        //!< Data size
    uint32_t                        u32DataOffset;                                      //!< Date offset
    uint32_t                        u32NumSlices;                                       //!< Num of slices
    uint32_t                        u32AvcDmvBufferSize;                                //!< DMV Buffer Size
    uint8_t                         u8AvcMvBufferIndex;                                 //!< MV Buffer Index
    uint16_t                        u16BsdMpcRowStoreScratchBufferPicWidthInMb;         //!< Bsd Mpc RowStore Scratch Buffer Pic Width in MB
    uint16_t                        u16MfdIntraRowStoreScratchBufferPicWidthInMb;       //!< Mfd Intra RowStore Scratch Buffer Pic Width in MB
    uint16_t                        u16MprRowStoreScratchBufferPicWidthInMb;            //!< Mpr RowStore Scratch Buffer Pic Width in MB
    uint8_t                         u8FirstFieldIdxList[CODECHAL_DECODE_AVC_MAX_NUM_MVC_VIEWS]; //!< First Field Index List
    uint32_t                        u32RefSurfaceNum;                                   //!< Number of reference frame surface

    bool                            bSecondField;                                       //!< Indicate it is second field
    bool                            bIntelProprietaryFormatInUse;                       //!< Indicate it is Intel Proprietary Format
    bool                            bShortFormatInUse;                                  //!< Indicate it is Short Format
    bool                            bPicIdRemappingInUse;                               //!< Indicate PicId Remapping are in use
    bool                            bDeblockingEnabled;                                 //!< Indicate Deblocking is enabled

#ifdef _DECODE_PROCESSING_SUPPORTED
    CODECHAL_AVC_SFC_STATE          SfcState;                                           //!< Avc Sfc State
#endif

    CODEC_PICTURE                   CurrPic;                                            //!< Current Picture Struct
    CODECHAL_AVC_FRAME_STORE_ID     avcFrameStoreID[CODEC_AVC_MAX_NUM_REF_FRAME];       //!< Avc Frame Store ID
    CODEC_AVC_DMV_LIST              avcDmvList[CODEC_AVC_NUM_DMV_BUFFERS];              //!< Avc Dmv List
    CODEC_PIC_ID                    avcPicIdx[CODEC_AVC_MAX_NUM_REF_FRAME];             //!< Avc Pic Index

    PCODEC_REF_LIST                 pAvcRefList[CODECHAL_AVC_NUM_UNCOMPRESSED_SURFACE]; //!< Pointer to AVC Ref List
    PCODEC_AVC_PIC_PARAMS           pAvcPicParams;                                      //!< Pointer to AVC picture parameter
    PCODEC_MVC_EXT_PIC_PARAMS       pMvcExtPicParams;                                   //!< Pointer to MVC ext picture parameter
    PCODEC_AVC_SLICE_PARAMS         pAvcSliceParams;                                    //!< Pointer to AVC slice parameter
    PCODECHAL_AVC_IQ_MATRIX_PARAMS  pAvcIQMatrixParams;                                 //!< Pointer to AVC IQ matrix parameter
    PCODECHAL_VLD_SLICE_RECORD      pVldSliceRecord;

    MOS_RESOURCE                    resDataBuffer;                                      //!< Handle of Data Buffer
    MOS_RESOURCE                    resMonoPictureChromaBuffer;                         //!< Handle of MonoPicture's default Chroma data surface
    MOS_RESOURCE                    resMfdIntraRowStoreScratchBuffer;                   //!< Handle of MFD Intra Row Store Scratch data surface
    MOS_RESOURCE                    resMfdDeblockingFilterRowStoreScratchBuffer;        //!< Handle of MFD Deblocking Filter Row Store Scratch data surface
    MOS_RESOURCE                    resBsdMpcRowStoreScratchBuffer;                     //!< Handle of BSD/MPC Row Store Scratch data surface
    MOS_RESOURCE                    resMprRowStoreScratchBuffer;                        //!< Handle of MPR Row Store Scratch data surface
    MOS_RESOURCE                    resAvcDmvBuffers[CODEC_AVC_NUM_DMV_BUFFERS];        //!< Handle of Dmv Buffers
    MOS_RESOURCE                    resInvalidRefBuffer;                                //!< Handle of Invalid Ref Buffer
    MOS_RESOURCE                    resMvcDummyDmvBuffer[2];                            //!< Handle of Mvc Dummy Dmv Buffer
    MOS_SURFACE                     sDestSurface;                                       //!< Handle of Dest data surface
    PMOS_SURFACE                    pRefFrameSurface;                                   //!< Handle of reference frame surface
    PMOS_RESOURCE                   presReferences[CODEC_AVC_MAX_NUM_REF_FRAME];        //!< Pointer to Handle of Reference Frames
    MOS_RESOURCE                    resSyncObjectWaContextInUse;                        //!< signals on the video WA context
    MOS_RESOURCE                    resSyncObjectVideoContextInUse;                     //!< signals on the video context
};
#endif  // __CODECHAL_DECODER_AVC_H__
