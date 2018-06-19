# Delphictl utility

Delphictl utility helps examine the contents of delphi database.

### Examples

```
delphictl -kind InterfaceSpec
```

Will mount objects of kind `InterfaceSpec` and display all objects in that subtree.

```
delphictl -shm
```

Will display state of delphi shared memory. This will mostly display the metadata about the tables in key-value store.

```
delphictl -shm -tbl InterfaceMetrics
```

Will display contents of the shared memory table `InterfaceMetrics`. This command displays the raw bytes in key-value store without decoding it.

```
delphictl -shm -metrics InterfaceMetrics
```

Will display all the metrics of kind `InterfaceMetrics`. This command will decode the contents of the metrics entries and display in human readable form.
