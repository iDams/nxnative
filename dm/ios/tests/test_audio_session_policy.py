"""Regression tests for DM_AUDIO_MIX policy helpers in dm/ios/Sources/audio_session_policy.h."""
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

IOS_SOURCES = Path(__file__).resolve().parents[1] / "Sources"


@unittest.skipUnless(shutil.which("cc") or shutil.which("clang"), "needs a C compiler")
class AudioSessionPolicyTests(unittest.TestCase):
    def test_c_policy_values_env_and_option_clearing(self):
        compiler = shutil.which("clang") or shutil.which("cc")
        source = r'''
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "audio_session_policy.h"

int main(void) {
    assert(!dm_audio_mix_enabled(NULL));
    assert(!dm_audio_mix_enabled(""));
    assert(!dm_audio_mix_enabled("0"));
    assert(!dm_audio_mix_enabled("00"));
    assert(!dm_audio_mix_enabled("off"));
    assert(dm_audio_mix_enabled("1"));
    assert(dm_audio_mix_enabled("2"));
    assert(dm_audio_mix_enabled("-1"));

    assert(strcmp(dm_audio_mix_env_value(NULL), "0") == 0);
    assert(strcmp(dm_audio_mix_env_value("0"), "0") == 0);
    assert(strcmp(dm_audio_mix_env_value("1"), "1") == 0);
    assert(strcmp(dm_audio_mix_env_value("2"), "1") == 0);

    const unsigned long mix_bit = 0x1UL;
    assert(dm_audio_session_category_options("1", mix_bit) == mix_bit);
    assert(dm_audio_session_category_options("0", mix_bit) == 0UL);
    assert(dm_audio_session_category_options(NULL, mix_bit) == 0UL);

    unsetenv("DM_AUDIO_MIX");
    assert(!dm_audio_mix_from_env());
    assert(dm_audio_session_category_options(getenv("DM_AUDIO_MIX"), mix_bit) == 0UL);

    setenv("DM_AUDIO_MIX", "1", 1);
    assert(dm_audio_mix_from_env());
    assert(dm_audio_session_category_options(getenv("DM_AUDIO_MIX"), mix_bit) == mix_bit);

    setenv("DM_AUDIO_MIX", "0", 1);
    assert(!dm_audio_mix_from_env());
    assert(dm_audio_session_category_options(getenv("DM_AUDIO_MIX"), mix_bit) == 0UL);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            c_path = Path(tmp) / "test_policy.c"
            exe = Path(tmp) / "test_policy"
            c_path.write_text(source)
            subprocess.run(
                [compiler, "-Wall", "-Werror", "-I", str(IOS_SOURCES), str(c_path), "-o", str(exe)],
                check=True,
            )
            subprocess.run([str(exe)], check=True)

    @unittest.skipUnless(sys.platform == "darwin" and shutil.which("xcrun"), "needs macOS Xcode toolchain")
    def test_ios_objc_helpers_typecheck_against_avfaudio(self):
        sdk = subprocess.run(
            ["xcrun", "--sdk", "iphonesimulator", "--show-sdk-path"],
            capture_output=True,
            text=True,
        )
        if sdk.returncode != 0 or not sdk.stdout.strip():
            self.skipTest("iphonesimulator SDK not installed")
        sdk_path = sdk.stdout.strip()
        source = r'''
#import <AVFAudio/AVFAudio.h>
#include "audio_session_policy.h"

BOOL exercise_audio_policy(AVAudioSession *session, NSError **error) {
    AVAudioSessionCategoryOptions opts_on = dm_audio_session_options("1");
    AVAudioSessionCategoryOptions opts_off = dm_audio_session_options("0");
    AVAudioSessionCategoryOptions opts_env = dm_audio_session_options_from_env();
    (void)opts_env;
    if ((opts_on & AVAudioSessionCategoryOptionMixWithOthers) == 0) return NO;
    if (opts_off != 0) return NO;
    return dm_audio_session_apply(session, getenv("DM_AUDIO_MIX"), error);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            m_path = Path(tmp) / "test_objc_policy.m"
            obj_path = Path(tmp) / "test_objc_policy.o"
            m_path.write_text(source)
            subprocess.run(
                [
                    "xcrun", "--sdk", "iphonesimulator", "clang",
                    "-target", "arm64-apple-ios17.0-simulator",
                    "-isysroot", sdk_path,
                    "-fobjc-arc", "-Wall", "-Werror", "-c",
                    "-I", str(IOS_SOURCES),
                    str(m_path), "-o", str(obj_path),
                ],
                check=True,
            )


if __name__ == "__main__":
    unittest.main()
