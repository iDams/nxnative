// Audio session policy for DM_AUDIO_MIX (LNPANDY/NX2Apple#1269):
// Keep AVAudioSessionCategoryPlayback ignoring the hardware silent switch unchanged,
// and opt into mixing with other apps only when DM_AUDIO_MIX is truthy.
#pragma once
#include <stdbool.h>
#include <stdlib.h>

static inline bool dm_audio_mix_enabled(const char *value) {
    return value && atoi(value) != 0;
}

static inline bool dm_audio_mix_from_env(void) {
    return dm_audio_mix_enabled(getenv("DM_AUDIO_MIX"));
}

static inline const char *dm_audio_mix_env_value(const char *value) {
    return dm_audio_mix_enabled(value) ? "1" : "0";
}

static inline unsigned long dm_audio_session_category_options(const char *value, unsigned long mix_option_mask) {
    return dm_audio_mix_enabled(value) ? mix_option_mask : 0UL;
}

#if defined(__OBJC__) && defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#import <AVFAudio/AVFAudio.h>

static inline AVAudioSessionCategoryOptions dm_audio_session_options(const char *value) {
    return (AVAudioSessionCategoryOptions)dm_audio_session_category_options(value, (unsigned long)AVAudioSessionCategoryOptionMixWithOthers);
}

static inline AVAudioSessionCategoryOptions dm_audio_session_options_from_env(void) {
    return dm_audio_session_options(getenv("DM_AUDIO_MIX"));
}

static inline BOOL dm_audio_session_apply(AVAudioSession *session, const char *mix_value, NSError **out_error) {
    return [session setCategory:AVAudioSessionCategoryPlayback withOptions:dm_audio_session_options(mix_value) error:out_error];
}
#endif
#endif
