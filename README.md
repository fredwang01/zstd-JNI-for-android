"# zstd-JNI-for-android" 

ZSTD is a new compress library developed by facebook, it's big advantage is much faster than deflate compress algorithm used in zlib, moreover it's compress ratio is also better than deflate.
About the details of ZSTD, please visit the github project [here](https://github.com/facebook/zstd).

I writed a JNI wrapper, and compiled the ZSTD library for android platform. In the ndk directory, the sub-directories of `common`, `compress`,
`decompress` and `dictBuilder` are all ZSTD library code, you can use the latest version to replace them.

By the way, in early release of ZSTD, the API for callers is a little bit confusing, caller must check the inner status of the decompress API, that is unreasonable.
New code of the ZSTD already fixed this issue and the API using is easy and clear. About this issue , I post a discussion [here](https://github.com/facebook/zstd/issues/374).

