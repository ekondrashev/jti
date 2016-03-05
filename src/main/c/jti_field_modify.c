#include <stdlib.h>
#include "jvmti.h"
#include <stdarg.h>


jvmtiEnv *jvmti;

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

  va_list empty_va_list;
  (*jni_env)->CallStaticVoidMethod(jni_env, callbackClass, callbackMethodID);

  printf("C:\tVMInit, callback Java method returned successfully\n");


}


static void     JNICALL
FieldModifyToJavaCallBack(jvmtiEnv * jvmti_env,
		    JNIEnv * jni_env,
		    jthread thread,
		    jmethodID method,
		    jlocation location,
		    jclass field_klass,
		    jobject object,
		    jfieldID field, char signature_type, jvalue new_value)
{

    char *className = "ek/JVMTICallback";
    char *methodName = "FieldModify";
    char *descriptor = "(Ljava/lang/String;)V";

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


    char *name_ptr;
    char *signature_ptr=NULL;
    char *generic_ptr=NULL;
    (*jvmti_env)->GetFieldName(jvmti_env,
        field_klass, field, &name_ptr, &signature_ptr, &generic_ptr);

    va_list args;

    (*jni_env)->CallStaticVoidMethodV(jni_env, callbackClass, callbackMethodID, args);
    jvmtiError error = (*jvmti_env)->Deallocate(jvmti_env, (unsigned char *) name_ptr);
    if (error != JVMTI_ERROR_NONE)
          fprintf(stderr, "!!!\n");

}

static void     JNICALL
FieldModifyCallBack(jvmtiEnv * jvmti_env,
		    JNIEnv * jni_env,
		    jthread thread,
		    jmethodID method,
		    jlocation location,
		    jclass field_klass,
		    jobject object,
		    jfieldID field, char signature_type, jvalue new_value)
{
    // do nothing
    char *name_ptr;
    char *signature_ptr=NULL;
    char *generic_ptr=NULL;
    (*jvmti_env)->GetFieldName(jvmti_env,
        field_klass, field, &name_ptr, &signature_ptr, &generic_ptr);
    printf("C:\tField modify: %s\n", name_ptr);
    jvmtiError error = (*jvmti_env)->Deallocate(jvmti_env, (unsigned char *) name_ptr);
        if (error != JVMTI_ERROR_NONE)
    	fprintf(stderr, "!!!\n");
}

static void     JNICALL
ClassPrepareCallback(jvmtiEnv * jvmti_env,
		     JNIEnv * jni_env, jthread thread, jclass klass)
{
    jint            field_num;
    jfieldID       *fieldIDs;
    jvmtiError      error = (*jvmti_env)->GetClassFields(jvmti_env, klass, &field_num,
						      &fieldIDs);
    if (error != JVMTI_ERROR_NONE)
	fprintf(stderr, "!!!%d\n", error);
    for (int i = 0; i < field_num; i++) {
	error = (*jvmti_env)->SetFieldModificationWatch(jvmti_env, klass, fieldIDs[i]);
	if (error != JVMTI_ERROR_NONE)
	    fprintf(stderr, "!!!\n");
    }
    error = (*jvmti_env)->Deallocate(jvmti_env, (unsigned char *) fieldIDs);
    if (error != JVMTI_ERROR_NONE)
	fprintf(stderr, "!!!\n");
}
JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM * jvm, char *options, void *reserved)
{
    jvmtiCapabilities *capa;
    jvmtiError      error;

    //jint            ret =
	//(jvm)->GetEnv((void **) (&jvmti), JVMTI_VERSION_1_0);
	jint returnCode = (*jvm)->GetEnv(jvm, (void **) &jvmti,
          JVMTI_VERSION_1_0);

    if (returnCode != JNI_OK)
        {
          fprintf(stderr,
              "The version of JVMTI requested (1.0) is not supported by this JVM.\n");
          return JVMTI_ERROR_UNSUPPORTED_VERSION;
        }

    //(void) memset(&capa, 0, sizeof(jvmtiCapabilities));
    capa = calloc(1, sizeof(jvmtiCapabilities));
    capa->can_generate_field_modification_events = 1;
    error = (*jvmti)->AddCapabilities(jvmti, capa);
    if (error != JVMTI_ERROR_NONE)
	fprintf(stderr, "!!!\n");

    //error = (*jvmti)->SetEventNotificationMode(JVMTI_ENABLE,
	//				    JVMTI_EVENT_CLASS_PREPARE,
	//				    (jthread) NULL);
    //if (error != JVMTI_ERROR_NONE)
	//fprintf(stderr, "!!!\n");

	returnCode = (*jvmti)->SetEventNotificationMode(
              jvmti, JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE, (jthread) NULL);
      if (returnCode != JNI_OK)
        {
          fprintf(
              stderr,
              "C:\tJVM does not have the required capabilities, JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE (%d)\n",
              returnCode);
          exit(-1);
        }

    //error = (*jvmti)->SetEventNotificationMode(JVMTI_ENABLE,
	//				    JVMTI_EVENT_FIELD_MODIFICATION,
	//				    NULL);
    //if (error != JVMTI_ERROR_NONE)
	//fprintf(stderr, "!!!\n");

	returnCode = (*jvmti)->SetEventNotificationMode(
          jvmti, JVMTI_ENABLE, JVMTI_EVENT_FIELD_MODIFICATION, (jthread) NULL);
      if (returnCode != JNI_OK)
        {
          fprintf(
              stderr,
              "C:\tJVM does not have the required capabilities, JVMTI_ENABLE, JVMTI_EVENT_FIELD_MODIFICATION (%d)\n",
              returnCode);
          exit(-1);
        }

   returnCode = (*jvmti)->SetEventNotificationMode(
      jvmti, JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, (jthread) NULL);
  if (returnCode != JNI_OK)
    {
      fprintf(
          stderr,
          "C:\tJVM does not have the required capabilities, JVMTI_ENABLE, JVMTI_EVENT_VM_INIT (%d)\n",
          returnCode);
      exit(-1);
    }

    //jvmtiEventCallbacks callbacks;
    //(void) memset(&callbacks, 0, sizeof(callbacks));
    //callbacks.ClassPrepare = &ClassPrepareCallback;
    //callbacks.FieldModification = &FieldModifyCallBack;
    jvmtiEventCallbacks *callbacks;
    callbacks = calloc(1, sizeof(jvmtiEventCallbacks));
    callbacks->ClassPrepare= &ClassPrepareCallback;
    callbacks->FieldModification= &FieldModifyToJavaCallBack;
    callbacks->VMInit = &vmInit;
    if (!callbacks)
      {
        fprintf(stderr, "Unable to allocate memory\n");
        return JVMTI_ERROR_OUT_OF_MEMORY;
      }

    //error = (*jvmti)->SetEventCallbacks(&callbacks, (jint) sizeof(callbacks));
    //if (error != JVMTI_ERROR_NONE)
	//fprintf(stderr, "!!!\n");

	returnCode = (*jvmti)->SetEventCallbacks(jvmti,
          callbacks, (jint) sizeof(*callbacks));
      if (returnCode != JNI_OK)
        {
          fprintf(stderr, "C:\tJVM does not have the required capabilities (%d)\n",
              returnCode);
          exit(-1);
        }

    return JNI_OK;
}

JNIEXPORT void JNICALL
Agent_OnUnload(JavaVM * jvm)
{
    return;
}
