{
  "targets": [
    {
      "target_name": "msg",
      "sources": [ "src/msg.cc" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
