# xz-detector

Just `xz --version` is not enough!

Recently there is a security report on `xz` version 5.6.0 and 5.6.1, that these versions of `xz` are malicious. However, simply calling `xz --version` just checks the default `xz` installed in our system. We don't know if there are any other versions of `xz` installed and used in our system.

To perform a thorough check, we can use the [linker audit mechanism](https://man7.org/linux/man-pages/man7/rtld-audit.7.html) to audit all the library load, to find if a program links to `liblzma.so` in any means.

Steps:

```shell
git clone git@github.com:youkaichao/xz-detector.git
cd xz-detector
gcc -shared -fPIC -o audit.so audit.c -ldl
export AUDIT_SO_PATH=$(realpath audit.so)
```

Then, to check if a program links to `liblzma.so` , we can run the command prefixed with `LD_AUDIT=$AUDIT_SO_PATH` .

For example:

```shell
AUDIT_PROGRAM=torch LD_AUDIT=$AUDIT_SO_PATH python -c "import torch; print(torch.randn(5).sum())"
```

After the program finishes, we can see three files: `torch-found.txt`, `torch-search.txt`, `torch-symbol.txt`.

With `grep liblzma.so torch-*.txt` , we can tell if our program is linked to `liblzma.so` . Unfortunately, many programs link to it. Hope that version 5.6.0 is the first poisoned version, and previous versions are good.

What's more problematic, every new conda environment will install `xz`: when we execute `conda create -n name python=3.9` , we will see the execution plan:

```
The following NEW packages will be INSTALLED:

  _libgcc_mutex      pkgs/main/linux-64::_libgcc_mutex-0.1-main 
  _openmp_mutex      pkgs/main/linux-64::_openmp_mutex-5.1-1_gnu 
  ca-certificates    pkgs/main/linux-64::ca-certificates-2024.3.11-h06a4308_0 
  ld_impl_linux-64   pkgs/main/linux-64::ld_impl_linux-64-2.38-h1181459_1 
  libffi             pkgs/main/linux-64::libffi-3.4.4-h6a678d5_0 
  libgcc-ng          pkgs/main/linux-64::libgcc-ng-11.2.0-h1234567_1 
  libgomp            pkgs/main/linux-64::libgomp-11.2.0-h1234567_1 
  libstdcxx-ng       pkgs/main/linux-64::libstdcxx-ng-11.2.0-h1234567_1 
  ncurses            pkgs/main/linux-64::ncurses-6.4-h6a678d5_0 
  openssl            pkgs/main/linux-64::openssl-3.0.13-h7f8727e_0 
  pip                pkgs/main/linux-64::pip-23.3.1-py39h06a4308_0 
  python             pkgs/main/linux-64::python-3.9.19-h955ad1f_0 
  readline           pkgs/main/linux-64::readline-8.2-h5eee18b_0 
  setuptools         pkgs/main/linux-64::setuptools-68.2.2-py39h06a4308_0 
  sqlite             pkgs/main/linux-64::sqlite-3.41.2-h5eee18b_0 
  tk                 pkgs/main/linux-64::tk-8.6.12-h1ccaba5_0 
  tzdata             pkgs/main/noarch::tzdata-2024a-h04d1e81_0 
  wheel              pkgs/main/linux-64::wheel-0.41.2-py39h06a4308_0 
  xz                 pkgs/main/linux-64::xz-5.4.6-h5eee18b_0 
  zlib               pkgs/main/linux-64::zlib-1.2.13-h5eee18b_0 
```

And `xz` is there.

# One more thing

The above program `audit.so` can also be used to trace symbol and library resolution, for debugging purpose.
