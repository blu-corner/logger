/*
 * Copyright 2014-2018 Neueda Ltd.
 */
%pragma(java) jniclasscode=%{
     // jniclasscode pragma code: Static block so that the JNI class loads the C++ DLL/shared object when the class is loaded
     static {
         try {
             System.loadLibrary("Logger");
         } catch (UnsatisfiedLinkError e) {
             System.err.println("Native code library failed to load.\n" + e);
             System.exit(1);
         }
     }
%}

%include "exception.i"
%exception {
    try {
        $action
    } catch (std::exception &e) {
        std::string s("logger-error: "), s2(e.what());
        s = s + s2;
        SWIG_exception(SWIG_RuntimeError, s.c_str());
    }
}

%pragma(java) moduleimports=%{
import com.neueda.properties.Properties;
%}

%pragma(java) jniclassimports=%{
import com.neueda.properties.Properties;
%}

%typemap(javaimports) SWIGTYPE %{
import com.neueda.properties.Properties;
%}

SWIG_JAVABODY_PROXY(public, public, SWIGTYPE)
SWIG_JAVABODY_TYPEWRAPPER(public, public, public, SWIGTYPE)

%javaexception("java.lang.Exception") neueda::logService::addHandler {
    try {
        $action
    } catch (std::exception &e) {
        std::string s("properties: "), s2(e.what());
        s = s + s2;
        SWIG_exception(SWIG_RuntimeError, s.c_str());
    }
 }

%javaexception("java.lang.Exception") neueda::logService::configure {
    try {
        $action
    } catch (std::exception &e) {
        std::string s("properties: "), s2(e.what());
        s = s + s2;
        SWIG_exception(SWIG_RuntimeError, s.c_str());
    }
 }
     
%include "logger.i"
