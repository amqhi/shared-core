
#ifndef SHARED_CORE_JAVA_UUID_H
#define SHARED_CORE_JAVA_UUID_H

#include <jni.h>
#include "uuid.h"
#include <vector>

struct JUUID
{
    JNIEnv* env;
    jobject value;

    JUUID(JNIEnv* e, std::uint64_t high, std::uint64_t low) : env(e) {
        jclass clazz = env->FindClass("java/util/UUID");
        jmethodID uuidInit = env->GetMethodID(clazz, "<init>", "(JJ)V");
        value = env->NewObject(clazz, uuidInit, static_cast<jlong>(high), static_cast<jlong>(low));
        env->DeleteLocalRef(clazz);
    }

    ~JUUID() {
        if (value) env->DeleteLocalRef(value);
    }

    JUUID(const JUUID&) = delete;
    JUUID& operator=(const JUUID&) = delete;

    operator jobject() const {
        return value;
    }
};

namespace java_uuid
{
    inline std::vector<UUID> vector_from_jlongArray(JNIEnv* env, jlongArray ids)
    {
        jsize length = env->GetArrayLength(ids);
        jlong* elements = env->GetLongArrayElements(ids, nullptr);
        if (elements != nullptr) {
            std::vector<UUID> result;
            for (jsize i = 0; i + 1 < length; i += 2) {
                UUID id{};
                id.high = static_cast<std::uint64_t>(elements[i]);
                id.low = static_cast<std::uint64_t>(elements[i + 1]);
                result.push_back(id);
            }
            env->ReleaseLongArrayElements(ids, elements, JNI_ABORT);
            return result;
        }
        return {};
    }
}

#endif //SHARED_CORE_JAVA_UUID_H
