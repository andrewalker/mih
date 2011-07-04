
/*
	Protocol handling functions
 */

// function prototypes

int ProcessRequest(char *in_buf, int in_len);
int ProcessService(char *in_buf, int in_len);
int ProcessEvent(char *in_buf, int in_len);
int ProcessCmd(char *in_buf, int in_len);
int ProcessInfo(char *in_buf, int in_len);

int CapabilityDiscover(char *in_buf, int in_len);
int Register(char *in_buf, int in_len);
int Deregister(char *in_buf, int in_len);
int EventSubscribe(char *in_buf, int in_len);
int EventUnsubscribe(char *in_buf, int in_len);

int LinkDetected(char *in_buf, int in_len);
int LinkUp(char *in_buf, int in_len);
int LinkDown(char *in_buf, int in_len);
int LinkParametersReport(char *in_buf, int in_len);
int LinkGoingDown(char *in_buf, int in_len);
int LinkHandoverImminent(char *in_buf, int in_len);
int LinkHandoverComplete(char *in_buf, int in_len);

int LinkGetParameters(char *in_buf, int in_len);
int LinkConfigureThresholds(char *in_buf, int in_len);
int LinkActions(char *in_buf, int in_len);
int NetHOCandidateQuery(char *in_buf, int in_len);
int MNHOCandidateQuery(char *in_buf, int in_len);
int N2NHOQueryResources(char *in_buf, int in_len);
int MNHOCommit(char *in_buf, int in_len);
int NetHOCommit(char *in_buf, int in_len);
int N2NHOCommit(char *in_buf, int in_len);
int MNHOComplete(char *in_buf, int in_len);
int N2NHOComplete(char *in_buf, int in_len);

