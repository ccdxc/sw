# Venice UI

Venice UI architecture, project setup documents are available [Here] https://drive.google.com/drive/folders/1thfvQGdoJYyojAUkRA_GmujQsGe2Tv0I)

To build all of the UI, from sw/ run 
```
make fixtures
```

venice UI is made of 3 main components

## Web-app-framework

Web-app-framework contains widgets and components that are general purpose. They are not meant to contain any business logic, and are intended to be consumed by any another angular app.

## Venice-sdk

Venice-sdk contains all the generated UI code. It generates typescript models and angular form groups and service from the Venice swagger files for use in webapp.

## webapp

The angular app that contains the business logic for Venice UI. Webapp imports both web-app-frameowrk and venice-sdk.
