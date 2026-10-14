
#ifndef SHARED_CORE_JAVA_USER_H
#define SHARED_CORE_JAVA_USER_H

#include <jni.h>
#include "user.h"
#include "java_string.h"

inline jobject user_to_juser(JNIEnv* env, jclass userClass, jmethodID methodId, const User& user) {
    jchar jLocalId = static_cast<jchar>(user.local_id); // NOLINT
    JString jUserName(env, user.name);
    JString jEmail(env, user.email);
    jshort jLoginType = static_cast<jshort>(user.login_type); // NOLINT
    jstring jCachedPicturePath = nullptr;
    if(!user.cached_picture_path.empty()) {
        jCachedPicturePath = env->NewStringUTF(user.cached_picture_path.c_str());
    }

    if (jCachedPicturePath != nullptr) {
        env->DeleteLocalRef(jCachedPicturePath);
    }
    return env->NewObject(userClass, methodId, jLocalId, jUserName.value, jEmail.value, jLoginType, jCachedPicturePath);
}

inline jobject user_to_juser(JNIEnv* env, const User& user)
{
    jclass userClass = env->FindClass("com/amqhi/cloud/core/User");
    jmethodID methodId = env->GetMethodID(userClass, "<init>", "(CLjava/lang/String;Ljava/lang/String;SLjava/lang/String;)V");
    return user_to_juser(env, userClass, methodId, user);
}

struct JUser
{
    JNIEnv* env;
    jobject value;

    JUser(JNIEnv* e, const User& user) : env(e) {
      jclass userClass = env->FindClass("com/amqhi/cloud/core/User");
        jmethodID methodId = env->GetMethodID(userClass, "<init>", "(CLjava/lang/String;Ljava/lang/String;SLjava/lang/String;)V");
        init(userClass, methodId, user);
    }
    
    JUser(JNIEnv* e, jclass userClass, jmethodID methodId, const User& user) : env(e) {
         init(userClass, methodId, user);
    }

    ~JUser() {
       // if (value) env->DeleteLocalRef(value);
    }

    JUser(const JUser&) = delete;
    JUser& operator=(const JUser&) = delete;

    operator jobject() const {
        return value;
    }
    
    private:
    void init(jclass userClass, jmethodID methodId, const User& user)
    {
        jchar jLocalId = static_cast<jchar>(user.local_id); // NOLINT
        JString jUserName(env, user.name);
        JString jEmail(env, user.email);
        jshort jLoginType = static_cast<jshort>(user.login_type); // NOLINT
        jstring jCachedPicturePath = nullptr;
        if(!user.cached_picture_path.empty()) {
            jCachedPicturePath = env->NewStringUTF(user.cached_picture_path.c_str());
        }

        value = env->NewObject(userClass, methodId, jLocalId, jUserName.value, jEmail.value, jLoginType, jCachedPicturePath);
        if (jCachedPicturePath != nullptr) {
            env->DeleteLocalRef(jCachedPicturePath);
        }
    }
};

#endif //SHARED_CORE_JAVA_USER_H
