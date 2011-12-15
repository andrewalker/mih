/*
 * Protocol handling functions
 */

int ProcessRequest(mih_message_t *message);
int ProcessService(mih_message_t *message);
int ProcessEvent(mih_message_t *message);
int ProcessCmd(mih_message_t *message);
int ProcessInfo(mih_message_t *message);

int CapabilityDiscover(mih_message_t *message);
int Register(mih_message_t *message);
int Deregister(mih_message_t *message);
int EventSubscribe(mih_message_t *message);
int EventUnsubscribe(mih_message_t *message);

int LinkDetected(mih_message_t *message);
int LinkUp(mih_message_t *message);
int LinkDown(mih_message_t *message);
int LinkParametersReport(mih_message_t *message);
int LinkGoingDown(mih_message_t *message);
int LinkHandoverImminent(mih_message_t *message);
int LinkHandoverComplete(mih_message_t *message);

int LinkGetParameters(mih_message_t *message);
int LinkConfigureThresholds(mih_message_t *message);
int LinkActions(mih_message_t *message);
int NetHOCandidateQuery(mih_message_t *message);
int MNHOCandidateQuery(mih_message_t *message);
int N2NHOQueryResources(mih_message_t *message);
int MNHOCommit(mih_message_t *message);
int NetHOCommit(mih_message_t *message);
int N2NHOCommit(mih_message_t *message);
int MNHOComplete(mih_message_t *message);
int N2NHOComplete(mih_message_t *message);
