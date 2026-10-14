
#ifndef SHARED_CORE_JAVA_LONG_H
#define SHARED_CORE_JAVA_LONG_H

#include <jni.h>

struct JLong
{
    JNIEnv* env;
    jobject value;

    JLong(JNIEnv* e, jlong longValue) : env(e) {
        jclass clazz = env->FindClass("java/lang/Long");
        jmethodID methodId = env->GetMethodID(clazz, "<init>", "(J)V");
        value = env->NewObject(clazz, methodId, longValue);
    }

    ~JLong() {
        if (value) env->DeleteLocalRef(value);
    }

    JLong(const JLong&) = delete;
    JLong& operator=(const JLong&) = delete;
};

struct JNullableLong
{
    JNIEnv* env;
    jobject value;

    JNullableLong(JNIEnv* e, std::optional<long> longValue) : env(e) {
        if (longValue.has_value())
        {
            jclass clazz = env->FindClass("java/lang/Long");
            jmethodID methodId = env->GetMethodID(clazz, "<init>", "(J)V");
            value = env->NewObject(clazz, methodId, longValue);
        }
        else
        {
            value = nullptr;
        }
    }

    ~JNullableLong() {
        if (value) env->DeleteLocalRef(value);
    }

    JNullableLong(const JLong&) = delete;
    JNullableLong& operator=(const JLong&) = delete;
};

#endif //SHARED_CORE_JAVA_LONG_H
