FROM debian:buster

# NGinX serving game client
EXPOSE 10900

# NGinX websocket port, forwarded to internal tunnel
EXPOSE 10800

# Tunnel-to-WOE Relay
EXPOSE 10802

# DGD HTTP port (direct)
EXPOSE 10080

# Future TODO: install supervisor package, run tunnels and DGD via supervisord in start_dgd_server.sh

# Set up Apt Packages
RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt-get install sudo build-essential bison git nginx-full -y
RUN curl -sL https://deb.nodesource.com/setup_14.x | bash -
RUN apt-get update -y
RUN apt-get install nodejs npm -y

# Configure websocket-to-tcp-tunnel
RUN mkdir -p /var/log/tunnel
RUN git clone https://github.com/ChatTheatre/websocket-to-tcp-tunnel /usr/local/websocket-to-tcp-tunnel
RUN cd /usr/local/websocket-to-tcp-tunnel && npm install
RUN cd /usr/local/websocket-to-tcp-tunnel && chmod a+x *.sh

COPY tunnel_config.json /usr/local/websocket-to-tcp-tunnel/config.json

# Build DGD
RUN git clone https://github.com/ChatTheatre/dgd /var/dgd
RUN cd /var/dgd/src && make DEFINES='-DUINDEX_TYPE="unsigned int" -DUINDEX_MAX=UINT_MAX -DEINDEX_TYPE="unsigned short" -DEINDEX_MAX=USHRT_MAX -DSSIZET_TYPE="unsigned int" -DSSIZET_MAX=1048576' install

# Clone SkotOS
RUN git clone https://github.com/ChatTheatre/SkotOS /var/skotos
COPY patch_devuserd.sh /root/patch_devuserd.sh
RUN chmod +x /root/patch_devuserd.sh
RUN bash /root/patch_devuserd.sh

# Configure NGinX
COPY skotos_game.conf /etc/nginx/sites-available/skotos_game.conf
RUN ln -s /etc/nginx/sites-available/skotos_game.conf /etc/nginx/sites-enabled/skotos_game.conf
RUN rm -f /etc/nginx/sites-enabled/default

RUN mkdir -p /var/www/html
RUN git clone https://github.com/ChatTheatre/orchil /var/www/html/client
COPY profiles.js /var/www/html/client/profiles.js

# Configure SkotOS as "the app"
COPY start_dgd_server.sh /var/start_dgd_server.sh
RUN chmod +x /var/start_dgd_server.sh
CMD cd /var && ./start_dgd_server.sh
