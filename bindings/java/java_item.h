#ifndef SHARED_JAVA_ITEM_H
#define SHARED_JAVA_ITEM_H

#include <jni.h>
#include "java_string.h"
#include "item.h"
#include "java_uuid.h"
#include <string>

struct JItemSummary
{
    JNIEnv* env;
    jobject value;

    JItemSummary(JNIEnv* e, const Item& item, const std::string& app_support_path, char user_local_id) : env(e) { // NOLINT
        // TODO: Allow configurable package path via arguments
        jclass item_class = env->FindClass("com/amqhi/shared/core/ItemSummary");
        init(item_class, item, app_support_path, user_local_id);
    }

    JItemSummary(JNIEnv* e, jclass summaryClass, const Item& item, const std::string& app_support_path, char user_local_id) : env(e) { // NOLINT
        init(summaryClass, item, app_support_path, user_local_id);
    }

    ~JItemSummary() {
        if (value) env->DeleteLocalRef(value);
    }

    JItemSummary(const JItemSummary&) = delete;
    JItemSummary& operator=(const JItemSummary&) = delete;

private:
    void init(jclass summaryClass, const Item& item, const std::string& app_support_path, char user_local_id)
    {
        jmethodID methodId = env->GetMethodID(summaryClass, "<init>", "(Ljava/util/UUID;CLjava/lang/String;Ljava/lang/String;CC)V");
        JUUID jId(env, item.id.high, item.id.low);
        jchar jType = static_cast<jchar>(item.type); // NOLINT
        JString jName(env, item.name);
        JString jThumbnailPath(env, item_thumbnail_path(app_support_path, user_local_id, item.id));
        jchar jIconType = static_cast<jchar>(item.icon_type); // NOLINT
        jchar jStatus = static_cast<jchar>(item.status); // NOLINT

        value = env->NewObject(summaryClass, methodId, jId.value, jType, jName.value, jThumbnailPath.value, jIconType, jStatus);
    }
};

inline ItemAttributes item_attributes_from_jobject(JNIEnv* env, jobject jItemAttributes)
{
    ItemAttributes attributes;
    if (jItemAttributes != nullptr)
    {
        jclass clazz = env->GetObjectClass(jItemAttributes);

        JNI_GET_STRING_FROM_CLASS(jItemAttributes, name, "name")
        attributes.name = jstring_to_string(env, name);
        env->DeleteLocalRef(name);

        JNI_GET_OBJECT_FROM_CLASS(jItemAttributes, jEventAt, "eventAt", "Ljava/lang/Long;")
        if (jEventAt != nullptr)
        {
            JNI_GET_LONG_FROM_CLASS(jEventAt, eventAt, "longValue")
            attributes.event_at = eventAt;
        }
        JNI_GET_LONG_FROM_CLASS(jItemAttributes, parentIdLow, "parentIdLow")
               JNI_GET_LONG_FROM_CLASS(jItemAttributes, parentIdHigh, "parentIdHigh")
               amqhi::UUID parentId{};
        parentId.high = parentIdHigh;
        parentId.low = parentIdLow;
        attributes.parent_id = parentId;

        JNI_GET_BOOLEAN_FROM_CLASS(jItemAttributes, encrypted, "encrypted")
        attributes.encrypted = encrypted;

        JNI_GET_OBJECT_FROM_CLASS(jItemAttributes, jComment, "comment", "Ljava/lang/String;")
        if (jComment != nullptr)
        {
            attributes.comment = jstring_to_string(env, reinterpret_cast<jstring>(jComment));
            env->DeleteLocalRef(jComment);
        }

        JNI_GET_SHORT_FROM_CLASS(jItemAttributes, appScope, "appScope")

        attributes.app_scope = appScope;

    }
    return attributes;
}

#endif //SHARED_JAVA_ITEM_H
