{
  "targets": [
    {
      "target_name": "msg",
      "sources": [ "src/msg.cc", "src/functions.cc" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
