import { BaseModel } from "../models/generated/cluster/base-model";
import * as _ from 'lodash';

  // Walks the object and if a nested object has only null or defaults, it is
  // removed fromt he request
  // Request should be an object with an interface of a Venice_sdk model object
  // model should be the venice sdk model object
  // This is useful for removing nested objects that have validation that
  // only apply if an object is given (Ex. pagination spec for telemetry queries)
  export const TrimDefaultsAndEmptyFields = (request: any, model) => {
    request = _.cloneDeep(request);
    const helperFunc = (obj, model): boolean => {
      let retValue = true;
      for (const key in obj) {
        if (obj.hasOwnProperty(key)) {
          // We want to send the timestamp as a string
          // to the backend, not as a js date object.
          if (obj[key] instanceof Date) {
            obj[key] = obj[key].toISOString();
          }
          if (_.isObjectLike(obj[key])) {
            if (helperFunc(obj[key], model[key])) {
              delete obj[key];
            } else {
              retValue = false;
            }
          } else if (obj[key] != null && (model.getPropInfo == null || model.getPropInfo(key)== null || obj[key] !== model.getPropInfo(key).default)) {
            retValue = false;
          }
        }
      }
      return retValue;
    };
    helperFunc(request, model);
    return request;
  }