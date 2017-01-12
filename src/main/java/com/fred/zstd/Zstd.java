package com.fred.zstd;


public class Zstd {
    private static final String TAG = "Zstd";
    public static final int DEFAULT_LEVEL = 1;
    private static final int OP_NONE = 0;
    private static final int OP_COMPRESS = 1;
    private static final int OP_DECOMPRESS = 2;
    private long instance;
    private int opCode ;
    private int errCode;

    static {
        System.loadLibrary("zstd");
    }

    private Zstd() {
    }

    public static Zstd getInstance() {
        return new Zstd();
    }

    private native long nt1(int level);
    public int cinit() {
        return cinit(DEFAULT_LEVEL);
    }

    private int cinit(int level) {
        instance = nt1(level);
        if (instance == 0) {
            return -1;
        }
        opCode = OP_COMPRESS;
        return 0;
    }

    private native long nt3();
    public int dinit() {
        instance = nt3();
        if (instance == 0) {
            return -1;
        }
        opCode = OP_DECOMPRESS;
        return 0;
    }

    private native byte[] nt2(long instance, byte[] data, int offset, int size, int isEnd);
    private native byte[] nt4(long instance, byte[] data, int offset, int size, int isEnd);

    public byte[] update(byte[] data, int offset, int length) {
        if (data == null) {
            throw new IllegalArgumentException("data is null.");
        }
        if (length <= 0) {
            throw new IllegalArgumentException("length is zero.");
        }
        if (offset < 0) {
            throw new IllegalArgumentException("offset is less than zero.");
        }
        if (length > data.length) {
            throw new IllegalArgumentException("length is more than data.length.");
        }
        if ((offset + length) > data.length) {
            throw new IllegalArgumentException("offset+length equal or more than data.length.");
        }
        if (opCode == OP_NONE) {
            throw new IllegalStateException("please init first!!!");
        }

        byte[] bytes = null;
        if (opCode == OP_COMPRESS) {
            bytes = nt2(instance, data, offset, offset + length, 0);
        } else {
            bytes = nt4(instance, data, offset, offset + length, 0);
        }
        if (bytes == null) {
            String errMsg = String.format("%s error:%d\n", (opCode == OP_COMPRESS ? "compress" : "decompress"), errCode);
            throw new RuntimeException(errMsg);
        }
        return bytes;
    }

    public byte[] update(byte[] data) {
        return update(data, 0, data.length);
    }

    public byte[] end(byte[] data, int offset, int length) {
        if (data != null) {
            if (length <= 0) {
                throw new IllegalArgumentException("length is zero.");
            }
            if (offset < 0) {
                throw new IllegalArgumentException("offset is less than zero.");
            }
            if (length > data.length) {
                throw new IllegalArgumentException("length is more than data.length.");
            }
            if ((offset + length) > data.length) {
                throw new IllegalArgumentException("offset+length equal or more than data.length.");
            }
        } else {
            if (offset != 0) {
                throw new IllegalArgumentException("offset not qual zero.");
            }
            if (length != 0) {
                throw new IllegalArgumentException("length not qual zero.");
            }
        }
        if (opCode == OP_NONE) {
            throw new IllegalStateException("please init first.");
        }
        byte[] bytes = null;
        if (opCode == OP_COMPRESS) {
            bytes = nt2(instance, data, offset, offset + length, 1);
        } else {
            bytes = nt4(instance, data, offset, offset + length, 1);
        }
        if (bytes == null) {
            String errMsg = String.format("%s error:%d\n", (opCode == OP_COMPRESS ? "compress" : "decompress"), errCode);
            throw new RuntimeException(errMsg);
        }
        opCode = OP_NONE;
        return bytes;
    }

    public byte[] end(byte[] data) {
        return end(data, 0, data.length);
    }

    public byte[] end() {
        return end(null, 0, 0);
    }
}
