
/*
enum {
	CFG80211_SME_IDLE,
	CFG80211_SME_CONNECTING,
	CFG80211_SME_CONNECTED,
} sme_state;
*/

/*
struct cfg80211_conn {
	struct cfg80211_connect_params params;
	// these are sub-states of the _CONNECTING sme_state
	enum {
		CFG80211_CONN_IDLE,
		CFG80211_CONN_SCANNING,
		CFG80211_CONN_SCAN_AGAIN,
		CFG80211_CONN_AUTHENTICATE_NEXT,
		CFG80211_CONN_AUTHENTICATING,
		CFG80211_CONN_ASSOCIATE_NEXT,
		CFG80211_CONN_ASSOCIATING,
		CFG80211_CONN_DEAUTH_ASSOC_FAIL,
	} state;
	u8 bssid[ETH_ALEN], prev_bssid[ETH_ALEN];
	u8 *ie;
	size_t ie_len;
	bool auto_auth, prev_bssid_valid;
};
*/
/*
struct cfg80211_bss {
	struct ieee80211_channel *channel;

	u8 bssid[ETH_ALEN];
	u64 tsf;
	u16 beacon_interval;
	u16 capability;
	u8 *information_elements;
	size_t len_information_elements;
	u8 *beacon_ies;
	size_t len_beacon_ies;
	u8 *proberesp_ies;
	size_t len_proberesp_ies;
	s32 signal;
	void (*free_priv)(struct cfg80211_bss *bss);
	u8 priv[0] __attribute__((__aligned__(sizeof(void *))));
};
*/

struct cfg80211_internal_bss {
	struct list_head list;
	struct rb_node rbn;
	unsigned long ts;
	struct kref ref;
	atomic_t hold;
	bool beacon_ies_allocated;
	bool proberesp_ies_allocated;

	// must be last because of priv member
	struct cfg80211_bss pub;
};

/*
enum {
	CFG80211_SME_IDLE,
	CFG80211_SME_CONNECTING,
	CFG80211_SME_CONNECTED,
} sme_state;
*/
#define MAX_AUTH_BSSES                    4
/*
#define NL80211_MAX_NR_CIPHER_SUITES      5
#define NL80211_MAX_NR_AKM_SUITES         2
*/

/*
enum nl80211_iftype {
	NL80211_IFTYPE_UNSPECIFIED,
	NL80211_IFTYPE_ADHOC,
	NL80211_IFTYPE_STATION,
	NL80211_IFTYPE_AP,
	NL80211_IFTYPE_AP_VLAN,
	NL80211_IFTYPE_WDS,
	NL80211_IFTYPE_MONITOR,
	NL80211_IFTYPE_MESH_POINT,
	// keep last
	__NL80211_IFTYPE_AFTER_LAST,
	NL80211_IFTYPE_MAX = __NL80211_IFTYPE_AFTER_LAST - 1
};
*/

struct cfg80211_ibss_params {
	u8 *ssid;
	u8 *bssid;
	struct ieee80211_channel *channel;
	u8 *ie;
	u8 ssid_len, ie_len;
	u16 beacon_interval;
	bool channel_fixed;
	bool privacy;
};

/*
enum nl80211_auth_type {
	NL80211_AUTHTYPE_OPEN_SYSTEM,
	NL80211_AUTHTYPE_SHARED_KEY,
	NL80211_AUTHTYPE_FT,
	NL80211_AUTHTYPE_NETWORK_EAP,
	// keep last
	__NL80211_AUTHTYPE_NUM,
	NL80211_AUTHTYPE_MAX = __NL80211_AUTHTYPE_NUM - 1,
	NL80211_AUTHTYPE_AUTOMATIC
};
*/
/*
struct cfg80211_crypto_settings {
	u32 wpa_versions;
	u32 cipher_group;
	int n_ciphers_pairwise;
	u32 ciphers_pairwise[NL80211_MAX_NR_CIPHER_SUITES];
	int n_akm_suites;
	u32 akm_suites[NL80211_MAX_NR_AKM_SUITES];
	bool control_port;
};
*/
/*
struct cfg80211_connect_params {
	struct ieee80211_channel *channel;
	u8 *bssid;
	u8 *ssid;
	size_t ssid_len;
	enum nl80211_auth_type auth_type;
	u8 *ie;
	size_t ie_len;
	bool privacy;
	struct cfg80211_crypto_settings crypto;
	const u8 *key;
	u8 key_len, key_idx;
};
*/

struct wireless_dev {
	struct wiphy *wiphy;
	enum nl80211_iftype iftype;

	// the remainder of this struct should be private to cfg80211
	struct list_head list;
	struct net_device *netdev;

	struct list_head action_registrations;
	spinlock_t action_registrations_lock;

	struct mutex mtx;

	struct work_struct cleanup_work;

	bool use_4addr;

	// currently used for IBSS and SME - might be rearranged later
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	u8 ssid_len;
	enum {
		CFG80211_SME_IDLE,
		CFG80211_SME_CONNECTING,
		CFG80211_SME_CONNECTED,
	} sme_state;
	struct cfg80211_conn *conn;
	struct cfg80211_cached_keys *connect_keys;

	struct list_head event_list;
	spinlock_t event_lock;

	struct cfg80211_internal_bss *authtry_bsses[MAX_AUTH_BSSES];
	struct cfg80211_internal_bss *auth_bsses[MAX_AUTH_BSSES];
	struct cfg80211_internal_bss *current_bss; // associated / joined
	struct ieee80211_channel *channel;

	bool ps;
	int ps_timeout;

#ifdef CONFIG_CFG80211_WEXT
	// wext data
	struct {
		struct cfg80211_ibss_params ibss;
		struct cfg80211_connect_params connect;
		struct cfg80211_cached_keys *keys;
		u8 *ie;
		size_t ie_len;
		u8 bssid[ETH_ALEN], prev_bssid[ETH_ALEN];
		u8 ssid[IEEE80211_MAX_SSID_LEN];
		s8 default_key, default_mgmt_key;
		bool prev_bssid_valid;
	} wext;
#endif
};

