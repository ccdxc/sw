# Venice-sdk

Venice-sdk contains the UI generated code from the Venice objects and services. Pensando-swagger-ts-generator.tgz contains the code generator, which parse the venice swagger files to generate this code.

To run the autogen, after running npm install run 
```
node swaggerGen.js
```
or if you don't have node installed from sw/ run
```
make ui-autogen
```

## Model API

Object creations take in a json of values to set on the object, and a boolean of whether default values should be set on the created object.

setValues(): Updates the object to have the values of the passed in object.

getFromGroup() or this.$formGroup: Returns the angular form group associated with this object. When it is created, it will have the same values as the object. However, changes to the model object are not reflected in the form group, and vice-versa. To get values from the form group, use this.$formGroup.values

setModelToBeFormGroupValues(): Overwrites the form group values to have the model values.

setFormGroupValuesToBeModelValues(): Overwrites the model objects value to have the form groups values.

getPropInfo(name): Returns the swagger info of the given property name.