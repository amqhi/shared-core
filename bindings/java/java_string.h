#ifndef SHARED_JAVA_STRING_H
#define SHARED_JAVA_STRING_H

#include <jni.h>
#include <string>

struct JString {
    JNIEnv* env;
    jstring value;

    JString(JNIEnv* e, const std::string& s) : env(e) {
        value = env->NewStringUTF(s.c_str());
    }

    ~JString() {
        if (value) env->DeleteLocalRef(value);
    }

    JString(const JString&) = delete;
    JString& operator=(const JString&) = delete;
};

inline std::string jstring_to_string(JNIEnv* env, jstring jStr) {
    if (!env || !jStr) {
        return "";
    }

    const char* chars = env->GetStringUTFChars(jStr, nullptr);
    if (!chars) {
        return "";
    }

    std::string result(chars);
    env->ReleaseStringUTFChars(jStr, chars);
    return result;
}

#endif //SHARED_JAVA_STRING_H
