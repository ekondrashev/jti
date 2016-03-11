//http://stackoverflow.com/questions/31001558/jvmti-heap-dump-on-exception-stacktrace-issues
#include <jni.h>
#include <jvmti.h>
#include <string.h>
#include <stdio.h>
#include "jmm.h"

JNIEXPORT void* JNICALL JVM_GetManagement(jint version);
jvmtiEnv* jvmti;

void JNICALL
Exception(jvmtiEnv *jvmti_env,
            JNIEnv* jni_env,
            jthread thread,
            jmethodID method,
            jlocation location,
            jobject exception,
            jmethodID catch_method,
            jlocation catch_location) {
    char *exception_signature;

    /* Obtain signature of the exception and compare type name with FNFE */
    jclass class = (*jni_env)->GetObjectClass(jni_env, exception);
    (*jvmti)->GetClassSignature(jvmti_env, class, &exception_signature, NULL);
    if (strcmp("Ljava/io/FileNotFoundException;", exception_signature)==0) {
        JmmInterface* jmm = (JmmInterface*) JVM_GetManagement(JMM_VERSION_1_0);
        if (jmm == NULL) {
            printf("Sorry, JMM is not supported\n");
        } else {
            jstring path = (*jni)->NewStringUTF(jni, "dump.hprof");
            jmm->DumpHeap0(jni, path, JNI_TRUE);
            printf("Heap dumped\n");
        }   
    }   
}   

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* vm, char* options, void* reserved) {
    (*vm)->GetEnv(vm, (void**)&jvmti, JVMTI_VERSION_1_0);

    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.Exception = Exception;
    (*jvmti)->SetEventCallbacks(jvmti, &callbacks, sizeof(callbacks));
    (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_EXCEPTION, NULL)

    return 0;
}
