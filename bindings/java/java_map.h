#ifndef SHARED_JAVA_MAP_H
#define SHARED_JAVA_MAP_H

#include <jni.h>
#include <string>
#include <map>

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

#endif //SHARED_JAVA_MAP_H
