#include "skel.h"
#include "queue.h"
struct route_table_entry{
	uint32_t prefix;
	uint32_t next_hop;
	uint32_t mask;
	int interface;
};

typedef struct arptable{
	uint32_t ip;
	uint8_t mac[6];
	int router;
}*arptable;

struct route_table_entry *rt;
int rtable_size;
arptable arp;
int arpsize = 0;
void parse_rtable(){
	char buff[256];
	char nrlinii[256];
	int nr = 0;
	int i = 0;
	FILE* f1 = fopen("rtable.txt","r");
	FILE* f2 = fopen("rtable.txt","r");

	while(fgets(nrlinii,256,f2)){
		nr++;
	}
	rtable_size = nr;
	rt = malloc((nr+1)*sizeof(struct route_table_entry));

	while(fgets(buff, 256, f1)) {
		char *token;
		int caz = 0;
		token = strtok(buff," ");
    		while(token != NULL){
    			if(caz == 0){
    				char* ip_addr = token;
    				uint32_t ipp =inet_addr(ip_addr);
    				rt[i].prefix = ipp;
    				caz++;
    			}
    			else if(caz == 1){
    				char* next_hop = token;
    				uint32_t hop =inet_addr(next_hop);
    				rt[i].next_hop = hop;
    				caz++;
    			}
    			else if(caz == 2){
    				char* mask = token;
    				uint32_t maskk =inet_addr(mask);
    				rt[i].mask = maskk;
    				caz++;
    			}
    			else if(caz == 3){
    				rt[i].interface = atoi(token);
    				caz++;
    			}
    			token = strtok(NULL," ");
    		}
    		i++;
	}	
}

struct route_table_entry *get_best_route(__u32 dest_ip) {
	int max = -1;
	int j = -1;
	for(int i = 0; i<rtable_size; i++){
			if((rt[i].mask & dest_ip) == (rt[i].mask & rt[i].prefix)){
			if(max < __builtin_popcount(rt[i].mask))
				{

					max = __builtin_popcount(rt[i].mask);
					j = i;
				}	
			}
	
	}
	if(j == -1)
		return NULL;
	return &rt[j];
}
 arptable  get_arp_entry(uint32_t ip) {
    int j = -1;
    for(int i = 0; i<arpsize; i++){
    	if(arp[i].ip == ip){
    		j = i;
    	}
    }

    if(j == -1)
    return NULL;
	else return &arp[j];
}
void init_arp_table(){//am introdus datele routerului in tabela arp pentru o cautare mai usoara
	arpsize = 4;
	arp = malloc(arpsize*sizeof(struct arptable));
	arp[0].ip = inet_addr(get_interface_ip(0)); 
	get_interface_mac(0,arp[0].mac);
	arp[0].router = 1;
	arp[1].router = 1;
	arp[2].router = 1;
	arp[3].router = 1;
	arp[1].ip = inet_addr(get_interface_ip(1)); 
	get_interface_mac(1,arp[1].mac);
	arp[2].ip = inet_addr(get_interface_ip(2)); 
	get_interface_mac(2,arp[2].mac);
	arp[3].ip = inet_addr(get_interface_ip(3)); 
	get_interface_mac(3,arp[3].mac);
}
void update_arp_table(uint8_t *ip,uint8_t *mac){//realoc memoria pentru tabela arp si introduc datele noi
	arpsize++;
	arp = realloc(arp,arpsize*sizeof(struct arptable));
	memcpy(&arp[arpsize-1].ip,ip,4);
	memcpy(arp[arpsize-1].mac,mac,6);
	arp[arpsize-1].router = 0;
}

void init_packet(packet *pkt) {//initializez un pachet cu 0
	memset(pkt->payload, 0, 1600);
	pkt->len = 0;
}


int main(int argc, char *argv[])
{
	queue q = queue_create () ;
	packet m;
	int rc;
	init();
	setvbuf ( stdout , NULL , _IONBF , 0) ;
    parse_rtable();
	init_arp_table();
	while (1) {
		rc = get_packet(&m);
		DIE(rc < 0, "get_message");
		struct ether_header *eth_hdr = (struct ether_header *)m.payload;
		/* Students will write code here */
		if(ntohs(eth_hdr->ether_type) == ETHERTYPE_ARP ){//verific daca pachetul e arp si am 2 variante
			struct ether_arp* eth_arp = (struct ether_arp *)(m.payload + sizeof(struct ether_header) );
			if(ntohs(eth_arp->arp_op) == ARPOP_REQUEST){//daca e arp request raspund cu arp reply cu adresa mea mac
				packet m_reply;
				init_packet(&m_reply);
				m_reply.len = sizeof(struct ether_header) + sizeof(struct ether_arp);
				struct ether_header *eth_hdr_reply = (struct ether_header *)(m_reply.payload);
				struct ether_arp* eth_arp_reply = (struct ether_arp *)(m_reply.payload +sizeof(struct ether_header));
				eth_hdr_reply->ether_type = htons(ETHERTYPE_ARP);
					eth_arp_reply->arp_op = htons(ARPOP_REPLY);
				 	eth_arp_reply->arp_hrd = htons(ARPHRD_ETHER);
				 	 eth_arp_reply->arp_pro = eth_arp->arp_pro;
				 	eth_arp_reply->arp_hln = 0x06;
				 	eth_arp_reply->arp_pln = 0x04;
				 memcpy(eth_hdr_reply->ether_dhost,eth_hdr->ether_shost,6);
				get_interface_mac(m.interface,eth_hdr_reply->ether_shost);  
				get_interface_mac(m.interface,eth_arp_reply->arp_sha);
				uint32_t ina;
			   ina	= inet_addr(get_interface_ip(m.interface));
				 memcpy(eth_arp_reply->arp_spa ,&ina,4);
				 memcpy(eth_arp_reply->arp_tha ,eth_arp->arp_sha,6);
				 memcpy(eth_arp_reply->arp_tpa ,eth_arp->arp_spa,4);
				m_reply.interface=m.interface;
				send_packet(m.interface,&m_reply);
			}
			


			else if(ntohs(eth_arp->arp_op) == ARPOP_REPLY){//daca e arp reply scot pachetul din coada updatez tabela arp si trimit pachetul la destinatia corespunzatoare
				update_arp_table(eth_arp->arp_spa,eth_arp->arp_sha);
				packet *m_final = queue_deq(q);
				struct ether_header *eth_hdr_final = (struct ether_header *)m_final->payload;
				memcpy(eth_hdr_final->ether_dhost,eth_hdr->ether_shost,6);
				get_interface_mac(m.interface,eth_hdr_final->ether_shost);
				m_final->interface = m.interface;
				send_packet(m_final->interface,m_final);
				continue;
			}
	


  }
  else if(ntohs(eth_hdr->ether_type) == ETHERTYPE_IP){//cazul cand e pachet icmp
		packet m_request;
		packet m_copie;
		init_packet(&m_copie);
		struct ip *ip_hdr = (struct ip *)(m.payload + sizeof(struct ether_header));
		struct icmphdr *icmp_hdr = (struct icmphdr *)(m.payload + sizeof(struct ether_header) + sizeof(struct ip));
		arptable ar  = get_arp_entry(ip_hdr->ip_dst.s_addr);
			uint32_t num;
			num = ip_hdr->ip_dst.s_addr;
			struct route_table_entry*route = get_best_route(num);
			   if(checksum(ip_hdr,sizeof(struct ip)) != 0)
			   continue;

			 if(ip_hdr->ip_ttl <= 1){//daca ttl <=1 pachetul e expirat si raspund cu time exceeded
				init_packet(&m_request);
				m_request.interface = m.interface;
				m_request.len = sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct icmphdr) ;
				struct ether_header *eth_hdr_reply = (struct ether_header *)(m_request.payload );
				struct ip *ip_hdr_reply = (struct ip *)(m_request.payload + sizeof(struct ether_header));
				struct icmphdr *icmp_hdr_reply = (struct icmphdr *)(m_request.payload + sizeof(struct ether_header) + sizeof(struct ip));
				eth_hdr_reply->ether_type = htons(ETHERTYPE_IP);
				get_interface_mac(m.interface,eth_hdr_reply->ether_shost);
				memcpy(eth_hdr_reply->ether_dhost,eth_hdr->ether_shost,6);
				ip_hdr_reply->ip_v = 4; 
				ip_hdr_reply->ip_hl = 5; 
				ip_hdr_reply->ip_ttl = 64;
				ip_hdr_reply->ip_p = IPPROTO_ICMP;
				ip_hdr_reply->ip_id = htons(getpid() & 0xFFFF); 
				ip_hdr_reply->ip_dst.s_addr = ip_hdr->ip_src.s_addr;
				ip_hdr_reply->ip_len = htons(sizeof(struct ip) + sizeof(struct icmphdr));
				ip_hdr_reply->ip_sum = checksum(ip_hdr_reply, sizeof(struct ip));
				icmp_hdr_reply->code = 0;
				icmp_hdr_reply->type = 11;
				icmp_hdr_reply->un.echo.id = htons(getpid() & 0xFFFF);
				icmp_hdr_reply->checksum = 0;
				icmp_hdr_reply->checksum = checksum(icmp_hdr_reply, sizeof(struct icmphdr));
				send_packet(m.interface,&m_request);
				continue;
			}



			 else if(route == NULL){//host unreachable
				init_packet(&m_request);
				m_request.interface = m.interface;
				m_request.len = sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct icmphdr) ;
				struct ether_header *eth_hdr_reply = (struct ether_header *)(m_request.payload );
				struct ip *ip_hdr_reply = (struct ip *)(m_request.payload + sizeof(struct ether_header));
				struct icmphdr *icmp_hdr_reply = (struct icmphdr *)(m_request.payload + sizeof(struct ether_header) + sizeof(struct ip));
				eth_hdr_reply->ether_type = htons(ETHERTYPE_IP);
				get_interface_mac(m.interface,eth_hdr_reply->ether_shost);
				memcpy(eth_hdr_reply->ether_dhost,eth_hdr->ether_shost,6);
				ip_hdr_reply->ip_v = 4; 
				ip_hdr_reply->ip_hl = 5;
				ip_hdr_reply->ip_ttl = 64;
				ip_hdr_reply->ip_p = IPPROTO_ICMP;
				ip_hdr_reply->ip_id = htons(getpid() & 0xFFFF);
				ip_hdr_reply->ip_dst.s_addr = ip_hdr->ip_src.s_addr;	
				ip_hdr_reply->ip_len = htons(sizeof(struct ip) + sizeof(struct icmphdr));
				ip_hdr_reply->ip_sum = checksum(ip_hdr_reply, sizeof(struct ip));
				icmp_hdr_reply->code = 0;
				icmp_hdr_reply->type = 3;
				icmp_hdr_reply->un.echo.id = htons(getpid() & 0xFFFF);
				icmp_hdr_reply->checksum = 0;
				icmp_hdr_reply->checksum = checksum(icmp_hdr_reply, sizeof(struct icmphdr));
				send_packet(m.interface,&m_request);
				continue;
			}


				ip_hdr->ip_ttl--;

		if(ar == NULL ){//nu am intrare in tabela arp deci o sa trimit un arp request
				init_packet(&m_request);//m request va fi pachetul arp request
			   	ip_hdr->ip_sum = 0;
			   	ip_hdr->ip_sum = checksum(ip_hdr, sizeof(struct ip));
				m_request.len = sizeof(struct ether_header) + sizeof(struct ether_arp) ;
				struct ether_header *eth_hdr_reply = (struct ether_header *)(m_request.payload );
				struct ether_arp* eth_arp_reply = (struct ether_arp *)(m_request.payload +sizeof(struct ether_header) );
					eth_hdr_reply->ether_type = htons(ETHERTYPE_ARP);
					eth_arp_reply->arp_op = htons(ARPOP_REQUEST);
				 	eth_arp_reply->arp_hrd = htons(ARPHRD_ETHER);
				 	eth_arp_reply->arp_pro = htons(0x0800);
				 	eth_arp_reply->arp_hln = 0x06;
				 	eth_arp_reply->arp_pln = 0x04;
				 memset(eth_hdr_reply->ether_dhost,0xff,6);
				get_interface_mac(route->interface,eth_hdr_reply->ether_shost);  
				get_interface_mac(route->interface,eth_arp_reply->arp_sha);
				uint32_t ina;
			   ina	= inet_addr(get_interface_ip(route->interface));
				memcpy(eth_arp_reply->arp_spa ,&ina,4);
				 memset(eth_arp_reply->arp_tha ,0x00,6);
				 memcpy(eth_arp_reply->arp_tpa ,&ip_hdr->ip_dst.s_addr,4);
				printf("Se trimite pachet 2\n");
				m_request.interface=route->interface;
				memcpy(&m_copie,&m,sizeof(m));//in m_copie copiez vechiul pachet si il bag in coada
				queue_enq (q ,&m_copie) ;
				send_packet(route->interface,&m_request);	
	}

				

	else if (ar != NULL){// daca am intrare in tabela arp pot face forwarding
				if(ar->router == 1){//daca pachetul este router echo
				if(icmp_hdr->type == 8){
					memcpy(eth_hdr->ether_dhost,eth_hdr->ether_shost,6);
					get_interface_mac(m.interface,eth_hdr->ether_shost);
					ip_hdr->ip_p = IPPROTO_ICMP;
					uint32_t aux = ip_hdr->ip_src.s_addr;
					ip_hdr->ip_src.s_addr = ip_hdr->ip_dst.s_addr;
					ip_hdr->ip_dst.s_addr = aux;
					ip_hdr->ip_off = 0;
					ip_hdr->ip_sum = 0;
					ip_hdr->ip_sum = checksum(ip_hdr, sizeof(struct ip));
					icmp_hdr->code = 0;
					icmp_hdr->type = 0;
					icmp_hdr->checksum = 0;
					icmp_hdr->checksum = checksum(icmp_hdr, sizeof(struct icmphdr));
					send_packet(m.interface,&m);
					}
					continue;
				}
				get_interface_mac(route->interface,eth_hdr->ether_shost);
				m.interface = route->interface;
				ip_hdr->ip_dst.s_addr = ar->ip;
				ip_hdr->ip_sum = 0;
			   	ip_hdr->ip_sum = checksum(ip_hdr, sizeof(struct ip));
			    memcpy(eth_hdr->ether_dhost,ar->mac,6);
				send_packet(route->interface,&m);
				continue;
	}

 }
}
}
