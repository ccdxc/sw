# Delphictl utility

Delphictl utility helps examine the contents of delphi database.

Delphictl can display contents of delphi database or delphi metrics. It support CRUD operations (list, get, set, delete) on objects in either database

### Examples

```
delphictl db list InterfaceSpec
```

Will mount objects of kind `InterfaceSpec` and display all objects in that subtree.

```
delphictl db get InterfaceSpec 1234
```

Will display `InterfaceSpec` object with key '1234'

```
delphictl metrics list InterfaceMetrics
```

Will display all the metrics of kind `InterfaceMetrics`. This command will decode the contents of the metrics entries and display in human readable form.

```
delphictl metrics get InterfaceMetrics 1234
```

Will display `InterfaceMetrics` object with key '1234'

```
delphictl shm
```

Will display state of delphi shared memory. This will mostly display the metadata about the tables in key-value store.

```
delphictl shm InterfaceMetrics
```

Will display contents of the shared memory table `InterfaceMetrics`. This command displays the raw bytes in key-value store without decoding it.

