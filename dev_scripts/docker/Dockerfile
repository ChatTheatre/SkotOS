FROM debian:buster

# Set up Apt Packages
RUN apt-get update -y
RUN apt-get upgrade -y
RUN curl -sL https://deb.nodesource.com/setup_15.x | bash -
RUN apt-get install nodejs npm -y
RUN apt-get install sudo git nginx-full cron -y

# Clone SkotOS
RUN git clone https://github.com/ChatTheatre/SkotOS /var/skotos

# Configure NGinX
COPY skotos_game.conf /etc/nginx/sites-available/
RUN ln -s /etc/nginx/sites-available/skotos_game.conf /etc/nginx/sites-enabled/skotos_game.conf
RUN rm -f /etc/nginx/sites-enabled/default
RUN nginx -s reload || nginx

# Configure websocket-to-tcp-tunnel
RUN mkdir -p /var/log/tunnel
RUN git clone https://github.com/ChatTheatre/websocket-to-tcp-tunnel /usr/local/websocket-to-tcp-tunnel
RUN cd /usr/local/websocket-to-tcp-tunnel && npm install
RUN cd /usr/local/websocket-to-tcp-tunnel && chmod a+x *.sh

COPY tunnel_config.json /usr/local/websocket-to-tcp-tunnel/config.json
COPY crontab.txt /root/crontab.txt

# Set up running the tunnel
RUN /usr/local/websocket-to-tcp-tunnel/search-tunnel.sh
RUN crontab /root/crontab.txt

RUN mkdir -p /var/www/html
RUN git clone https://github.com/ChatTheatre/orchil /var/www/html/client
COPY profiles.js /var/www/html/client/profiles.js
