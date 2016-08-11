/*
**	Maintain an efficient bidirectional data structure for N-to-1
**	relationships, such as that of a person to the person's age,
**	or a player's faction membership.
**
**	We use the terms 'client' and 'service' for the N and the 1;
**	each client has one service, each service has many clients.
**
**	See also /lib/one_to_one.c and /lib/n_to_n.c
**
**	Copyright Skotos Tech Inc 2000
**
**	Zell 000826
*/

private mapping client_to_service;
private mapping service_to_clients;

atomic void clear_all();
atomic void clear_membership(mixed client, mixed service);

void create() {
   clear_all();
}

atomic
void clear_all() {
   client_to_service = ([ ]);
   service_to_clients = ([ ]);
}

atomic
void delete_client(mixed client) {
   clear_membership(client, client_to_service[client]);
   client_to_service[client] = nil;
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

# define MkService(s) \
    if (!service_to_clients[s]) service_to_clients[s] = ([ ])

atomic
void clear_membership(mixed client, mixed service) {
   MkService(service);
   client_to_service[client] = nil;
   service_to_clients[service][client] = nil;
}

atomic
void set_membership(string client, string service) {
   MkService(service);
   client_to_service[client] = service;
   service_to_clients[service][client] = TRUE;
}

mixed query_service(mixed client) {
   return client_to_service[client];
}

mixed *query_clients(mixed service) {
   MkService(service);
   return map_indices(service_to_clients[service]);
}
