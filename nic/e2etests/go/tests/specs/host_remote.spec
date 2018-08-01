name: Host-Remote
enabled: True
endpoint:
  dst: host
  src: remote

modules:
    - module:
        name    : PING TEST
        program : Ping
    - module:
        name    : HTTP TEST
        program : HTTP
