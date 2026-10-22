#ifndef SHARED_JAVA_MAP_H
#define SHARED_JAVA_MAP_H

#include <jni.h>
#include <string>
#include <map>
#include <unordered_map>
#include "java_string.h"

// Note: This does not convert complex data types like JSON arrays or nested objects.
// If you need to handle complex structures, use JSON strings instead.

struct JMap
{
    JNIEnv* env;
    jobject value;

    JMap(JNIEnv* e, const std::map<std::string, std::string>& cpp_map) : env(e) {
        jclass mapClass = env->FindClass("java/util/HashMap");
        jmethodID init = env->GetMethodID(mapClass, "<init>", "()V");
        value = env->NewObject(mapClass, init);
        jmethodID put = env->GetMethodID(mapClass, "put",
                                         "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

        for (const auto& [key, val] : cpp_map)
        {
            jstring jkey = env->NewStringUTF(key.c_str());
            jstring jval = env->NewStringUTF(val.c_str());
            env->CallObjectMethod(value, put, jkey, jval);
            env->DeleteLocalRef(jkey);
            env->DeleteLocalRef(jval);
        }
    }

    ~JMap() {
        if (value) env->DeleteLocalRef(value);
    }

    JMap(const JMap&) = delete;
    JMap& operator=(const JMap&) = delete;
};

namespace java_map
{
    inline std::unordered_map<std::string, std::string> jobject_to_unordered_map(JNIEnv *env, jobject jmap) {
        std::unordered_map<std::string, std::string> cpp_map;
        if (!jmap) return cpp_map;

        jclass mapClass = env->GetObjectClass(jmap);
        jmethodID entrySetMethod = env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
        jobject entrySet = env->CallObjectMethod(jmap, entrySetMethod);

        jclass setClass = env->FindClass("java/util/Set");
        jmethodID iteratorMethod = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
        jobject iterator = env->CallObjectMethod(entrySet, iteratorMethod);

        jclass iteratorClass = env->FindClass("java/util/Iterator");
        jmethodID hasNextMethod = env->GetMethodID(iteratorClass, "hasNext", "()Z");
        jmethodID nextMethod = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");

        jclass entryClass = env->FindClass("java/util/Map$Entry");
        jmethodID getKeyMethod = env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
        jmethodID getValueMethod = env->GetMethodID(entryClass, "getValue", "()Ljava/lang/Object;");

        while (env->CallBooleanMethod(iterator, hasNextMethod)) {
            jobject entry = env->CallObjectMethod(iterator, nextMethod);

            auto jkey = (jstring) env->CallObjectMethod(entry, getKeyMethod);
            auto jval = (jstring) env->CallObjectMethod(entry, getValueMethod);

            cpp_map[jstring_to_string(env, jkey)] = jstring_to_string(env, jval);

            env->DeleteLocalRef(entry);
        }

        return cpp_map;
    }
}

#endif //SHARED_JAVA_MAP_H
