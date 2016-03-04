#include <stdlib.h>
#include "jvmti.h"


jvmtiEnv *globalJVMTIInterface;

void JNICALL
vmInit(jvmtiEnv * jvmti_env, JNIEnv * jni_env, jthread thread)
{

  printf("C:\tVMInit, preparing to callback Java method\n");

  char *className = "ek/JVMTICallback";
  char *methodName = "VMInit";
  char *descriptor = "()V";

  jclass callbackClass = (*jni_env)->FindClass(jni_env, className);

  if (!callbackClass) {
      fprintf(stderr,"C:\tUnable to locate callback class.\n");
      return;
      }
  printf("C:\tClass found\n");

  jmethodID callbackMethodID = (*jni_env)->GetStaticMethodID(jni_env, callbackClass, methodName, descriptor);

  if (!callbackMethodID)
    {
      fprintf(stderr, "C:\tUnable to locate callback VMInit method\n");
      return;
    }
  printf("C:\tMethod found\n");

  (*jni_env)->CallStaticVoidMethodV(jni_env, callbackClass, callbackMethodID, NULL);

  printf("C:\tVMInit, callback Java method returned successfully\n");


}

JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM * jvm, char *options, void *reserved)
{

  jint returnCode = (*jvm)->GetEnv(jvm, (void **) &globalJVMTIInterface,
      JVMTI_VERSION_1_0);

  if (returnCode != JNI_OK)
    {
      fprintf(stderr,
          "The version of JVMTI requested (1.0) is not supported by this JVM.\n");
      return JVMTI_ERROR_UNSUPPORTED_VERSION;
    }

  jvmtiEventCallbacks *eventCallbacks;

  eventCallbacks = calloc(1, sizeof(jvmtiEventCallbacks));
  if (!eventCallbacks)
    {
      fprintf(stderr, "Unable to allocate memory\n");
      return JVMTI_ERROR_OUT_OF_MEMORY;
    }

  eventCallbacks->VMInit = &vmInit;

  returnCode = (*globalJVMTIInterface)->SetEventCallbacks(globalJVMTIInterface,
      eventCallbacks, (jint) sizeof(*eventCallbacks));
  if (returnCode != JNI_OK)
    {
      fprintf(stderr, "C:\tJVM does not have the required capabilities (%d)\n",
          returnCode);
      exit(-1);
    }

  returnCode = (*globalJVMTIInterface)->SetEventNotificationMode(
      globalJVMTIInterface, JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, (jthread) NULL);
  if (returnCode != JNI_OK)
    {
      fprintf(
          stderr,
          "C:\tJVM does not have the required capabilities, JVMTI_ENABLE, JVMTI_EVENT_VM_INIT (%d)\n",
          returnCode);
      exit(-1);
    }

  return JVMTI_ERROR_NONE;
}
