# Install cgal library

1. Clone repo from https://github.com/CGAL/cgal
2. Move to the rerepo directory:
    ```shell
   cd <REPO_DIR>
    ```
3. Configure and install with cmake:
    `CGAL` is a header only library, there is no need for build.
    ```shell
   rm -rf build && mkdir build && cd build && cmake -DCMAKE_INSTALL_PREFIX=./install .. &&  cmake --install . && cd ..
    ```
   You can create an alias for your preferred shell:
    e.g. for `zsh` copy the alias bellow in `~/.zshrc`:
    ```
   alias cmake_install_cgal="rm -rf build && mkdir build && cd build && cmake -DCMAKE_INSTALL_PREFIX=./install .. &&  cmake --install . && cd .."
    ```
   Notice the `./install` directory where all headers are installed.
