/**
 * Copyright 2019 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include "debug.h"

#include "mcc_output.h"
#include "output_utils.h"
#include "cc_utils.h"

/*----------------------------------------------------------------------------*/
/*--                       Public Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                      Private Member Variables                          --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                     Private Member Declarations                        --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                       Public Member Functions                          --*/
/*----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 | NAME:
 |    MccOutInitialize()
 |
 | INPUT PARAMETERS:
 |    ctxPtr - Pointer to all Pipeline Elements Contexts, including this one.
 |
 | RETURN VALUES:
 |    LinkInfo -  Information about this element of the pipeline, such that it can
 |                be chained to elements that can consume the specific type of data
 |                that it produces.
 |
 | DESCRIPTION:
 |    This initializes this element of the pipeline.
 -------------------------------------------------------------------------------*/
LinkInfo MccOutInitialize( Context* ctxPtr ) {
    ASSERT(ctxPtr);
    ASSERT(!ctxPtr->mccOutputCtxPtr);
    char tempFilename[MAX_FILE_NAME_LEN];

    ctxPtr->mccOutputCtxPtr = malloc(sizeof(MccOutputCtx));
    ctxPtr->mccOutputCtxPtr->fp = NULL;

    buildOutputPath(ctxPtr->config.inputFilename, ctxPtr->config.outputDirectory, "mcc",
                    ctxPtr->mccOutputCtxPtr->mccFileName);

    LinkInfo linkInfo;
    linkInfo.linkType = MCC_DATA___TEXT_FILE;
    linkInfo.sourceType = DATA_TYPE_MCC_DATA;
    linkInfo.sinkType = DATA_TYPE_MCC_DATA_TXT_FILE;
    linkInfo.NextBufferFnPtr = &MccOutProcNextBuffer;
    linkInfo.ShutdownFnPtr = &MccOutShutdown;
    return linkInfo;
}  // MccOutInitialize()

/*------------------------------------------------------------------------------
 | NAME:
 |    MccOutProcNextBuffer()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Context Pointer.
 |    inBuffer - Pointer to the buffer to process.
 |
 | RETURN VALUES:
 |    uint8 - Success is TRUE / PIPELINE_SUCCESS, Failure is FALSE / PIPELINE_FAILURE
 |            All other codes specified in header.
 |
 | DESCRIPTION:
 |    This method processes an incoming buffer, writing the contents into an
 |    ASCII file.
 -------------------------------------------------------------------------------*/
uint8 MccOutProcNextBuffer( void* rootCtxPtr, Buffer* buffPtr ) {
    ASSERT(buffPtr);
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->mccOutputCtxPtr);

    if( ((Context*)rootCtxPtr)->mccOutputCtxPtr->fp == NULL ) {
        LOG(DEBUG_LEVEL_INFO, DBG_FILE_OUT, "Creating new MCC File for Output: %s", ((Context*)rootCtxPtr)->mccOutputCtxPtr->mccFileName);
        ((Context*)rootCtxPtr)->mccOutputCtxPtr->fp = fileOutputInit(((Context*)rootCtxPtr)->mccOutputCtxPtr->mccFileName);
    }

    writeToFile(((Context*)rootCtxPtr)->mccOutputCtxPtr->fp, "%s\n", buffPtr->dataPtr);
    FreeBuffer(buffPtr);
    return PIPELINE_SUCCESS;
} // MccOutProcNextBuffer()

/*------------------------------------------------------------------------------
 | NAME:
 |    MccOutShutdown()
 |
 | INPUT PARAMETERS:
 |    rootCtxPtr - Context Pointer.
 |
 | RETURN VALUES:
 |    uint8 - Success is TRUE / PIPELINE_SUCCESS, Failure is FALSE / PIPELINE_FAILURE
 |            All other codes specified in header.
 |
 | DESCRIPTION:
 |    This method is called when the previous element in the pipeline determines
 |    that there is no more data coming down the pipline. This element will
 |    perform any necessary actions as a result and pass this call down the
 |    pipeline.
 -------------------------------------------------------------------------------*/
uint8 MccOutShutdown( void* rootCtxPtr ) {
    ASSERT(rootCtxPtr);
    ASSERT(((Context*)rootCtxPtr)->mccOutputCtxPtr);

    if( ((Context*)rootCtxPtr)->mccOutputCtxPtr->fp != NULL ) {
        closeFile(((Context*)rootCtxPtr)->mccOutputCtxPtr->fp);
    } else {
        LOG(DEBUG_LEVEL_ERROR, DBG_FILE_OUT, "Nothing to place in MCC File. File not created!");
    }

    free(((Context*)rootCtxPtr)->mccOutputCtxPtr);
    ((Context*)rootCtxPtr)->mccOutputCtxPtr = NULL;
    return PIPELINE_SUCCESS;
} // MccOutShutdown()

/*----------------------------------------------------------------------------*/
/*--                       Private Member Functions                         --*/
/*----------------------------------------------------------------------------*/
