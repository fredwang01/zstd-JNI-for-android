package com.fred.zstd;


import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.util.Arrays;

public class ZstdTest {
    private static final String TAG = "ZstdTest";

    public boolean test(byte[] original) {
        ByteArrayOutputStream compressedBytes = new ByteArrayOutputStream();
        ByteArrayOutputStream decompressedBytes = new ByteArrayOutputStream();

        Zstd zstd = Zstd.getInstance();
        // compress status
        int ret = zstd.cinit();
        if (ret != 0) {
            Log.e(TAG, "cinit error.");
            return false;
        }
        byte[] bytes = zstd.update(original);
        if (bytes.length > 0) {
            compressedBytes.write(bytes, 0, bytes.length);
        }
        bytes = zstd.end();
        if (bytes.length > 0) {
            compressedBytes.write(bytes, 0, bytes.length);
        }

        // uncompress status
        ret = zstd.dinit();
        if (ret != 0) {
            Log.e(TAG, "dinit error.");
            return false;
        }
        bytes = zstd.update(compressedBytes.toByteArray());
        if (bytes.length > 0) {
            decompressedBytes.write(bytes, 0, bytes.length);
        }
        bytes = zstd.end();
        if (bytes.length > 0) {
            decompressedBytes.write(bytes, 0, bytes.length);
        }

        // compare the original with the decompressed data. should be equal.
        return Arrays.equals(original, decompressedBytes.toByteArray());
    }
}
