/*
  There are many things to fix:
- octet_string type: should have a maximum size allocated instead of being a pointer
- many typedefs incomplete (default to integer)
- 3gpp... names changed to tgpp...
-
- CHECK the list elements: lenght (1 or more octets) + sequence of elements
- Current approach:
  - make the list into a vector (pointer to data type)
  - use the length value just when packing and unpacking the PDUs
*/

#ifndef MIH_H
#define MIH_H

// RFC 5677
// ieee-mih   4551/tcp   MIH Services
// ieee-mih   4551/udp   MIH Services
#define MIHF_PORT        4551
#define MIHF_MOD_PORT    4551


#define MIH_HLEN	   8      // Total octets in header
#define MIH_PDU_LEN	65535  // Max. octets in payload (16 bits lenght)
#define MIH_ID_LEN   253    // Max. octetc in ID


/*
// attempts to define a list structure... no good...
#define LIST(type)               \
	struct {                      \
		unsigned char length[3];	\
		type elem[1];              \
	}
#define LIST(type)     \
	struct {            \
		type elem;       \
		type *next;      \
	}
*/

// review the 2 types below: bad...
typedef struct { unsigned char data[3]; } u24;
typedef long double u128;

// Maximum TID value (12 bits)
#define MAX_TID 0x0fff

// Information element identifier values (G.1, pp. 254)
// Information element identifier values for different individual IEs and IE containers
#define IE_NETWORK_TYPE               0x10000000
#define IE_OPERATOR_ID                0x10000001
#define IE_SERVICE_PROVIDER_ID        0x10000002
#define IE_COUNTRY_CODE               0x10000003
#define IE_NETWORK_ID                 0x10000100
#define IE_NEWORK_AUX_ID              0x10000101
#define IE_ROAMING_PARTNERS           0x10000102
#define IE_COST                       0x10000103
#define IE_NETWORK_QOS                0x10000105
#define IE_NETWORK_DATA_RATE          0x10000106
#define IE_NET_REGULAT_DOMAIN         0x10000107
#define IE_NET_FREQUENCY_BANDS        0x10000108
#define IE_NET_IP_CFG_METHODS         0x10000109
#define IE_NET_CAPABILITIES           0x1000010A
#define IE_NET_SUPPORTED_LCP          0x1000010B
#define IE_NET_MOB_MGMT_PROT          0x1000010C
#define IE_NET_EMSERV_PROXY           0x1000010D
#define IE_NET_IMS_PROXY_CSCF         0x1000010E
#define IE_NET_MOBILE_NETWORK         0x1000010F
#define IE_POA_LINK_ADDR              0x10000200
#define IE_POA_LOCATION               0x10000201
#define IE_POA_CHANNEL_RANGE          0x10000202
#define IE_POA_SYSTEM_INFO            0x10000203
#define IE_POA_SUBNET_INFO            0x10000204
#define IE_POA_IP_ADDR                0x10000205
#define IE_CONTAINER_LIST_OF_NETWORKS 0x10000300
#define IE_CONTAINER_NETWORK          0x10000301
#define IE_CONTAINER_POA              0x10000302

#define SUCCESS                0
#define UNSPECIFIED_FAILURE    1
#define REJECTED               2
#define AUTHORIZATION_FAILURE  3
#define NETWORK_ERROR          4

// #define FALSE                  0
// #define TRUE                   1

// type of operation to be performed with respect to the SID
#define REQUEST      1
#define RESPONSE     2
#define INDICATION   3


// Action identifiers (AID) for MIH messages. Table L.1 (p. 289)
// Conflicting types with MIH Capabilities (p. 249, 250) ...
// MIH messages for Service Management
#define MIH_CAPABILITY_DISCOVER       1
#define MIH_REGISTER                  2
#define MIH_DEREGISTER                3
#define MIH_EVENT_SUBSCRIBE           4
#define MIH_EVENT_UNSUBSCRIBE         5

// MIH messages for Event Service
#define MIH_LINK_DETECTED             1
#define MIH_LINK_UP                   2
#define MIH_LINK_DOWN                 3
#define MIH_LINK_PARAMETERS_REPORT    5
#define MIH_LINK_GOING_DOWN           6
#define MIH_LINK_HANDOVER_IMMINENT    7
#define MIH_LINK_HANDOVER_COMPLETE    8

// MIH messages for Command Service
#define MIH_LINK_GET_PARAMETERS       1
#define MIH_LINK_CONFIGURE_THRESHOLDS 2
#define MIH_LINK_ACTIONS              3
#define MIH_NET_HO_CANDIDATE_QUERY    4
#define MIH_MN_HO_CANDIDATE_QUERY     5
#define MIH_N2N_HO_QUERY_RESOURCES    6
#define MIH_MN_HO_COMMIT              7
#define MIH_NET_HO_COMMIT             8
#define MIH_N2N_HO_COMMIT             9
#define MIH_MN_HO_COMPLETE           10
#define MIH_N2N_HO_COMPLETE          11

// MIH messages for Information Service
#define MIH_GET_INFORMATION           1
#define MIH_PUSH_INFORMATION          2

// A list of IS query types. Bitmap values:
#define BINARY_DATA_CAP                    1 << 0
#define RDF_DATA_CAP                       1 << 1
#define RDF_SCHEMA URL_CAP                 1 << 2
#define RDF_SCHEMA_CAP                     1 << 3
#define IE_NETWORK_TYPE_CAP                1 << 4
#define IE_OPERATOR_ID_CAP                 1 << 5
#define IE_SERVICE_PROVIDER_ID_CAP         1 << 6
#define IE_COUNTRY_CODE_CAP                1 << 7
#define IE_NETWORK_ID_CAP                  1 << 8
#define IE_NEWORK_AUX_ID_CAP               1 << 9
#define IE_ROAMING_PARTNERS_CAP            1 << 10
#define IE_COST_CAP                        1 << 11
#define IE_NETWORK_QOS_CAP                 1 << 12
#define IE_NETWORK_DATE_RATE_CAP           1 << 13
#define IE_NET_REGULT_DOMAIN_CAP           1 << 14
#define IE_NET_FREQUENCY_BANDS_CAP         1 << 15
#define IE_NET_IP_CFG_METHODS_CAP          1 << 16
#define IE_NET_CAPABILITIES_CAP            1 << 17
#define IE_NET_SUPPORTED_LCP_CAP           1 << 18
#define IE_NET_MOB_MGMT_PROT_CAP           1 << 19
#define IE_NET_EMSERV_PROXY_CAP            1 << 20
#define IE_NET_IMS_PROXY_CSCF_CAP          1 << 21
#define IE_NET_MOBILE_NETWORK_CAP          1 << 22
#define IE_POA_LINK_ADDR_CAP               1 << 23
#define IE_POA_LOCATION_CAP                1 << 24
#define IE_POA_CHANNEL_RANGE_CAP           1 << 25
#define IE_POA_SYSTEM_INFO_CAP             1 << 26
#define IE_POA_SUBNET_INFO_CAP             1 << 27
#define IE_POA_IP_ADDR_CAP                 1 << 28
// Bits 29-63: (Reserved)


// MIH Link types (link_type_t) (pp 233)
// 0: Reserved
#define WIRELESS_GSM              1
#define WIRELESS_GPRS             2
#define WIRELESS_EDGE             3
#define ETHERNET                 15
#define WIRELESS_OTHER           18
#define WIRELESS_IEEE_802_11     19
#define WIRELESS_CDMA2000        22
#define WIRELESS_UMTS            23
#define WIRELESS_CDMA2000_HRPD   24
#define WIRELESS_IEEE_802_16     27
#define WIRELESS_IEEE_802_20     28
#define WIRELESS_IEEE_802_22     29

// op_namespace: types to represent a type of operator name
#define GSM_UMTS     0
#define CDMA         1
#define REALM        2
#define ITU_T_TSB    3
#define GENERAL      4
// 5-255: (Reserved)

// A type to represent the unit of a cost
#define SECOND         0
#define MINUTE         1
#define HOURS          2
#define DAY            3
#define WEEK           4
#define MONTH          5
#define YEAR           6
#define FREE           7
#define FLAT_RATE      8

/* Type values for TLV encoding (290) */
#define SRC_MIHF_ID_TLV				1  /* Source MIHF ID */
#define DST_MIHF_ID_TLV				2  /* Destination MIHF ID */
#define STATUS_TLV				3  /* Status */
#define LINK_TYPE_TLV				4  /* Link type */
#define MIH_EVENT_LIST_TLV			5  /* MIH event list */
#define MIH_COMMAND_LIST_TLV			6  /* MIH command list */
#define MIIS_QUERY_TYPE_LIST_TLV		7  /* MIIS query type list */
#define TRANSPORT_OPTION_LIST_TLV		8  /* Transport option list */
#define LINK_ADDRESS_LIST_TLV			9
#define MBB_HO_SUPP_TLV				10  /* MBB handover support */
#define REG_REQUEST_CODE_TLV			11  /* Register request code */
#define VALID_TIME_INTERVAL_TLV			12  /* Valid time interval */
#define LINK_IDENTIFIER_TLV			13
#define NEW_LINK_IDENTIFIER_TLV			14
#define OLD_LINK_IDENTIFIER_TLV			15
#define NEW_ACCESS_ROUTER_TLV			16
#define IP_RENEWAL_FLAG_TLV			17
#define MOBILITY_MGMT_SUPP_TLV			18
#define IP_ADDR_CONFIG_MTHDS_TLV		19
#define LINK_DOWN_REASON_CODE_TLV		20
#define TIME_INTERVAL_TLV			21
#define LINK_GOING_DOWN_REASON_TLV		22
#define LINK_PARAMETER_REPORT_LIST_TLV		23
#define DEVICE_STATES_REQUEST_TLV		24
#define LINK_IDENTIFIER_LIST_TLV		25
#define DEVICE_STATES_RESPONSE_LIST_TLV		26
#define GET_STATUS_REQUEST_SET_TLV		27
#define GET_STATUS_RESPONSE_LIST_TLV		28
#define CONFIGURE_REQUEST_LIST_TLV		29
#define CONFIGURE_RESPONSE_LIST_TLV		30
#define LIST_OF_LINK_POA_LIST_TLV		31
#define PREFERRED_LINK_LIST_TLV			32
#define HO_RESOURCE_QUERY_LIST_TLV		33
#define HO_STATUS_TLV				34
#define ACCESS_ROUTER_ADDRESS_TLV		35
#define DHCP_SERVER_ADDRESS_TLV			36
#define FA_ADDRESS_TLV				37
#define LINK_ACTIONS_LIST_TLV			38
#define LINK_ACTIONS_RESULT_LIST_TLV		39
#define HO_RESULT_TLV				40
#define RESOURCE_STATUS_TLV			41
#define RESOURCE_RETENTION_STATUS_TLV		42
#define INFO_QUERY_BINARY_DATA_LIST_TLV		43
#define INFO_QUERY_RDF_DATA_LIST_TLV		44
#define INFO_QUERY_RDF_SCHEMA_URL_TLV		45
#define INFO_QUERY_RDF_SCHEMA_LIST_TLV		46
#define MAX_RESPONSE_SIZE_TLV			47
#define INFO_RESPONSE_BINARY_DATA_LIST_TLV	48
#define INFO_RESPONSE_RDF_DATA_LIST_TLV		49
#define INFO_RESPONSE_RDF_SCHEMA_URL_LIST_TLV	50
#define INFO_RESPONSE_RDF_SCHEMA_LIST_TLV	51
#define MOBILE_NODE_MIHF_ID_TLV			52
#define QUERY_RESOURCE_REPORT_FLAG_TLV		53
#define EVENT_CONFIGURATION_INFO_LIST_TLV	54
#define TARGET_NETWORK_INFO_TLV			55
#define LIST_OF_TARGET_NETWORK_INFO_TLV		56
#define ASSIGNED_RESOURCE_SET_TLV		57
#define LINK_DETECTED_INFO_LIST_TLV		58
#define MN_LINK_ID_TLV				59
#define POA_TLV					60
#define UNAUTHENTICATED_INFO_REQUEST_TLV	61
#define NETWORK_TYPE_TLV			62
#define REQUESTED_RESOURCE_SET_TLV		63
/* Reserved TLVs				64-99 */
/* Vendor Specific TLV				100 */
/* Reserved for exeprimental TLVs		101-255 */

// Link down reason codes (235)
#define LD_EXPLICIT_DISCONNECT       0
#define LD_PACKET_TIMEOUT            1
#define LD_NO_RESOURCE               2
#define LD_NO_BROADCAST              3
#define LD_AUTHENTICATION_FAILURE    4
#define LD_BILLING_FAILURE           5
// 6-127 (Reserved)
// 128-255 Vendor specific reason codes

// Link going down reason codes (235)
#define LGD_EXPLICIT_DISCONNECT       0
#define LGD_LINK_PAR_DEGRAGING        1
#define LGD_LOW_POWER                 2
#define LGD_NO_RESOURCE               3
// 4-127 (Reserved)
// 128-255 Vendor specific reason codes


// Non-null terminated string whose lenght shall not exceed 253 octets
typedef unsigned char * network_id_t;	// octet_string (240)
// typedef unsigned char network_id_t[254];	// octet_string (240)


typedef uint8_t op_mode_t; // unsigned_int(1)

typedef uint8_t cos_id_t; // unsigned_int(1)

typedef struct {
	cos_id_t cos_id;
	uint16_t val;
} num_cos_types_t; // sequence(cos_id,unsigned_int(2))
typedef struct {
	cos_id_t cos_id;
	uint16_t val;
} min_pk_tx_delay_t; // sequence(cos_id,unsigned_int(2))
typedef struct {
	cos_id_t cos_id;
	uint16_t val;
} avg_pk_tx_delay_t; // sequence(cos_id,unsigned_int(2))
typedef struct {
	cos_id_t cos_id;
	uint16_t val;
} max_pk_tx_delay_t; // sequence(cos_id,unsigned_int(2))
typedef struct {
	cos_id_t cos_id;
	uint16_t val;
} pk_delay_jitter_t; // sequence(cos_id,unsigned_int(2))
typedef struct {
	cos_id_t cos_id;
	uint16_t val;
} pk_loss_rate_t; // sequence(cos_id,unsigned_int(2))

typedef struct {
	num_cos_types_t num_cos_types;
	// LIST(min_pk_tx_delay_t) * min_pk_tx_delay;
	min_pk_tx_delay_t * min_pk_tx_delay;
	// LIST(avg_pk_tx_delay_t) * avg_pk_tx_delay;
	avg_pk_tx_delay_t * avg_pk_tx_delay;
	// LIST(max_pk_tx_delay_t) * max_pk_tx_delay;
	max_pk_tx_delay_t * max_pk_tx_delay;
	// LIST(pk_delay_jitter_t) * pk_delay_jitter;
	pk_delay_jitter_t * pk_delay_jitter;
	// LIST(pk_loss_rate_t) * pk_loss_rate;
	pk_loss_rate_t * pk_loss_rate;
} qos_list_t;
// (236) sequence(num_cos_types,list(min_pk_tx_delay),list(avg_pk_tx_delay),
//          list(max_pk_tx_delay),list(pk_delay_jitter),list(pk_loss_rate))

// Indicates the supported mobility management protocols
// Bit 0: Mobile IPv4 (IETF RFC 3344)
// Bit 1: Mobile IPv4 Regional Registration (IETF RFC 4857)
// Bit 2: Mobile IPv6 (IETF RFC 3775)
// Bit 3: Hierarchical Mobile IPv6 (IETF RFC 4140)
// Bit 4: Low Latency Handoffs (IETF RFC 4881)
// Bit 5: Mobile IPv6 Fast Handovers (IETF RFC 5268)
// Bit 6: IKEv2 Mobility and Multihoming Protocol (IETF RFC 4555)
// Bit 7-15: (Reserved)
typedef uint16_t ip_mob_mgmt_t; // bitmap(16) (239)

// typedef uint32_t *freq_bands_t; // list(unsigned_int(4)) (241)
// typedef LIST(uint32_t) *freq_bands_t; // list(unsigned_int(4)) (241)
typedef uint32_t *freq_bands_t; // list(unsigned_int(4)) (241)

typedef uint32_t net_caps_t; // bitmap(32) (pp.242)
// Bitmap Values:
// Bit 0: Security - Indicates that some level of security is supported when set.
// Bit 1: QoS Class0 - Indicates that QoS for class 0 is supported when set.
// Bit 2: QoS Class1 - Indicates that QoS for class 1 is supported when set.
// Bit 3: QoS Class2 - Indicates that QoS for class 2 is supported when set.
//     Otherwise, no QoS for class 2 support is available.
// Bit 4: QoS Class3 - Indicates that QoS for class 3 is supported when set.
//     Otherwise, no QoS for class 3 support is available.
// Bit 5: QoS Class4 - Indicates that QoS for class 4 is supported when set.
//     Otherwise, no QoS for class 4 support is available.
// Bit 6: QoS Class5 - Indicates that QoS for class 5 is supported when set.
//     Otherwise, no QoS for class 5 support is available.
// Bit 7: Internet Access - Indicates that Internet access is supported when set.
//     Otherwise, no Internet access support is available.
// Bit 8: Emergency Services - Indicates that some level of emergency services is supported when set.
//     Otherwise, no emergency service support is available.
// Bit 9: MIH Capability - Indicates that MIH is supported when set.
//     Otherwise, no MIH support is available.
// Bit 10-31: (Reserved)


typedef uint16_t cntry_code_t; // octet(2) (ISO 3166-1) (240)

// A type to represent a regulatory domain.
typedef struct {
	cntry_code_t cntry_code;
	uint8_t class;
} regu_domain_t; // sequence(cntry_code,unsigned_int(1))

// A type represent supported Location Configuration Protocol (LCP) (pp 244)
typedef uint8_t supported_lcp_t; // unsigned_int(1)

typedef uint8_t * type_ext_t; // octet_string

typedef uint16_t pilot_pn_t; // integer(2)

typedef uint32_t ip4_addr_t; // octet(4)

typedef u128 ip6_addr_t; // octet(16)

// rfc2181: A full domain name is limited to 255 octets
typedef unsigned char * fqdn_t; // octet_string

typedef union {
	ip4_addr_t ip4_addr;
	ip6_addr_t ip6_addr;
	fqdn_t fqdn;
} proxy_addr_t; // choice(ip4_addr,ip6_addr,fqdn)

typedef uint16_t freq_id_t; // integer(2)

typedef uint16_t base_id_t; // unsigned_int(2) (240)

typedef uint8_t band_class_t; // unsigned_int(1)

// sys_params_t // sequence(base_id,pilot_pn,freq_id,band_class))
typedef struct {
	base_id_t base_id;
	pilot_pn_t pilot_pn;
	freq_id_t freq_id;
	band_class_t band_class;
} sys_params_t;

typedef uint64_t subtype_t; // bitmap(64) (245)

typedef uint8_t link_type_t; // unsigned_int(1) (233)

// sequence(choice(null,link_type),choice(null,subtype),choice(null,type_ext)) (pp 242)
typedef struct {
	link_type_t link_type;
	subtype_t subtype;
	type_ext_t type_ext;
} network_type_t;

typedef uint16_t fq_code_num_t; // integer(2) (241)

typedef uint32_t cell_id_t;	// unsigned_int(4)

typedef struct {
	cell_id_t cell_id;
	fq_code_num_t fq_code_num;
} sib_t; // sequence(cell_id,fq_code_num)

typedef struct {
	uint16_t ttg;
	uint8_t rtg;
} gap_t; // sequence(unsigned_int(2),unsigned_int(1)) (241)

typedef uint32_t data_rate_t; // unsigned_int(4) (241)
// maximum data rate in kb/s (0..2^32 -1)

typedef uint16_t eirp_t; // integer(1) (241)

typedef unsigned char * down_bp_t; // bitmap(256) (241)

typedef unsigned char * up_bp_t; // bitmap(256) (244)

typedef struct {
	down_bp_t down_bp;
	up_bp_t up_bp;
} burst_prof_t; // sequence(down_bp,up_bp) (240)

typedef uint16_t bandwidth_t; // unsigned_int(2) (kb/s) (240)

typedef unsigned char du_ctr_freq_t; // integer(8)

typedef unsigned char init_code_t; // integer(1)

typedef unsigned char ho_code_t; // integer(1)

typedef struct {
	init_code_t init_code;
	ho_code_t ho_code;
} cdma_codes_t; // sequence(init_code,ho_code)

typedef struct {
	base_id_t base_id;
	bandwidth_t bandwidth;
	du_ctr_freq_t du_ctr_freq;
	eirp_t eirp;
	gap_t gap;
	burst_prof_t burst_prof;
	cdma_codes_t cdma_codes;
} dcd_ucd_t; // sequence(base_id,bandwidth,du_ctr_freq,eirp,gap,burst_prof,cdma_codes)

typedef union {
	dcd_ucd_t dcd_ucd;       // DCD_UCD: IEEE 802.16
	sib_t sib;               // SIB: UMTS
	sys_params_t sys_params; // SYS_PARAMS: cdma2000
} parameters_t; // choice(dcd_ucd,sib_sys_params) (243)

// A type to represent an auxiliary access network identifier.
// This is HESSID if network type is IEEE 802.11
typedef unsigned char *net_aux_id_t;	// octec_string (240)

typedef uint8_t cost_unit_t; // unsigned_int(1) (240)
// 0: second
// 1: minute
// 2: hours
// 3: day
// 4: week
// 5: month
// 6: year
// 7: free
// 8: flat rate
// 9-255: (Reserved)

// op_namespace: types to represent a type of operator name
typedef uint8_t op_namespace_t;	// unsigned_int(1)

// Non-null terminated string whose lenght shall not exceed 253 octets
typedef unsigned char * op_name_t;	// octet_string // not to exceed 253 octets.

typedef struct {
	op_name_t op_name;
	op_namespace_t op_namespace;
} operator_id_t; // sequence(op_name,op_namespace)

// should we allocate memory for lists?
// typedef operator_id_t * roaming_ptns_t; // list(operator_id) (pp. 243)
// typedef LIST(operator_id_t) *roaming_ptns_t; // list(operator_id) (pp. 243)
typedef operator_id_t *roaming_ptns_t; // list(operator_id) (pp. 243)

typedef unsigned char *sp_id_t; // octet_string // not to exceed 253 octets.

typedef struct {
	uint32_t val_int;  // integer part of the cost
	uint16_t val_frac; // fraction part of the cost ([0,999])
} cost_value_t; //sequence(unsigned_int(4),unsigned_int(2)) (240)

typedef u24 cost_curr_t; // octet(3)
// The currency of a cost. Three-letter currency code (e.g, USD) ISO 4217. (240)

typedef struct {
	cost_unit_t unit;
	cost_value_t value;
	cost_curr_t curr;
} cost_t;

typedef enum {
	FALSE = 0,
	TRUE = 1,
} boolean_t; // BOOLEAN ENUMERATED


typedef uint8_t batt_level_t; // integer(1) -1..100 (-1 indicates battery level unknown)

typedef uint16_t channel_id_t; // unsigned_int(2) (valid range: 0..65535)

typedef boolean_t config_status_t;

typedef uint8_t * device_info_t; // octet string

typedef uint8_t ip_prefix_len_t;	// unsigned_int(1) (239)

// (225)
typedef struct {
	uint16_t addr_type;	// (1) unsigned_int(2)
	uint8_t * addr;      // octet_string
} transport_addr_t;
// (1) http://www.iana.org/assignments/address-family-numbers
// 1  IP (IP version 4)
// 2  IP6 (IP version 6)
// ...
// 6  802 (includes all 802 media plus Ethernet "canonical format")

typedef transport_addr_t mac_addr_t;

typedef transport_addr_t ip_addr_t;

typedef ip_addr_t dhcp_serv_t;

typedef struct {
	ip_prefix_len_t ip_prefix_len;
	ip_addr_t ip_addr;
} ip_subnet_info_t; // sequence(ip_prefix_len,ip_addr)

typedef ip_addr_t fn_agnt_t; // ip_addr (252)
typedef ip_addr_t acc_rtr_t; // ip_addr (252)

typedef uint32_t ip_cfg_mthds_t; //bitmap(32) (239)

typedef struct {
	ip_cfg_mthds_t ip_cfg_mthds;
	dhcp_serv_t dhcp_serv;
	fn_agnt_t fn_agnt;
	acc_rtr_t acc_rtr;
} ip_config_t;
// sequence(ip_cfg_mthds,choice(null,dhcp_serv),
//          choice(null,fn_agnt),choice(null,acc_rtr))) (pp 241)


typedef uint16_t ci_t; // octet(2)

typedef uint16_t lac_t; // octet(2)

typedef u24 plmn_id_t; // octet(3)

typedef struct {
	plmn_id_t plmn_id;
	lac_t lac;
	ci_t ci;
} tgpp_2g_cell_id_t; // 3gpp_2g_cell_id_t // sequence(plmn_id,lac,ci)

typedef struct {
	plmn_id_t  plmn_id;
	cell_id_t  cell_id;
} tgpp_3g_cell_id_t; // 3gpp_3g_cell_id_t // sequence(plmn_id,cell_id)

typedef uint8_t * tgpp_addr_t; // 3gpp_addr_t // octet_string

typedef uint8_t * tgpp2_addr_t; // 3gpp2_addr_t // octet_string

typedef uint8_t * other_l2_addr_t;  // octet_string

typedef union {
	mac_addr_t         mac_addr;
	tgpp_3g_cell_id_t  tgpp_3g_cell_id;
	tgpp_2g_cell_id_t  tgpp_2g_cell_id;
	tgpp_addr_t        tgpp_addr;
	tgpp2_addr_t       tgpp2_addr;
	other_l2_addr_t    other_l2_addr;
} link_addr_t;

typedef struct {
	network_type_t  network_type;
	link_addr_t     link_addr;
	parameters_t    parameters;
} system_info_t; // sequence(network_type,link_addr,choice(null,parameters))

typedef struct {
	link_type_t  link_type;
	link_addr_t  link_addr;
} link_id_t;		// sequence(link_type,link_addr) (229)

typedef enum {
	L2 = 0,
	L3 = 1
} transport_type_t;

// The MIHF Identifier. Maximum length: 253 characters, but limmited
// to 63 octets in some protocols that also use NAI identifiers
typedef unsigned char *mihf_id_t; // octet_string (248)
// should we make it "unsigned char mihf_id_t[MIH_ID_LEN]" ?
// MIHF_ID is a network access identifier (NAI). If L3 communication is used
// and MIHF entity resides in the network node, then MIHF_ID is the fully
// qualified domain name or NAI-encoded IP address (IP4_ADDR or IP6_ADDR)
// of the entity that hosts the MIH Services.

// define a maximum lenght?
typedef unsigned char *civic_addr_t; // octet_string (237) (rfc 4119)

typedef struct {
	cntry_code_t cntry_code;
	civic_addr_t civic_addr;
} bin_civic_loc_t; // sequence(cntry_code,civic_addr)

typedef unsigned char * xml_civic_loc_t; // octet_string

typedef u128 bin_geo_loc_t; // octect(16)

typedef unsigned char * xml_geo_loc_t; // octect string

typedef union {
	bin_civic_loc_t bin_civic_loc;
	xml_civic_loc_t xml_civic_loc;
} civic_loc_t; // choice(bin_civic_loc,xml_civic_loc)

typedef union {
	bin_geo_loc_t bin_geo;
	xml_geo_loc_t xml_geo_loc;
} geo_loc_t; // choice(bin_geo_loc,xml_geo_loc)

typedef union {
	civic_loc_t civic_loc;
	geo_loc_t geo_loc;
	cell_id_t cell_id;
} location_t; // choice(civic_loc,geo_loc,cell_id)

typedef struct {
	uint32_t low;
	uint32_t high;
} ch_range_t; // sequence(unsigned_int(4),unsigned_int(4)) (240)

// PoA Container, containing information that depicts a PoA
typedef struct ie_container_poa {
	link_addr_t ie_poa_link_addr;
	location_t ie_poa_location;
	ch_range_t ie_poa_channel_range;
	system_info_t ie_poa_system_info;
	ip_subnet_info_t *ie_poa_subnet_info;
	ip_addr_t *ie_poa_ip_addr;
	// Vendor-specific services
	struct ie_container_poa *next;
} ie_container_poa_t;

// Access Network Container, containing information that depicts an access network
typedef struct ie_container_network {
	network_type_t      ie_network_type;
	operator_id_t       ie_operator_id;
	sp_id_t             ie_service_provider_id;
	cntry_code_t        ie_country_code;
	network_id_t        ie_network_id;
	net_aux_id_t        ie_network_aux_id;
	roaming_ptns_t      ie_roaming_partners;
	cost_t              ie_cost;
	qos_list_t          ie_network_qos;
	data_rate_t         ie_network_data_rate;
	regu_domain_t       ie_net_regulat_domain;
	freq_bands_t        ie_net_frequency_bands;
	ip_config_t         ie_net_ip_cfg_methods;
	net_caps_t          ie_net_capabilities;
	supported_lcp_t     ie_net_supported_lcp;
	ip_mob_mgmt_t       ie_net_mob_mgmt_prot;
	proxy_addr_t        ie_net_emserv_proxy;
	proxy_addr_t        ie_net_ims_proxy_cscf;
	boolean_t           ie_net_mobile_network;
	ie_container_poa_t *ie_container_poa;
	// ie_conainer_poa #1 (optional)
	// ie_conainer_poa #2 (optional)
	// ...
	// vendor specific network IE (optional)
	struct ie_container_network *next;
} ie_container_network_t;


// MIH protocol header
typedef struct {
	unsigned short int
		version         : 4,
		// (4) Version of the MIH protocol used.
	   //     0: Not to be used; 1: First version; 2-15: (Reserved)
		ackreq          : 1,
		// (1) Used for requesting an acknowledge for the message.
		ackrsp          : 1,
		// (1) Used for responding to the request for an ack for the message.
		uir             : 1,
		// (1) Unauthenticated Information Request. Used by the MIH
	   // Information Service to indicate if the protocol message is sent in pre-
		// authentication/pre-associtation state so that the lenght
		// of the response message can be limited. Should be set to '1'
		// by the originator when making an MIH information service
		// request over a certain link in the un=-associated/unalthenticated
		// or unregisterd state. In all other cases, this bit is set to '0'.
		morefragment    : 1,
		// (1) Used for indicating that the message is a fragment to be
		// followed by another fragment. It is set to '0' for a message
		// that is not fragmented and for the last fragment.
		fragmentnumber  : 7,
		// (7) Used to represent the sequence number of a fragment.
		// Set to '0' for a message that is not fragmented.
		rsvd1           : 1;
		// (1) Reserved. When not used, should be set to '0'
   unsigned short int
		// MIH Message ID (16) (SID+Opcode+AID)
		sid             : 4,
		// (4) Indentifies the different MIH services. Possible values:
		// 1: Service Management
		// 2: Event Service
		// 3: Command Service
		// 4: Information Service
		opcode          : 2,
		// (2) Type of operation to be performed with respect to the SID. Values:
		// 1: Request
		// 2: Response
		// 3: Indication
		aid             : 10;
		// (10) This indicates the action to be taken with regard to the SID
   unsigned short int
		rsvd2           : 4,
		tid             : 12;
		// (12) Used for matching Request and Responde, as well as
		// matching Request, Response and Indication to an ACK.
   unsigned short int
		payloadlength   : 16;
		// (16) The total lenght of the variable payoad embedded in
		// this MIH protocol frame. The lenght of the MIH protocol
		// header is NOT included.
} mih_header_t;

typedef struct tlv {
	struct list_head list;

	unsigned char type;
	unsigned int length; /* length semantics is dependent on TLV type  */
	void* value;
} mih_tlv_t;

typedef struct {
	mih_header_t header;
	mih_tlv_t tlvs;
} mih_message_t;


// type definitions (pp. 221,222)
// BITMAP(size) --> uint8_t var[size];
// CHOICE(DATATYPE1, DATATYPE2[,...]) --> union var { DATATYPE1 m1; DATATYPE2 m2[;...] }
// INFO_ELEMENT
// INTEGER(size)
// LIST(DATATYPE) --> DATATYPE *var;
// NULL --> void var; // ??? No octet is encoded for this data type
// OCTET(size) --> uint8_t var[size];
// SEQUENCE(DATATYPE1, DATATYPE2[,...]) --> struct { DATATYPE1 f1; DATATYPE2 f2; [...] } var;
// UNSIGNED_INT(size) --> unsigned int var [size];

// ENUMERATED UNSIGNED_INT(1)
typedef uint8_t enumerated_t;

// OCTET_STRING LIST(OCTET(1))
// PERCENTAGE UNSIGNED INT(1)

// STATUS ENUMERATED
// 0: Success, 1: Unspecified Failure, 2: Rejected, 3: Authorization FAilure, 4: Network Error
/*
typedef enum {
	SUCCESS = 0,
	UNSPECIFIED_FAILURE = 1,
	REJECTED = 2,
	AUTHORIZATION_FAILURE = 3,
	NETWORK_ERROR	= 4,
} status_t;
*/
typedef int status_t;


// Operation Code : Type of operation performed
// typedef enum {
// 	REQUEST    = 1,
// 	RESPONSE   = 2,
// 	INDICATION = 3
// } mih_opcode_t;

// Service Identifier : Identifies the different MIH services
typedef enum {
	SERVICE_MANAGEMENT  = 1,
	EVENT_SERVICE       = 2,
	COMMAND_SERVICE     = 3,
	INFORMATION_SERVICE = 4
} mih_sid_t;


typedef struct {
	uint32_t
		Link_Detected			: 1,
		Link_Up				: 1,
		Link_Down			: 1,
		Link_Parameters_Report		: 1,
		Link_Going_Down			: 1,
		Link_Handover_Imminent		: 1,
		Link_Handover_Complete		: 1,
		Link_PDU_Transmit_Status	: 1,
		Reserved			: 24;
	struct list_head list;
} link_event_list_t;

typedef struct {
	uint32_t
		Reserved			: 1,
		Link_Event_Subscribe		: 1,
		Link_Event_Unsubscribe		: 1,
		Link_Get_Parameters		: 1,
		Link_Configure_Thresholds	: 1,
		Link_Action			: 1,
		Reserved2			: 26;
	struct list_head list;
} link_cmd_list_t;

typedef struct {
	uint32_t
		MIH_Link_Detected		: 1,
		MIH_Link_Up			: 1,
		MIH_Link_Down			: 1,
		MIH_Link_Parameters_Report	: 1,
		MIH_Link_Going_Down		: 1,
		MIH_Link_Handover_Imminent	: 1,
		MIH_Link_Handover_Complete	: 1,
		MIH_Link_PDU_Transmit_Status	: 1,
		Reserved			: 24;
	struct list_head list;
} mih_evt_list_t;

typedef struct {
	uint32_t
		MIH_Link_Get_Parameters		: 1,
		MIH_Link_Configure_Thresholds	: 1,
		MIH_Link_Actions		: 1,
		MIH_Net_HO_Candidate_Query	: 1,
		/* Or is it:
                 * MIH_Net_HO_Commit
                 * MIH_N2N_HO_Query_Resources
                 * MIH_N2N_HO_Commit
                 * MIH_N2N_HO_Complete
                 */
		MIH_MN_HO_Candidate_Query	: 1,
		/* Or is it:
		 * MIH_MN_HO_Commit
		 * MIH_MN_HO_Complete
		 */
		Reserved			: 25;
	struct list_head list;
} mih_cmd_list_t;

typedef struct {
	uint64_t
		Binary_Data			: 1,
		RDF_Data			: 1,
		RDF_Schema_URL			: 1,
		RDF_Schema			: 1,
		IE_Network_Type			: 1,
		IE_Operator_ID			: 1,
		IE_Service_Provider_ID		: 1,
		IE_Country_Code			: 1,
		IE_Network_ID			: 1,
		IE_Nework_Aux_ID		: 1,
		IE_Roaming_Partners		: 1,
		IE_Cost				: 1,
		IE_Network_QoS			: 1,
		IE_Network_Date_Rate		: 1,
		IE_Net_Regult_Domain		: 1,
		IE_Net_Frequency_bands		: 1,
		IE_Net_IP_Cfg_Methods		: 1,
		IE_Net_Capabilities		: 1,
		IE_Net_Supported_LCP		: 1,
		IE_Net_Mob_Mgmt_Prot		: 1,
		IE_Net_Emserv_Proxy		: 1,
		IE_Net_IMS_Proxy_CSCF		: 1,
		IE_Net_Mobile_Network		: 1,
		IE_PoA_Link_Addr		: 1,
		IE_PoA_Location			: 1,
		IE_PoA_Channel_Range		: 1,
		IE_PoA_System_Info		: 1,
		IE_PoA_Subnet_Info		: 1,
		IE_PoA_IP_Addr			: 1,
		Reserved			: 1;
	struct list_head list;
} mih_iq_type_lst_t;

typedef struct {
	uint16_t
		UDP				: 1,
		TCP				: 1,
		Reserved			: 14;
	struct list_head list;
} mih_trans_lst_t;

typedef unsigned char reg_request_code_t;	// enumerated // unsigned_int(1)
// The registration code: 0-Registration, 1-Re-Registration

typedef unsigned char percentage_t;	//unsigned_int(1) (224)

// choice(integer(1),percentage) (232)
typedef unsigned char sig_strength_t;

typedef uint32_t link_data_rate_t; // unsigned_int(4) (228)

typedef uint32_t link_dn_reason_t; // unsigned_int(1) (228) (Table F.7 235)

/* Table F.7 (235)
 0       Explicit disconnect
 1       Packet timeout
 2       No resource
 3       No broadcast
 4       Authentication failure
 5       Billing failure
 6-127   (Reserved)
 128-255 Vendor specific reason codes
*/

// A data type for configuring link detected event trigger (249)
// sequence ( choice(null, network_id), choice(null, sig_strength),
//            choice(null, link_data_rate) )
typedef struct {
	network_id_t network_id;
	sig_strength_t *sig_strength;
	link_data_rate_t *link_data_rate;
} link_det_cfg_t;

typedef unsigned char link_param_gen_t;    // unsigned_int(1) (229)
typedef unsigned char link_param_qos_t;    // unsigned_int(1)
typedef unsigned char link_param_gg_t;     // unsigned_int(1)
typedef unsigned char link_param_edge_t;   // unsigned_int(1)
typedef unsigned char link_param_eth_t;    // unsigned_int(1)
typedef unsigned char link_param_802_11_t; // unsigned_int(1)
typedef unsigned char link_param_c2k_t;    // unsigned_int(1)
typedef unsigned char link_param_fdd_t;    // unsigned_int(1)
typedef unsigned char link_param_hrpd_t;   // unsigned_int(1)
typedef unsigned char link_param_802_16_t; // unsigned_int(1)
typedef unsigned char link_param_802_20_t; // unsigned_int(1)
typedef unsigned char link_param_802_22_t; // unsigned_int(1)


// link_param_type (choice (link_param_gen,link_param_qos,
//		link_param_gg,link_param_edge,link_param_eth,link_param_802_11,
//		link_param_c2k, link_param_fdd, link_param_hrpd,
//		link_param_802_16, link_param_802_20,link_param_802_22)
typedef unsigned char link_param_type_t;

typedef uint16_t timer_interval_t;  // unsigned_int(2) (234)

typedef unsigned char th_action_t;  // th_action enumerated (233) (unsigned_int(1)

typedef uint16_t threshold_val_t;   // unsigned_int(2) (234)

typedef unsigned char threshold_x_dir_t;	// unsigned_int(1)

// sequence(threshold_val, thresold_x_dir) (233)
typedef struct {
	threshold_val_t threshold_val;
	threshold_x_dir_t threshold_x_dir;
} threshold_t;

// sequence (link_param_type, choice(null,timer_interval),th_action,list(threshold))
typedef struct {
	link_param_type_t link_param_type;
	timer_interval_t  *timer_interval;
	th_action_t       th_action;
	// LIST(threshold_t) *threshold;
	threshold_t *threshold;
} link_cfg_param_t;

// choice( list(link_det_cfg), list(link_cfg_param) )
typedef union {
	// LIST(link_det_cfg_t) *link_det_cfg;
	link_det_cfg_t *link_det_cfg;
	// LIST(link_cfg_param_t) *link_cfg_param;
	link_cfg_param_t *link_cfg_param;
} evt_cfg_info_t;

// sequence(link_param_type,threshold,config_status)
typedef struct {
	link_param_type_t link_param_type;
	threshold_t threshold;
	config_status_t config_status;
} link_cfg_status_t;

// sequence(link_id,choice(null,link_addr))
typedef struct {
	link_id_t link_id;
	link_addr_t *link_addr;
} link_tuple_id_t;

typedef struct {
	unsigned char
		Reserved			: 1,
		Event_Service			: 1,
		Command_Service			: 1,
		Information_Service		: 1,
		Reserved2			: 4;
} link_mihcap_flag_t;

typedef struct { 	// (249)
	link_tuple_id_t     link_tuple_id;
	network_id_t        network_id;
	net_aux_id_t        net_aux_id;
	sig_strength_t      sig_strength;
	uint16_t            sinr;
	link_data_rate_t    link_data_rate;
	link_mihcap_flag_t  link_mihcap_flag;
	net_caps_t          net_caps;
} link_det_info_t;

typedef struct {	// sequence(network_type, link_addr) (251)
	network_type_t  network_type;
	link_addr_t     link_addr;
} net_type_addr_t;

// mbb_ho_supp: sequence(network_type,network_type,boolean) (249)
typedef struct {
	network_type_t from_network;
	network_type_t to_network;
	boolean_t      make_b_break;
} mbb_ho_supp_t;

// indicates wether MN's IP address needs to be changed or not.
// TRUE: changed required; FALSE: change not required
typedef boolean_t ip_renewal_flag_t; // (239)

typedef uint8_t link_gd_reason_t;	// (229)

typedef uint16_t link_states_req_t; // (232) bitmap(16)

typedef union {
	op_mode_t	 op_mode;
	channel_id_t channel_id;
}link_states_rsp_t;	// (232) choice(op_mode,channel_id)

typedef uint16_t link_param_val_t; // (232) unsigned_int(2)

// (237) choice(num_cos_types,list(min_pk_tx_delay),list(avg_pk_tx_delay),
//              list(max_pk_tx_delay),list(pk_delay_jitter), list(pk_loss_rate)
typedef union {
	num_cos_types_t num_cos_types;
	// LIST(min_pk_tx_delay_t) min_pk_tx_delay;
	min_pk_tx_delay_t * min_pk_tx_delay;
	// LIST(avg_pk_tx_delay_t) avg_pk_tx_delay;
	avg_pk_tx_delay_t * avg_pk_tx_delay;
	// LIST(max_pk_tx_delay_t) max_pk_tx_delay;
	max_pk_tx_delay_t * max_pk_tx_delay;
	// LIST(pk_delay_jitter_t) pk_delay_jitter;
	pk_delay_jitter_t * pk_delay_jitter;
	// LIST(pk_loss_rate_t)    pk_loss_rate;
	pk_loss_rate_t * pk_loss_rate;
} qos_param_val_t;

// (229) sequence(link_param_type,choice(link_param_val,qos_param_val))
typedef struct {
	link_param_type_t   link_param_type;
	union {
		link_param_val_t link_param_val;
		qos_param_val_t  qos_param_val;
	} param_val;
} link_param_t;

// link_param_rpt (231) sequence(link_param, choice(null,threshold))
typedef struct {
	link_param_t link_param;
	threshold_t  threshold;
} link_param_rpt_t;


typedef uint16_t link_desc_req_t; // (228) bitmap(16)

typedef uint8_t num_cos_t; // (233) unsigned_int(1)

typedef uint8_t num_queue_t; // (233) unsigned_int(1)

typedef union {
	num_cos_t   num_cos;
	num_queue_t num_queue;
} link_desc_rsp_t; // (228) choice(num_cos,num_queue)

typedef uint8_t link_ac_type_t; // (227) unsigned_int(1)

typedef uint8_t link_ac_attr_t; // (227) bitmap(8)

typedef struct {
	link_ac_type_t link_ac_type;
	link_ac_attr_t link_ac_attr;
} link_action_t; // (227) sequence(link_ac_type,link_ac_attr)

// (232) sequence(link_addr,network_id,sig_strength)
typedef struct {
	link_addr_t    link_addr;
	network_id_t   network_id;
	sig_strength_t sig_strength;
} link_scan_rsp_t;

/*
typedef enum {
	SUCCESS = 0,
	FAILURE = 1,
	REFUSED = 2,
	INCAPABLE = 3,
} link_ac_result_t; // (226) enumerated
*/
typedef int link_ac_result_t;

// (20/07/2010)
// redefinition of struct addrinfo...
// couldn't find it for use in the kernel...
#ifndef _NETDB_H
struct addrinfo {
	int              ai_flags;
	int              ai_family;
	int              ai_socktype;
	int              ai_protocol;
	size_t           ai_addrlen;
	struct sockaddr *ai_addr;
	char            *ai_canonname;
	struct addrinfo *ai_next;
};
#endif

#endif

