### Submodule Dependencies
The contents of this directory include submodules pointing to forked versions of various external source repositories.

Using submodules is somewhat dangerous for the repo maintainer, since there isn't an easy way or tool for removing or renaming them.

The following repo's where added as submodules:
```
$ git submodule add https://github.com/verizonlabs/ts_sdk_c_platforms_none_nucleo-l476.git
$ git submodule add https://github.com/verizonlabs/ts_sdk_c_platforms_unix_raspberry-pi3.git
$ git submodule add https://github.com/verizonlabs/ts_sdk_c_platforms_none_frdm-k82f.git
$ git submodule add https://github.com/verizonlabs/ts_sdk_c_platforms_threadx_renesas_s5d9.git
```

This is how you delete a submodule,

1. delete the submodule section from .gitmodules
2. stage .gitmodules by git add .gitmodules
3. delete the submodule section from .git/config
4. run git rm --cached path_to_submodule
5. run rm -rf .git/modules/path_to_submodule
6. run git commit -m 'removed submodules, path_to_submodule'
7. run rm -rf path_to_submodule
