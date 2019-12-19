import { BaseModel } from "../models/generated/basemodel/base-model";
import * as _ from 'lodash';

  // Walks the object and if a nested object has only null or defaults, it is
  // removed fromt he request
  // Request should be an object with an interface of a Venice_sdk model object
  // model should be the venice sdk model object
  // This is useful for removing nested objects that have validation that
  // only apply if an object is given (Ex. pagination spec for telemetry queries)
  // If the field's empty value is present in previousVal, the field is not trimmed
  export const TrimDefaultsAndEmptyFields = (request: any, model, previousVal = null,
                                      trimDefaults = true) => {
    request = _.cloneDeep(request);
    const helperFunc = (obj, model, previousVal): boolean => {
      let retValue = true;
      for (const key in obj) {
        if (obj.hasOwnProperty(key)) {
          // for debug: console.log(key, obj, model);
          // We want to send the timestamp as a string
          // to the backend, not as a js date object.
          if (obj[key] instanceof Date) {
            obj[key] = obj[key].toISOString();
          }
          if (_.isObjectLike(obj[key])) {
            let prevVal = previousVal
            if (previousVal != null) {
              prevVal = previousVal[key]
            }
            if (helperFunc(obj[key], model[key], prevVal)) {
              // If the existing object has a value as {}, we submit the value 
              // as {}, not null
              if (_.isEqual(prevVal, {})) {
                obj[key] = {};
              } else {
                delete obj[key];
              }
            } else {
              /* this is for object.key is an array
               * delete key for array object will won't for array object
               * if there are multiple items in an array and one of them is not empty
               * after delete key, the array will be [object, null, null, ...]
               * and array length is not 1. you need removed those empty object
               */
              if (Array.isArray(obj[key]) && obj[key].length > 0) {
                obj[key] = obj[key].filter(item => (item) ? true : false);
              }
              retValue = false;
            }
          } else if (obj[key] != null && (trimDefaults === false || model == null || model.getPropInfo == null || model.getPropInfo(key)== null || obj[key] !== model.getPropInfo(key).default)) {
            retValue = false;
          }
        }
      }
      return retValue;
    };
    helperFunc(request, model, previousVal);
    return request;
  }