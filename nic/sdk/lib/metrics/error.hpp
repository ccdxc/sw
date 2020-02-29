// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
// This file was forked from delphi

#ifndef _SDK_METRICS_ERROR_H_
#define _SDK_METRICS_ERROR_H_

#include <assert.h>
#include <string>

namespace sdk {
namespace metrics {

// error: base class for all error return values
class error {
public:
    // Constructor
    error() {
        errstr_ = NULL;
    }
    error(const char *errstr) {
        errstr_ = errstr;
    }

    // Return error string
    virtual std::string Error() {
        if (this->errstr_ == NULL) {
            return "";
        }
        return std::string(this->errstr_);
    }

    // Is this an error or OK?
    inline bool IsOK() {
        if (errstr_ == NULL) {
            return true;
        }
        return false;
    }

    // inverse of IsOK
    inline bool IsNotOK() {
        return !this->IsOK();
    }

    // Check if two error objects are same
    inline bool operator==(error rhs)const{
        if (this->errstr_ == rhs.errstr_) {
            return true;
        } else if ((this->errstr_ != NULL) && (rhs.errstr_ != NULL) && (std::string(this->errstr_) == std::string(rhs.errstr_))) {
            return true;
        }
        return false;
    }

    // Check if two error objects are not same
    inline bool operator!=(error rhs)const{
        if (this->errstr_ == rhs.errstr_) {
            return false;
        } else if ((this->errstr_ != NULL) && (rhs.errstr_ != NULL) && (std::string(this->errstr_) == std::string(rhs.errstr_))) {
            return false;
        }
        return true;
    }

    // override << operator to pretty print the error object
    friend std::ostream& operator<<(std::ostream& os, const error &err) {
       return os << ((error)err).Error();
    }

    // return a new error object with specified error string
    static inline error New(const char *errstr) {
        // Some error checks. errstr cant be NULL and cant be an empty string
        assert(errstr != NULL);
        assert(std::string(errstr) != "");

        // return an error object
        return error(errstr);
    }

    static inline error OK() {
        return error(NULL);
    }
private:
    const char *errstr_;
};

} // namespace metrics
} // namespace sdk

#endif // _SDK_METRICS_ERROR_H_
