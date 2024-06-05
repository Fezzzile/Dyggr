# Dyggr 

Recover deleted files.

The word *Dyggr* is pronounced as *Digger*. 

Currently, the program recovers deleted or lost files by carving,
that is, searching for file signatures in a given file or device.
 
I started with formats that use RIFF (WAV and WebP).
I will then slowly add file formats with more-complicated headers.

If the file is fragmented, that is, broken up and stored in non-contiguous blocks in the device, carving will not work; restoring fragmented files requires reading file system tables/headers, which is not in the short-term plan.

Carving will not work on an encrypted drive, unless it is decrypted first.

Make sure you save the restored files to a separate drive, not the original, to avoid overriding other deleted files.
