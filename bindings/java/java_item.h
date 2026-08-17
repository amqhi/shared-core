#ifndef SHARED_JAVA_ITEM_H
#define SHARED_JAVA_ITEM_H

#include <jni.h>
#include "java_string.h"
#include "item.h"

struct JItemSummary
{
    JNIEnv* env;
    jobject value;

    JItemSummary(JNIEnv* e, const ItemSummary& itemSummary) : env(e) {
        // TODO: Allow configurable package path via arguments
        jclass item_class = env->FindClass("com/amqhi/cloud/models/ItemSummary");
        init(item_class, itemSummary);
    }

    JItemSummary(JNIEnv* e, jclass summaryClass, const ItemSummary& itemSummary) : env(e) {
        init(summaryClass, itemSummary);
    }

    ~JItemSummary() {
        if (value) env->DeleteLocalRef(value);
    }

    JItemSummary(const JItemSummary&) = delete;
    JItemSummary& operator=(const JItemSummary&) = delete;

    operator jobject() const { return value; }

private:
    void init(jclass summaryClass, const ItemSummary& itemSummary)
    {
        jmethodID cbInit = env->GetMethodID(summaryClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
        JString jId(env, itemSummary.id);
        JString jType(env, itemSummary.type);
        JString jName(env, itemSummary.name);
        JString jThumbnailPath(env, itemSummary.thumbnail_path);

        value = env->NewObject(summaryClass, cbInit, jId.value, jType.value, jName.value, jThumbnailPath.value);
    }
};

#endif //SHARED_JAVA_ITEM_H
