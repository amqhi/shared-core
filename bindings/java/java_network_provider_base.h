#ifndef SHARED_JAVA_NETWORK_PROVIDER_BASE_H
#define SHARED_JAVA_NETWORK_PROVIDER_BASE_H

#include "network_provider_base.h"
#include <jni.h>

#include "java_string.h"
#include "java_map.h"

class JavaNetworkProviderBase : public INetworkProviderBase {
private:
    JavaVM *jvm_{};
    jobject java_provider_global_;

    [[nodiscard]] JNIEnv *getEnv() const {
        JNIEnv *env;
        if (jvm_->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) == JNI_EDETACHED) {
            jvm_->AttachCurrentThread(&env, nullptr);
        }
        return env;
    }

    struct CallbackContext {
        std::function<void(int, const std::string &)> response;
        std::function<void(int, const std::string &)> failure;
    };

    struct ObjectStorageCallbackContext {
        std::function<void(int64_t, int64_t)> progress;
        std::function<void(int, const std::string &)> response;
        std::function<void(int, const std::string &)> failure;
    };

public:
    JavaNetworkProviderBase(JNIEnv *env, jobject java_provider) {
        env->GetJavaVM(&jvm_);
        java_provider_global_ = env->NewGlobalRef(java_provider);
    }

    ~JavaNetworkProviderBase() override {
        JNIEnv *env = getEnv();
        if (env && java_provider_global_) {
            env->DeleteGlobalRef(java_provider_global_);
        }
    }

    void executeHttp(const char* methodName, const std::string &url, const std::map<std::string, std::string> &headers,
                  const nlohmann::json &body, const std::function<void(int status_code,
                                                                 const std::string &response)>& on_response,    std::function<void(int error_code, const std::string& data)> on_failure) const
    {
        JNIEnv *env = getEnv();
        jclass clazz = env->GetObjectClass(java_provider_global_);

        // TODO: Allow configurable package path via arguments
        jmethodID methodId = env->GetMethodID(clazz, methodName,
                                              "(Ljava/lang/String;Ljava/util/Map;Ljava/lang/String;Lcom/amqhi/cloud/core/NetworkCallback;)V");

        jstring jurl = env->NewStringUTF(url.c_str());
        JMap jHeaders(env, headers);
        std::string bodyString = body.dump();
        JString jBody(env, bodyString);

        // This struct is freed when the Android callback executes, so you don't have to free it.
        auto* context = new CallbackContext{on_response, on_failure};

        // TODO: Allow configurable package path via arguments
        jclass cbProxyClass = env->FindClass("com/amqhi/cloud/core/NativeNetworkCallbackProxy");
        jmethodID cbInit = env->GetMethodID(cbProxyClass, "<init>", "(J)V");
        jobject jcallback = env->NewObject(cbProxyClass, cbInit, reinterpret_cast<jlong>(context));

        env->CallVoidMethod(java_provider_global_, methodId, jurl, jHeaders.value, jBody.value, jcallback);

        env->DeleteLocalRef(jurl);
        env->DeleteLocalRef(jcallback);
    }

    void executeHttp(const char* methodName, const std::string &url, const std::map<std::string, std::string> &headers, const std::function<void(int status_code,
                                                                const std::string &response)>& on_response, std::function<void(int error_code, const std::string& data)> on_failure) const
    {
        JNIEnv *env = getEnv();
        jclass clazz = env->GetObjectClass(java_provider_global_);

        // TODO: Allow configurable package path via arguments
        jmethodID methodId = env->GetMethodID(clazz, methodName,
                                              "(Ljava/lang/String;Ljava/util/Map;Lcom/amqhi/cloud/core/NetworkCallback;)V");

        jstring jurl = env->NewStringUTF(url.c_str());
        JMap jHeaders(env, headers);

        //This struct is freed when the Android callback executes, so you don't have to free it.
        auto* context = new CallbackContext{on_response, on_failure};

        // TODO: Allow configurable package path via arguments
        jclass cbProxyClass = env->FindClass("com/amqhi/cloud/core/NativeNetworkCallbackProxy");
        jmethodID cbInit = env->GetMethodID(cbProxyClass, "<init>", "(J)V");
        jobject jcallback = env->NewObject(cbProxyClass, cbInit, reinterpret_cast<jlong>(context));

        env->CallVoidMethod(java_provider_global_, methodId, jurl, jHeaders.value, jcallback);

        env->DeleteLocalRef(jurl);
        env->DeleteLocalRef(jcallback);
    }

    void post_json(
            const std::string &url,
            const std::map<std::string, std::string> &headers,
            const nlohmann::json &body,
            std::function<void(int status_code, const std::string &response)> on_response,
            std::function<void(int error_code, const std::string& data)> on_failure
    ) override {
        executeHttp("postJson", url, headers, body, on_response, on_failure);
    }

    void get(const std::string &url, const std::map<std::string, std::string> &headers,
             std::function<void(int status_code,
                                const std::string &response)> on_response,
                                   std::function<void(int error_code, const std::string& data)> on_failure) override
    {
        executeHttp("get", url, headers, on_response, on_failure);
    }

    void patch_json(const std::string &url, const std::map<std::string, std::string> &headers,
                    const nlohmann::json &body, std::function<void(int status_code,
                                                                   const std::string &response)> on_response,
                                                                      std::function<void(int error_code, const std::string& data)> on_failure) override
    {
        executeHttp("patchJson", url, headers, body, on_response, on_failure);
    }

    void destroy(const std::string &url, const std::map<std::string, std::string> &headers,
                 std::function<void(int status_code,
                                    const std::string &response)> on_response, std::function<void(int error_code, const std::string& data)> on_failure) override
    {
        executeHttp("delete", url, headers, on_response, on_failure);
    }

    void put_file(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& file_path, std::function<void(int64_t bytes_written, int64_t total_bytes)> on_progress, std::function<void(int status_code, const std::string& response)> on_response, std::function<void(int error_code, const std::string& data)> on_failure) override
    {
        JNIEnv *env = getEnv();
        jclass clazz = env->GetObjectClass(java_provider_global_);

        // TODO: Allow configurable package path via arguments
        jmethodID methodId = env->GetMethodID(clazz, "putFile",
                                              "(Ljava/lang/String;Ljava/util/Map;Ljava/lang/String;Lcom/amqhi/cloud/core/ObjectStorageCallback;)V");
        jstring jurl = env->NewStringUTF(url.c_str());
        JMap jHeaders(env, headers);
        jstring jfile_path = env->NewStringUTF(file_path.c_str());

        //This struct is freed when the Android callback executes, so you don't have to free it.
        auto* context = new ObjectStorageCallbackContext{on_progress, on_response, on_failure};

        // TODO: Allow configurable package path via arguments
        jclass cbProxyClass = env->FindClass("com/amqhi/cloud/core/ObjectStorageCallbackProxy");
        jmethodID cbInit = env->GetMethodID(cbProxyClass, "<init>", "(J)V");
        jobject jcallback = env->NewObject(cbProxyClass, cbInit, reinterpret_cast<jlong>(context));

        env->CallVoidMethod(java_provider_global_, methodId, jurl, jHeaders.value, jfile_path, jcallback);

        env->DeleteLocalRef(jurl);
        env->DeleteLocalRef(jfile_path);
        env->DeleteLocalRef(jcallback);
    }

    void put_file(const nlohmann::json& parts, const std::string& mime_type, const std::string& file_path, std::function<void(int64_t bytes_written, int64_t total_bytes)> on_progress, std::function<void(int status_code, const std::string& response)> on_response, std::function<void(int error_code, const std::string& data)> on_failure) override
    {
        JNIEnv *env = getEnv();
        jclass clazz = env->GetObjectClass(java_provider_global_);

        // TODO: Allow configurable package path via arguments
        jmethodID methodId = env->GetMethodID(clazz, "putMultipartFile",
                                              "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Lcom/amqhi/cloud/core/ObjectStorageCallback;)V");

        std::string parts_string = parts.dump();
        JString jParts(env, parts_string);
        JString jMimeType(env, mime_type);
        jstring jfile_path = env->NewStringUTF(file_path.c_str());

        //This struct is freed when the Android callback executes, so you don't have to free it.
        auto* context = new ObjectStorageCallbackContext{on_progress, on_response, on_failure};

        // TODO: Allow configurable package path via arguments
        jclass cbProxyClass = env->FindClass("com/amqhi/cloud/core/ObjectStorageCallbackProxy");
        jmethodID cbInit = env->GetMethodID(cbProxyClass, "<init>", "(J)V");
        jobject jcallback = env->NewObject(cbProxyClass, cbInit, reinterpret_cast<jlong>(context));

        env->CallVoidMethod(java_provider_global_, methodId, jParts.value, jMimeType.value, jfile_path, jcallback);

        env->DeleteLocalRef(jfile_path);
        env->DeleteLocalRef(jcallback);
    }

    void download_file(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& file_path, std::function<void(int64_t bytes_received, int64_t total_bytes)> on_progress, std::function<void(int status_code, const std::string& response)> on_response, std::function<void(int error_code, const std::string& data)> on_failure) override
    {
        JNIEnv *env = getEnv();
        jclass clazz = env->GetObjectClass(java_provider_global_);

        // TODO: Allow configurable package path via arguments
        jmethodID methodId = env->GetMethodID(clazz, "downloadFile",
                                              "(Ljava/lang/String;Ljava/util/Map;Ljava/lang/String;Lcom/amqhi/cloud/core/ObjectStorageCallback;)V");
        JString jUrl(env, url);
        JMap jHeaders(env, headers);
        JString jFilePath(env, file_path);

        // This struct is freed when the Android callback executes, so you don't have to free it.
        auto* context = new ObjectStorageCallbackContext{on_progress, on_response, on_failure};

        // TODO: Allow configurable package path via arguments
        jclass cbProxyClass = env->FindClass("com/amqhi/cloud/core/ObjectStorageCallbackProxy");
        jmethodID cbInit = env->GetMethodID(cbProxyClass, "<init>", "(J)V");
        jobject jCallback = env->NewObject(cbProxyClass, cbInit, reinterpret_cast<jlong>(context));

        env->CallVoidMethod(java_provider_global_, methodId, jUrl.value, jHeaders.value, jFilePath.value, jCallback);
        env->DeleteLocalRef(jCallback);
    }
};

#endif //SHARED_JAVA_NETWORK_PROVIDER_BASE_H
