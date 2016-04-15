//http://stackoverflow.com/questions/31001558/jvmti-heap-dump-on-exception-stacktrace-issues
#include <jni.h>
#include <jvmti.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

JNIEXPORT void* JNICALL JVM_GetManagement(jint version);
jvmtiEnv* jvmti;
char* signature;

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

    jclass class = (*jni_env)->GetObjectClass(jni_env, exception);
    (*jvmti)->GetClassSignature(jvmti_env, class, &exception_signature, NULL);
    if (strcmp(signature, exception_signature)==0) {
        printf("Got requested exception: %s\n", exception_signature);
        printf("Process ID : %d\n", getpid());
        char cmdline [100];
        sprintf(cmdline, "jmap -F -dump:live,format=b,file=dump.hprof %d", getpid());
        int status = system(cmdline);
        printf("Done generating heap dump, status code %d\n", status);
    }
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* vm, char* options, void* reserved) {
    signature = options;
    printf("Options: %s\n", options);
    (*vm)->GetEnv(vm, (void**)&jvmti, JVMTI_VERSION_1_0);
    jvmtiCapabilities *capa;
    capa = calloc(1, sizeof(jvmtiCapabilities));
    if (!capa) {
        fprintf(stderr, "Unable to allocate memory\n");
        return JVMTI_ERROR_OUT_OF_MEMORY;
    }
    capa->can_generate_exception_events = 1;
    jint returnCode = (*jvmti)->AddCapabilities(jvmti, capa);
    if (returnCode != JNI_OK) {
        fprintf(
            stderr,
            "C:\tJVM does not have the required capabilities, d(%d)\n",
            returnCode
        );
        exit(-1);
    }
    jvmtiEventCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.Exception = Exception;
    (*jvmti)->SetEventCallbacks(jvmti, &callbacks, sizeof(callbacks));
    (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_EXCEPTION, NULL);

    return JVMTI_ERROR_NONE;
}
