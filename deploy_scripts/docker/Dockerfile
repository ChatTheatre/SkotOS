FROM debian:buster

# DGD HTTP port (direct)
EXPOSE 10080

# DGD TextIF port
EXPOSE 10443

# DGD telnet port
EXPOSE 10098

# DGD emergency binary port (admin-only)
EXPOSE 10099

# Set up Apt Packages
RUN apt-get update -y
RUN apt-get upgrade -y
RUN apt-get install sudo build-essential bison git nginx-full -y
RUN apt-get update -y

# Build DGD
RUN git clone https://github.com/ChatTheatre/dgd /var/dgd
RUN cd /var/dgd/src && make DEFINES='-DUINDEX_TYPE="unsigned int" -DUINDEX_MAX=UINT_MAX -DEINDEX_TYPE="unsigned short" -DEINDEX_MAX=USHRT_MAX -DSSIZET_TYPE="unsigned int" -DSSIZET_MAX=1048576' install

# Clone SkotOS
RUN git clone https://github.com/ChatTheatre/SkotOS /var/skotos
COPY patch_devuserd.sh /root/patch_devuserd.sh
RUN chmod +x /root/patch_devuserd.sh
RUN cd /var/skotos && /root/patch_devuserd.sh

# Configure SkotOS as "the app"
COPY start_dgd_server.sh /var/start_dgd_server.sh
RUN chmod +x /var/start_dgd_server.sh
CMD cd /var && ./start_dgd_server.sh
