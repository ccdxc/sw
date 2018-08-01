name: Host-Host
enabled: True
endpoint:
  dst: host
  src: host

modules:
    - module:
        name    : PING TEST
        program : Ping
    - module:
        name    : HTTP TEST
        program : HTTP
