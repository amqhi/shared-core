#include "java_list.h"

#include "java_string.h"

std::vector<std::string> jstring_array_to_vector(JNIEnv *env, jobjectArray stringArray) {
    jsize length = env->GetArrayLength(stringArray);
    std::vector<std::string> result;
    result.reserve(length);

    for (jsize i = 0; i < length; ++i) {
        jstring jStr = (jstring)env->GetObjectArrayElement(stringArray, i);
        if (jStr) {
            const char *cStr = env->GetStringUTFChars(jStr, nullptr);
            result.emplace_back(cStr);
            env->ReleaseStringUTFChars(jStr, cStr);
            env->DeleteLocalRef(jStr);
        }
    }
    return result;
}
