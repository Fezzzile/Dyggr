# buysla

Restore deleted files.

The word *buysla* is from the Nguni word *buyisela*, meaning "restore."

I promise, it is not a short for *buy a Tesla*, although it doesn't hurt to use this phrase as a mnemonic.

Currently, the program restores deleted or lost files by carving,
that is, searching for file signatures in a given file or device.
 
I am going to start with file formats that use the RIFF header, such as WAV and WebP.

I chose RIFF because of its simplicity.

I will then slowly add file formats with more-complicated headers.

If the file is fragmented, that is, broken up and stored in non-consecutive addresses in the device, carving will not work.

Restoring fragmented files requires reading file system tables/headers, which is not in the short-term plan.

Make sure you save the restored files to a separate drive, not the original, to avoid overriding other deleted files.
