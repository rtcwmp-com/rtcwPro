
#ifndef cg_ndp_h
#define cg_ndp_h
//Syscalls to Client
void trap_CNQ3_NDP_Enable(void);
int trap_CNQ3_NDP_Seek(int servertime);
void trap_CNQ3_NDP_ReadUntil(int serverTime);
void trap_CNQ3_NDP_StartVideo(void);
void trap_CNQ3_NDP_StopVideo(void);
void trap_LocateInteropData(void* bufferIn, int bufferInSize, void* bufferOut, int bufferOutSize);

//Syscalls from Client
void CG_NDP_EndAnalysis(const char* filePath, int firstServerTime, int lastServerTime, qboolean videoRestart);
void CG_NDP_AnalyzeCommand(int serverTime);
qbool CG_NDP_AnalyzeSnapshot(int progress);

//Public functions
void CG_NDP_ResetStateWhenBackInTime(void);

extern int ndp_myKills[1024];
extern int ndp_myKillsSize;
extern int ndp_alliesWins[1024];
extern int ndp_axisWins[1024];
extern int ndp_alliesWinsSize;
extern int ndp_axisWinsSize;
extern int ndp_lastgamestate;
extern int ndp_currentRound;
extern int ndp_nextTimeLimit;
extern int ndp_round1End[32];
extern int ndp_round2End[32];
extern int ndp_round1EndSize;
extern int ndp_round2EndSize;
extern int ndp_docDropTime[1024];
extern int ndp_docPickupTime[1024];
extern int ndp_docDropSize;
extern int ndp_docPickupSize;
extern qbool ndp_killStreak[1024];
extern qbool isRtcwPro;
extern qbool isRtcwProV128;
extern qbool isRtcwProV129;
extern qbool isRtcwProV130;
extern int m_currServerTime;
extern int m_firstServerTime;
extern int m_lastServerTime;
#endif