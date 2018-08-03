name: ScaleSpecs
enabled: True
endpoint:
  dst: None
  src: None

modules:
    - module:
        name    : PING TEST
        program : Ping
    - module:
        name    : HTTP TEST
        program : HTTP
