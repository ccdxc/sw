
### How to use memkv
See the example in `../examples/memkv/user.go`. The example does few simple things to illustrate the use of it:

#### Create memkv
```
	s := runtime.NewScheme()
	s.AddKnownTypes(&User{}, &UserList{})

	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(s)}

	kv, err := store.New(config)
	if err != nil {
		log.Fatalf("Failed to create store with error: %v", err)
	}
```

#### Create, Get operations on Kvstore (works as usual)
```
	key := "/users/joe"
	user := User{}
	if err := kv.Create(context.Background(), key, &user, 0, &user); err != nil {
		log.Errorf("error storing key in the key-value store")
	}
	if err := kv.Get(context.Background(), key, &user); err != nil {
		log.Errorf("error getting key in the key-value store")
	}
```
