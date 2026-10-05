/* SPDX-License-Identifier: MIT
 * Windows Platform Filesystem Test.
 */
#include "platform/fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

int main(void) {
    printf("[TEST] Starting Windows Platform Filesystem Test...\n");

    const char* test_file = "test_fs_temp.bin";
    const char* test_dir  = "test_fs_temp_dir";

    // 1. Create and Write File
    int64_t fd = bb_platform_file_open(test_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    assert(fd >= 0);
    printf("  [PASS] Created test file '%s' (fd=%lld)\n", test_file, (long long)fd);

    const char magic_data[] = "BLOODBORNE_WINDOWS_FS_VERIFICATION_PAYLOAD_1234567890";
    uint64_t data_len = sizeof(magic_data);
    int64_t written = bb_platform_file_write(fd, magic_data, data_len);
    assert(written == (int64_t)data_len);
    printf("  [PASS] Wrote %llu bytes to file\n", (unsigned long long)written);

    bb_platform_file_close(fd);

    // 2. Stat File
    uint64_t file_size = 0, mtime = 0;
    bool is_dir = false;
    int32_t stat_res = bb_platform_file_stat(test_file, &file_size, &mtime, &is_dir);
    assert(stat_res == 0 && file_size == data_len && !is_dir);
    printf("  [PASS] Verified stat: size=%llu bytes, is_dir=%d\n", (unsigned long long)file_size, is_dir);

    // 3. Read and verify content
    fd = bb_platform_file_open(test_file, O_RDONLY, 0);
    assert(fd >= 0);
    char read_buf[128] = {0};
    int64_t read_bytes = bb_platform_file_read(fd, read_buf, sizeof(read_buf));
    assert(read_bytes == (int64_t)data_len);
    assert(memcmp(read_buf, magic_data, data_len) == 0);
    printf("  [PASS] Read payload matches exactly: '%s'\n", read_buf);
    bb_platform_file_close(fd);

    // 4. Directory creation and enumeration
    bb_platform_mkdir(test_dir);
    BbDir* dir = bb_platform_dir_open(".");
    assert(dir != NULL);
    bool found_file = false, found_dir = false;
    BbDirEntry entry;
    while (bb_platform_dir_next(dir, &entry)) {
        if (strcmp(entry.name, test_file) == 0) found_file = true;
        if (strcmp(entry.name, test_dir) == 0)  found_dir  = true;
    }
    bb_platform_dir_close(dir);
    assert(found_file && found_dir);
    printf("  [PASS] Directory enumeration found created file and directory\n");

    // Clean up
    bb_platform_remove(test_file);
    bb_platform_remove(test_dir);
    printf("  [PASS] Cleaned up test files\n");

    printf("[SUCCESS] Windows Platform Filesystem test PASSED!\n");
    return 0;
}
