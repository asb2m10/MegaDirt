FROM asb2m10/tidalcycles
 
RUN apt-get update && \
    apt-get install tmux neovim -y \
    && rm -rf /var/lib/apt/lists/*

RUN busybox wget https://github.com/tidalcycles/vim-tidal/archive/refs/tags/1.4.8.tar.gz -O tidalvim.tar.gz && \
    tar xzvf tidalvim.tar.gz && \ 
    rm tidalvim.tar.gz && \
    mkdir -p ~/.local/share/nvim/site/pack/plugins/start && \
    mv vim-tidal-* ~/.local/share/nvim/site/pack/plugins/start && \
    mkdir -p ~/.config/nvim && \
    echo "filetype plugin on" > ~/.config/nvim/init.vim && \
    busybox wget https://github.com/vim-airline/vim-airline/archive/refs/tags/v0.11.tar.gz -O vim-airline.tar.gz && \
    tar xzvf vim-airline.tar.gz && \
    rm vim-airline.tar.gz && \
    mv vim-airline* ~/.local/share/nvim/site/pack/plugins/start

COPY tidalvim /home/tidal

RUN chmod +x /home/tidal/tidalvim

WORKDIR /work
VOLUME /work

ENTRYPOINT [ "/home/tidal/tidalvim" ]
