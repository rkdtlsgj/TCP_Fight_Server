#pragma once
#define dfNETWORK_PORT 11810
#define dfNETWORK_PACKET_RECV_TIME 10000

#define dfRANGE_MOVE_TOP	0
#define dfRANGE_MOVE_LEFT	0
#define dfRANGE_MOVE_RIGHT	6400
#define dfRANGE_MOVE_BOTTOM	6400


//---------------------------------------------------------------
// 공격범위.
//---------------------------------------------------------------
#define dfATTACK1_RANGE_X		80
#define dfATTACK2_RANGE_X		90
#define dfATTACK3_RANGE_X		100
#define dfATTACK1_RANGE_Y		10
#define dfATTACK2_RANGE_Y		10
#define dfATTACK3_RANGE_Y		20


//---------------------------------------------------------------
// 공격 데미지.
//---------------------------------------------------------------
#define dfATTACK1_DAMAGE		1
#define dfATTACK2_DAMAGE		2
#define dfATTACK3_DAMAGE		3


#define dfMAP_WIDTH 6400
#define dfMAP_HEIGHT 6400

#define dfACTION_MOVE_LL	0
#define dfACTION_MOVE_LU	1
#define dfACTION_MOVE_UU	2
#define dfACTION_MOVE_RU	3
#define dfACTION_MOVE_RR	4
#define dfACTION_MOVE_RD	5
#define dfACTION_MOVE_DD	6
#define dfACTION_MOVE_LD	7

#define dfACTION_ATTACK1	8
#define dfACTION_ATTACK2	9
#define dfACTION_ATTACK3	10

#define dfACTION_STAND		11

#define dfSPEED_PLAYER_X	3	// 3   50fps
#define dfSPEED_PLAYER_Y	2	// 2   50fps

#define dfERROR_RANGE		50


#define dfSECTOR_SIZE_X 128
#define dfSECTOR_SIZE_Y 128

#define dfNETWORK_PACKET_RECV_TIMEOUT 1000000

#define dfSECTOR_MAX_X dfMAP_WIDTH / dfSECTOR_SIZE_X
#define dfSECTOR_MAX_Y dfMAP_HEIGHT / dfSECTOR_SIZE_Y
