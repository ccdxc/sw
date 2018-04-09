name: Host-Remote
enabled: True
endpoint: 
  dst: host
  src: remote
modules:
    - module:
        name    : PING TEST
        program : ping_test
    - module:
        name    : HTTP TEST
        program : http_test
