package com.tencent.mars.sample.wrapper.remote;

import com.google.protobuf.nano.CodedOutputByteBufferNano;
import com.google.protobuf.nano.MessageNano;

import com.tencent.mars.sample.utils.print.MemoryDump;
import com.tencent.mars.stn.StnLogic;
import com.tencent.mars.xlog.Log;

/**
 * MarsTaskWrapper using nano protocol buffer encoding
 * <p></p>
 * Created by kirozhao on 16/2/29.
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
