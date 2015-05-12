{
  "targets": [
    {
      "target_name": "fdpassing",
      "sources": [ "fdpassing.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
