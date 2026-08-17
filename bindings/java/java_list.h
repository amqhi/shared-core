#ifndef SHARED_JAVA_LIST_H
#define SHARED_JAVA_LIST_H

#include <vector>
#include <jni.h>

std::vector<std::string> jstring_array_to_vector(JNIEnv *env, jobjectArray stringArray);

#endif //SHARED_JAVA_LIST_H
