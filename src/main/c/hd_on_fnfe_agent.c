//http://stackoverflow.com/questions/31001558/jvmti-heap-dump-on-exception-stacktrace-issues
#include <jni.h>
#include <jvmti.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

JNIEXPORT void* JNICALL JVM_GetManagement(jint version);
jvmtiEnv* jvmti;
char* signature;
char* msg;

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}


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
        jclass clazz = (*jni_env)->GetObjectClass(jni_env, exception);
        jmethodID getMessage = (*jni_env)->GetMethodID(jni_env, clazz,
                                                        "getMessage",
                                                        "()Ljava/lang/String;");
        jstring message = (jstring)(*jni_env)->CallObjectMethod(jni_env, exception, getMessage);
        if (message != NULL) {
            char *mstr = (*jni_env)->GetStringUTFChars(jni_env, message, NULL);
            (*jni_env)->ReleaseStringUTFChars(jni_env, message, mstr);
            if (strcmp(msg, mstr)==0) {
                printf("Got requested exception message match: %s\n", msg);
                char cmdline [100];
                sprintf(cmdline, "jmap -F -dump:live,format=b,file=dump.hprof %d", getpid());
                int status = system(cmdline);
                printf("Done generating heap dump, status code %d\n", status);
            } else {
                printf("Message did not match: %s vs %s\n", msg, mstr);
            }
        } else {
            printf("Message is NULL");
        }
        (*jni_env)->DeleteLocalRef(jni_env, message);
        (*jni_env)->DeleteLocalRef(jni_env, clazz);
    }
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM* vm, char* options, void* reserved) {
    char** tokens;
    tokens = str_split(options, '|');
    signature = tokens[0];
    msg=tokens[1];
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
