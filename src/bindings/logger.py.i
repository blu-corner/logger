/*
 * Copyright 2014-2018 Neueda Ltd.
 */
%feature("autodoc");

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

%include "logger.i"
