#ifndef GLOBAL_H // !GLOBAL_H
#define GLOBAL_H

#include <android/log.h>

#define TAG "AImGui"

#define LogInfo(formatter, ...) __android_log_print(ANDROID_LOG_INFO, TAG, formatter __VA_OPT__(, ) __VA_ARGS__)
#define LogDebug(formatter, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, formatter __VA_OPT__(, ) __VA_ARGS__)
#define LogError(formatter, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, formatter __VA_OPT__(, ) __VA_ARGS__)

#endif // !GLOBAL_H