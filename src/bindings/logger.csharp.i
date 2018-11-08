/*
 * Copyright 2014-2018 Neueda Ltd.
 */
%typemap(throws, canthrow=1) std::runtime_error {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpApplicationException, $1.what(), NULL);
    return $null;
}

%typemap(csimports) SWIGTYPE %{
using Neueda.Properties;
%}

SWIG_CSBODY_PROXY(public, public, SWIGTYPE)
SWIG_CSBODY_TYPEWRAPPER(public, public, public, SWIGTYPE)

%include "logger.i"
