TidalVim
========

This is based on the TidalCycle image with the extension of Neovim with the 
[vim-tidal](https://github.com/tidalcycles/vim-tidal) installation.

See asb2m10/tidalcycle image for more information about running this image.

All tidal session files will be saved in the work directory.

Use C-b <up>/<down> to switch between tidal repl and nvim.

```
docker run -it -v ./work:/work --rm asb2m10/tidalvim <your host that is running Dirt IP>
```
