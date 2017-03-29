/*
* Tencent is pleased to support the open source community by making Mars available.
* Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the MIT License (the "License"); you may not use this file except in 
* compliance with the License. You may obtain a copy of the License at
* http://opensource.org/licenses/MIT
*
* Unless required by applicable law or agreed to in writing, software distributed under the License is
* distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
* either express or implied. See the License for the specific language governing permissions and
* limitations under the License.
*/

package com.tencent.mars.sample.wrapper.remote;

import com.google.protobuf.nano.CodedOutputByteBufferNano;
import com.google.protobuf.nano.MessageNano;
import com.tencent.mars.sample.utils.print.MemoryDump;
import com.tencent.mars.stn.StnLogic;
import com.tencent.mars.xlog.Log;

/**
 * MarsTaskWrapper using nano protocol buffer encoding
 * <p></p>
 * Created by zhaoyuan on 16/2/29.
 */
public abstract class NanoMarsTaskWrapper<T extends MessageNano, R extends MessageNano> extends AbstractTaskWrapper {

    private static final String TAG = "Mars.Sample.NanoMarsTaskWrapper";

    protected T request;
    protected R response;

    public NanoMarsTaskWrapper(T req, R resp) {
        super();

        this.request = req;
        this.response = resp;
    }

    @Override
    public byte[] req2buf() {
        try {
            onPreEncode(request);

            final byte[] flatArray = new byte[request.getSerializedSize()];
            final CodedOutputByteBufferNano output = CodedOutputByteBufferNano.newInstance(flatArray);
            request.writeTo(output);

            Log.d(TAG, "encoded request to buffer, [%s]", MemoryDump.dumpHex(flatArray));

            return flatArray;

        } catch (Exception e) {
            e.printStackTrace();
        }

        return new byte[0];
    }

    @Override
    public int buf2resp(byte[] buf) {
        try {
            Log.d(TAG, "decode response buffer, [%s]", MemoryDump.dumpHex(buf));

            response = MessageNano.mergeFrom(response, buf);
            onPostDecode(response);
            return StnLogic.RESP_FAIL_HANDLE_NORMAL;

        } catch (Exception e) {
            Log.e(TAG, "%s", e);
        }

        return StnLogic.RESP_FAIL_HANDLE_TASK_END;
    }

    public abstract void onPreEncode(T request);

    public abstract void onPostDecode(R response);
}
