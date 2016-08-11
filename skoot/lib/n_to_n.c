/*
**	Maintain an efficient bidirectional data structure for N-to-N
**	relationships, such as that of a person to his hair colour,
**	or a player's list of sworn enemies.
**
**	We use the terms 'client' and 'service' for the N and the N;
**	each client has many services, each service has many clients.
**
**	See also /lib/one_to_one.c and /lib/n_to_n.c
**
**	Copyright Skotos Tech Inc 2000
**
**	Zell 000826
*/

private mapping client_to_services;
private mapping service_to_clients;

atomic void clear_all();
atomic void clear_membership(mixed client, mixed service);

void create() {
   clear_all();
}

atomic
void clear_all() {
   client_to_services = ([ ]);
   service_to_clients = ([ ]);
}

atomic
void delete_client(mixed client) {
   mixed *srv;
   int i;

   if (srv = client_to_services[client]) {
      for (i = 0; i < sizeof(srv); i ++) {
	 clear_membership(client, srv[i]);
      }
   }
   client_to_services[client] = nil;	/* should be empty by now */
}

atomic
void delete_service(mixed service) {
   mixed *clt;
   int i;

   if (clt = service_to_clients[service]) {
      for (i = 0; i < sizeof(clt); i ++) {
	 clear_membership(clt[i], service);
      }
   }
   service_to_clients[service] = nil;
}

# define MkClient(c) \
    if (!client_to_services[c]) client_to_services[c] = ([ ])

# define MkService(s) \
    if (!service_to_clients[s]) service_to_clients[s] = ([ ])

atomic
void clear_membership(mixed client, mixed service) {
   MkClient(client);
   MkService(service);
   client_to_services[client][service] = nil;
   service_to_clients[service][client] = nil;
}

atomic
void set_membership(string client, string service) {
   MkClient(client);
   MkService(service);
   client_to_services[client][service] = TRUE;
   service_to_clients[service][client] = TRUE;
}

mixed *query_services(mixed client) {
   MkClient(client);
   return map_indices(client_to_services[client]);
}

mixed *query_clients(mixed service) {
   MkService(service);
   return map_indices(service_to_clients[service]);
}
