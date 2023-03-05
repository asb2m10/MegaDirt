FROM haskell:slim

WORKDIR /home/tidal

EXPOSE 6010/udp

RUN cabal update && \
    cabal install --lib tidal && \
    rm -f -r ~/.cabal/packages

RUN apt-get update && \
    apt-get install nano busybox -y && \
    rm -rf /var/lib/apt/lists/*

COPY tidal.ghci /home/tidal
COPY tidal /home/tidal

RUN chmod +x /home/tidal/tidal

ENTRYPOINT [ "/home/tidal/tidal" ]