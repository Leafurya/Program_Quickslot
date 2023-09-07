#ifndef _log__h_
#define _log__h_

//unsigned __stdcall LogThreadFunc(void *args);
void LogMessage(char *msg);
void SaveLog();
void EndLogging();

#endif
