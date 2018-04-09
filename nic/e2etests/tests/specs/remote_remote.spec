name: Remote-Remote
enabled: True
endpoint: 
  dst: remote
  src: remote

modules:
    - module:
        name    : PING TEST
        program : ping_test
    - module:
        name    : HTTP TEST
        program : http_test
