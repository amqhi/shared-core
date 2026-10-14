
#ifndef SHARED_CORE_JAVA_UUID_H
#define SHARED_CORE_JAVA_UUID_H

#include <jni.h>

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

#endif //SHARED_CORE_JAVA_UUID_H
