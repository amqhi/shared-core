#ifndef SHARED_CORE_JAVA_INTEGER_H
#define SHARED_CORE_JAVA_INTEGER_H

#include <jni.h>
#include <optional>

struct JInteger
{
    JNIEnv* env;
    jobject value;

    JInteger(JNIEnv* e, jint intValue) : env(e)
    {
        jclass clazz = env->FindClass("java/lang/Integer");
        jmethodID methodId = env->GetMethodID(clazz, "<init>", "(I)V");
        value = env->NewObject(clazz, methodId, intValue);
    }

    ~JInteger()
    {
        if (value) env->DeleteLocalRef(value);
    }

    JInteger(const JInteger&) = delete;
    JInteger& operator=(const JInteger&) = delete;
};

struct JNullableInteger
{
    JNIEnv* env;
    jobject value;

    JNullableInteger(JNIEnv* e, std::optional<int> intValue) : env(e)
    {
        if (intValue.has_value())
        {
            jclass clazz = env->FindClass("java/lang/Integer");
            jmethodID methodId = env->GetMethodID(clazz, "<init>", "(I)V");
            value = env->NewObject(clazz, methodId, intValue.value());
        }
        else
        {
            value = nullptr;
        }
    }

    ~JNullableInteger()
    {
        if (value) env->DeleteLocalRef(value);
    }

    JNullableInteger(const JNullableInteger&) = delete;
    JNullableInteger& operator=(const JNullableInteger&) = delete;
};

#endif //SHARED_CORE_JAVA_INTEGER_H
