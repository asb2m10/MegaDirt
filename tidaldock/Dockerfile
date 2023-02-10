FROM haskell:slim

WORKDIR /home/tidal

RUN cabal update && \
    cabal install --lib tidal && \
    rm -f -r ~/.cabal/packages

RUN apt-get update && \
    apt-get install neovim -y \
    && rm -rf /var/lib/apt/lists/*

COPY Tidal.ghci /home/tidal/

ENTRYPOINT [ "ghci", "-ghci-script", "Tidal.ghci" ]