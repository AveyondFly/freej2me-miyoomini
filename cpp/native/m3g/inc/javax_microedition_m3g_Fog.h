/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class javax_microedition_m3g_Fog */

#ifndef _Included_javax_microedition_m3g_Fog
#define _Included_javax_microedition_m3g_Fog
#ifdef __cplusplus
extern "C" {
#endif
#undef javax_microedition_m3g_Fog_EXPONENTIAL
#define javax_microedition_m3g_Fog_EXPONENTIAL 80L
#undef javax_microedition_m3g_Fog_LINEAR
#define javax_microedition_m3g_Fog_LINEAR 81L
/*
 * Class:     javax_microedition_m3g_Fog
 * Method:    _ctor
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_javax_microedition_m3g_Fog__1ctor
  (JNIEnv *, jclass, jlong);

/*
 * Class:     javax_microedition_m3g_Fog
 * Method:    _setMode
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_javax_microedition_m3g_Fog__1setMode
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     javax_microedition_m3g_Fog
 * Method:    _getMode
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_javax_microedition_m3g_Fog__1getMode
  (JNIEnv *, jclass, jlong);

/*
 * Class:     javax_microedition_m3g_Fog
 * Method:    _setLinear
 * Signature: (JFF)V
 */
JNIEXPORT void JNICALL Java_javax_microedition_m3g_Fog__1setLinear
  (JNIEnv *, jclass, jlong, jfloat, jfloat);

/*
 * Class:     javax_microedition_m3g_Fog
 * Method:    _getDistance
 * Signature: (JI)F
 */
JNIEXPORT jfloat JNICALL Java_javax_microedition_m3g_Fog__1getDistance
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     javax_microedition_m3g_Fog
 * Method:    _setDensity
 * Signature: (JF)V
 */
JNIEXPORT void JNICALL Java_javax_microedition_m3g_Fog__1setDensity
  (JNIEnv *, jclass, jlong, jfloat);

/*
 * Class:     javax_microedition_m3g_Fog
 * Method:    _getDensity
 * Signature: (J)F
 */
JNIEXPORT jfloat JNICALL Java_javax_microedition_m3g_Fog__1getDensity
  (JNIEnv *, jclass, jlong);

/*
 * Class:     javax_microedition_m3g_Fog
 * Method:    _setColor
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_javax_microedition_m3g_Fog__1setColor
  (JNIEnv *, jclass, jlong, jint);

/*
 * Class:     javax_microedition_m3g_Fog
 * Method:    _getColor
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_javax_microedition_m3g_Fog__1getColor
  (JNIEnv *, jclass, jlong);

#ifdef __cplusplus
}
#endif
#endif