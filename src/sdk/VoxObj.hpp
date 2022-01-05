#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <cstdint>

namespace SoLoud {
	class WavStream;
	class Wav;
}

class VoxObj
{
public:
	SoLoud::WavStream* m_wave = nullptr;
	SoLoud::Wav* m_w = nullptr;
	int m_wave_handle = -1;
	float m_volume = 0.0f;
	double m_loop_start = 0.0f;
	double m_loop_end = 0.0f;
	double m_time = 0.0f;
	unsigned int m_frame = 0;
	unsigned int m_previous_volume_bits = 0;
	bool m_loop = false;
	bool m_once_flag = false;
	char pad_0004[8]; //0x0004
	int16_t buffer[2048]; //0x000C
	char pad_100C[4158]; //0x100C

	virtual BOOL _stdcall load(const char* filename);
	virtual BOOL _stdcall restart();
	virtual BOOL _stdcall pause();
	virtual BOOL _stdcall free_buffer_and_close_file_probably();
	virtual BOOL _stdcall unk_sub_10001BC0(int a2);
	virtual void _stdcall Function5();
	virtual BOOL _stdcall set_volume(float volume);
	virtual void _stdcall Function7();
	virtual void _stdcall Function8();
	virtual void _stdcall Function9();
	virtual void _stdcall Function10();
	virtual void _stdcall Function11();
	virtual void _stdcall Function12();
	virtual BOOL _stdcall Function13();
	virtual void* _stdcall get_ogg_comment_ptr();
	virtual void _stdcall Function15();
	virtual void _stdcall Function16();
	virtual BOOL _stdcall Function17(int a3, float a4, float a5);
	virtual BOOL _stdcall load_mem(unsigned char *a_mem, int a_length);
	virtual void _stdcall play3d(float a_pos_x, float a_pos_y, float a_pos_z, float volume);
	virtual void _stdcall set_listener3d(
		float a_lpos_x, float a_lpos_y, float a_lpos_z,
		float a_at_x,   float a_at_y,   float a_at_z,
		float a_up_x,   float a_up_y,   float a_up_z );
	//virtual void _stdcall set_source3d_pos(float x, float y, float z);
}; //Size: 0x204A
