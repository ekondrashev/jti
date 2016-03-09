#include <stdlib.h>
#include "jvmti.h"
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

jvmtiEnv *globalJVMTIInterface;
jclass callbackClass;
jmethodID callbackFieldModifyMethodID;
bool inited=0;

/* Given a jclass, get the class name any way you can */
static void
get_class_name(jvmtiEnv *jvmti, jclass klass, char *cname, int maxlen)
{
    jvmtiError error;
    char      *signature;

    error = (*jvmti)->GetClassSignature(jvmti, klass, &cname, NULL);
    if ( error != JVMTI_ERROR_NONE ) {
        (void)strcpy(cname, "Unknown");
	printf("C:\t Error getting signature\n");
        return;
    }
    if ( signature == NULL ) {
	printf("C:\t Error getting signature: NULL\n");
        (void)strcpy(cname, "Unknown");
        return;
    } else {
	printf("C:\t Got signature\n");
//        int len;

//        len = (int)strlen(signature);
//        if ( len > 2 && signature[0] == 'L' && signature[len-1] == ';' ) {
//           if ( len-2 >= maxlen ) {
//                (void)strncpy(cname, signature+1, maxlen-1);
//                cname[maxlen-1] = 0;
//            } else {
//                (void)strncpy(cname, signature+1, len-2);
//                cname[len-2] = 0;
//            }
//        } else {
//            if ( len >= maxlen ) {
//                (void)strncpy(cname, signature, maxlen-1);
//                cname[maxlen-1] = 0;
//            } else {
//                (void)strcpy(cname, signature);
//            }
//        }
//        deallocate(jvmti, signature);
//        jvmtiError error = (*jvmti)->Deallocate(jvmti, (unsigned char *) signature);
//        if (error != JVMTI_ERROR_NONE)
//              fprintf(stderr, "!!!\n");
    }
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
//    char *className = "ek/JVMTICallback";
//    char *methodName = "FieldModify";
//    char *descriptor = "(Ljava/lang/String;)V";
//
//    jclass callbackClass = (*jni_env)->FindClass(jni_env, className);
//
//      if (!callbackClass) {
//          fprintf(stderr,"C:\tUnable to locate callback class.\n");
//          return;
//          }
//


    char *name_ptr;
    char *cls_name=NULL;
    char *signature_ptr=NULL;
    char *generic_ptr=NULL;
    jvmtiError error;


    if (inited)
    {
      printf("C:\tField modify catched\n");
      (*jvmti_env)->GetFieldName(jvmti_env,
        field_klass, field, &name_ptr, &signature_ptr, &generic_ptr);
      printf("C:\t Field Name: %s\n", name_ptr);
      jclass clsObj = (*jni_env)->GetObjectClass(jni_env, object);
      if (!clsObj)
       {
        fprintf(stderr, "C:\tUnable to get cls\n");
        return;
       }
      printf("C:\t Got class obj\n");
      get_class_name(jvmti_env, clsObj, cls_name, 50);
      printf("C:\t Name: %s->%s\n", cls_name, name_ptr);
    jstring jstr1 = (*jni_env)->NewStringUTF(jni_env, name_ptr);
    (*jni_env)->CallStaticVoidMethod(jni_env, callbackClass, callbackFieldModifyMethodID, jstr1);
    //TODO:ek:relese jstr1
    error = (*jvmti_env)->Deallocate(jvmti_env, (unsigned char *) name_ptr);
        if (error != JVMTI_ERROR_NONE)
              fprintf(stderr, "!!!\n");
    error = (*jvmti_env)->Deallocate(jvmti_env, (unsigned char *) cls_name);
        if (error != JVMTI_ERROR_NONE)
              fprintf(stderr, "!!!\n");
    }

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


void JNICALL
vmInit(jvmtiEnv * jvmti_env, JNIEnv * jni_env, jthread thread)
{

  printf("C:\tVMInit, preparing to callback Java method\n");

  char *className = "ek/JVMTICallback";
  char *methodName = "VMInit";
  char *descriptor = "()V";

  callbackClass = (*jni_env)->FindClass(jni_env, className);
  callbackClass = (jclass)(*jni_env)->NewGlobalRef(jni_env, callbackClass);

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
  printf("C:\tMethod found1\n");

  methodName = "FieldModify";
  descriptor = "(Ljava/lang/String;)V";
  callbackFieldModifyMethodID = (*jni_env)->GetStaticMethodID(jni_env, callbackClass, methodName, descriptor);
  callbackFieldModifyMethodID = (jmethodID)(*jni_env)->NewGlobalRef(jni_env, callbackFieldModifyMethodID);

  if (!callbackFieldModifyMethodID)
    {
      fprintf(stderr, "C:\tUnable to locate callback field modify method\n");
      return;
    }
  printf("C:\tMethod found2\n");

  va_list empty_va_list;
  (*jni_env)->CallStaticVoidMethod(jni_env, callbackClass, callbackMethodID);

  printf("C:\tVMInit, callback Java method returned successfully\n");

  jmethodID ctorMethod = (*jni_env)->GetMethodID(jni_env, callbackClass, "<init>", "()V");
  if (!ctorMethod)
    {
      fprintf(stderr, "C:\tUnable to locate constructor\n");
      return;
    }
  printf("C:\t Got ctor\n");  
  jobject jObj = (*jni_env)->NewObject(jni_env, callbackClass, ctorMethod);
  if (!jObj)
    {
      fprintf(stderr, "C:\tUnable to create JVMTICallback\n");
      return;
    }
  printf("C:\t Got obj\n");  
  jclass clsObj = (*jni_env)->GetObjectClass(jni_env, jObj);
  if (!clsObj)
    {
      fprintf(stderr, "C:\tUnable to get cls\n");
      return;
    }
  printf("C:\t Got class obj\n");  
  char *cls_name=NULL;
  get_class_name(jvmti_env, clsObj, cls_name, 50);
  printf("C:\t Cls Name: %s\n", cls_name);
  inited = true;

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

  jvmtiCapabilities *capa;
  capa = calloc(1, sizeof(jvmtiCapabilities));
  if (!capa)
   {
       fprintf(stderr, "Unable to allocate memory\n");
       return JVMTI_ERROR_OUT_OF_MEMORY;
   }
  capa->can_generate_field_modification_events = 1;
  returnCode = (*globalJVMTIInterface)->AddCapabilities(globalJVMTIInterface, capa);
  if (returnCode != JNI_OK)
          {
            fprintf(
                stderr,
                "C:\tJVM does not have the required capabilities, d(%d)\n",
                returnCode);
            exit(-1);
          }

  jvmtiEventCallbacks *eventCallbacks;

  eventCallbacks = calloc(1, sizeof(jvmtiEventCallbacks));
  if (!eventCallbacks)
    {
      fprintf(stderr, "Unable to allocate memory\n");
      return JVMTI_ERROR_OUT_OF_MEMORY;
    }

  eventCallbacks->ClassPrepare= &ClassPrepareCallback;
  eventCallbacks->FieldModification= &FieldModifyCallBack;
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
  returnCode = (*globalJVMTIInterface)->SetEventNotificationMode(
        globalJVMTIInterface, JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE, (jthread) NULL);
  if (returnCode != JNI_OK)
   {
    fprintf(
        stderr,
        "C:\tJVM does not have the required capabilities, JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE (%d)\n",
        returnCode);
    exit(-1);
   }

   returnCode = (*globalJVMTIInterface)->SetEventNotificationMode(
             globalJVMTIInterface, JVMTI_ENABLE, JVMTI_EVENT_FIELD_MODIFICATION, (jthread) NULL);
   if (returnCode != JNI_OK)
    {
     fprintf(
         stderr,
         "C:\tJVM does not have the required capabilities, JVMTI_ENABLE, JVMTI_EVENT_FIELD_MODIFICATION (%d)\n",
         returnCode);
     exit(-1);
    }

  return JVMTI_ERROR_NONE;
}
